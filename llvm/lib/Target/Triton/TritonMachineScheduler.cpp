//===- TritonMachineScheduler.cpp - MI Scheduler for Triton ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Triton Machine Instruction Scheduler.
// It provides a custom pre-RA scheduling strategy that extends GenericScheduler
// with Triton-specific heuristics for memory clustering and latency hiding.
//
//===----------------------------------------------------------------------===//

#include "TritonMachineScheduler.h"
#include "TritonSubtarget.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/ScheduleDAGInstrs.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "triton-sched"

//===----------------------------------------------------------------------===//
// TritonPreRASchedStrategy Implementation
//===----------------------------------------------------------------------===//

TritonPreRASchedStrategy::TritonPreRASchedStrategy(const MachineSchedContext *C)
    : GenericScheduler(C) {
  LLVM_DEBUG(dbgs() << "Triton Pre-RA Scheduler initialized\n");
}

void TritonPreRASchedStrategy::initPolicy(MachineBasicBlock::iterator Begin,
                                          MachineBasicBlock::iterator End,
                                          unsigned NumRegionInstrs) {
  // First, call the base class implementation to set up standard policy
  GenericScheduler::initPolicy(Begin, End, NumRegionInstrs);

  // Log the policy initialization for debugging
  LLVM_DEBUG(dbgs() << "Triton initPolicy: region with " << NumRegionInstrs
                    << " instructions\n");

  // For very small regions, we might want to adjust the policy
  // to avoid overhead of complex scheduling decisions
  if (NumRegionInstrs <= 3) {
    LLVM_DEBUG(dbgs() << "  Small region - using simplified scheduling\n");
  }
}

void TritonPreRASchedStrategy::dumpPolicy() const {
  dbgs() << "Triton Scheduling Policy:\n";
  dbgs() << "  ShouldTrackPressure: " << RegionPolicy.ShouldTrackPressure
         << "\n";
  dbgs() << "  ShouldTrackLaneMasks: " << RegionPolicy.ShouldTrackLaneMasks
         << "\n";
  dbgs() << "  OnlyTopDown: " << RegionPolicy.OnlyTopDown << "\n";
  dbgs() << "  OnlyBottomUp: " << RegionPolicy.OnlyBottomUp << "\n";
  dbgs() << "  DisableLatencyHeuristic: "
         << RegionPolicy.DisableLatencyHeuristic << "\n";
}

bool TritonPreRASchedStrategy::biasMemoryCluster(SchedCandidate &Cand,
                                                 SchedCandidate &TryCand,
                                                 SchedBoundary &Zone) const {
  // Check if either candidate is a memory operation
  const MachineInstr *CandMI = Cand.SU ? Cand.SU->getInstr() : nullptr;
  const MachineInstr *TryCandMI = TryCand.SU ? TryCand.SU->getInstr() : nullptr;

  if (!CandMI || !TryCandMI)
    return false;

  bool CandIsMemOp = CandMI->mayLoad() || CandMI->mayStore();
  bool TryCandIsMemOp = TryCandMI->mayLoad() || TryCandMI->mayStore();

  // If TryCand is a memory operation and Cand is not, prefer TryCand
  // to help cluster memory operations together
  if (TryCandIsMemOp && !CandIsMemOp) {
    TryCand.Reason = Cluster;
    LLVM_DEBUG(dbgs() << "  Triton: preferring memory op for clustering\n");
    return true;
  }

  // If both are memory operations, prefer loads over stores in top-down
  // scheduling to expose more ILP
  if (TryCandIsMemOp && CandIsMemOp && Zone.isTop()) {
    if (TryCandMI->mayLoad() && CandMI->mayStore()) {
      TryCand.Reason = Cluster;
      LLVM_DEBUG(dbgs() << "  Triton: preferring load over store (top-down)\n");
      return true;
    }
  }

  return false;
}

bool TritonPreRASchedStrategy::biasHighLatency(SchedCandidate &Cand,
                                               SchedCandidate &TryCand,
                                               SchedBoundary &Zone) const {
  // This heuristic is primarily useful for bottom-up scheduling
  // where we want to schedule high-latency instructions early
  // to hide their latency
  if (Zone.isTop())
    return false;

  if (!Cand.SU || !TryCand.SU)
    return false;

  // Get the latencies of both candidates
  unsigned CandLatency = Cand.SU->getHeight();
  unsigned TryCandLatency = TryCand.SU->getHeight();

  // In bottom-up scheduling, prefer the instruction with higher latency
  // (larger height means more latency to hide)
  if (TryCandLatency > CandLatency + 1) {
    TryCand.Reason = Stall;
    LLVM_DEBUG(dbgs() << "  Triton: preferring high-latency instruction "
                      << "(height " << TryCandLatency << " vs " << CandLatency
                      << ")\n");
    return true;
  }

  return false;
}

bool TritonPreRASchedStrategy::tryCandidate(SchedCandidate &Cand,
                                            SchedCandidate &TryCand,
                                            SchedBoundary *Zone) const {
  // First, apply all standard GenericScheduler heuristics
  // The base class handles: PhysReg, RegExcess, RegCritical, Stall,
  // Cluster, Weak, RegMax, ResourceReduce, ResourceDemand, latency, etc.

  // Initialize the candidate if needed (from GenericScheduler)
  // If Cand is invalid, TryCand is the first valid candidate, but we still
  // want to apply Triton-specific heuristics to potentially improve the reason
  bool CandWasInvalid = !Cand.isValid();
  if (CandWasInvalid) {
    TryCand.Reason = FirstValid;
    // Skip all comparison-based heuristics when Cand is invalid, but still
    // apply Triton-specific heuristics at the end
  } else {
    // Bias PhysReg Defs and copies to their uses and defined respectively.
    if (tryGreater(biasPhysReg(TryCand.SU, TryCand.AtTop),
                   biasPhysReg(Cand.SU, Cand.AtTop), TryCand, Cand, PhysReg))
      return TryCand.Reason != NoCand;

    // Avoid exceeding the target's limit.
    if (DAG->isTrackingPressure() &&
        tryPressure(TryCand.RPDelta.Excess, Cand.RPDelta.Excess, TryCand, Cand,
                    RegExcess, TRI, DAG->MF))
      return TryCand.Reason != NoCand;

    // Avoid increasing the max critical pressure in the scheduled region.
    if (DAG->isTrackingPressure() &&
        tryPressure(TryCand.RPDelta.CriticalMax, Cand.RPDelta.CriticalMax,
                    TryCand, Cand, RegCritical, TRI, DAG->MF))
      return TryCand.Reason != NoCand;
  }

  bool SameBoundary = Zone != nullptr;
  if (SameBoundary && !CandWasInvalid) {
    // For loops that are acyclic path limited, aggressively schedule for
    // latency.
    if (Rem.IsAcyclicLatencyLimited && !Zone->getCurrMOps() &&
        tryLatency(TryCand, Cand, *Zone))
      return TryCand.Reason != NoCand;

    // Prioritize instructions that read unbuffered resources by stall cycles.
    if (tryLess(Zone->getLatencyStallCycles(TryCand.SU),
                Zone->getLatencyStallCycles(Cand.SU), TryCand, Cand, Stall))
      return TryCand.Reason != NoCand;
  }

  if (!CandWasInvalid) {
    // Keep clustered nodes together to encourage downstream peephole
    // optimizations which may reduce resource requirements.
    const ClusterInfo *CandCluster = Cand.AtTop ? TopCluster : BotCluster;
    const ClusterInfo *TryCandCluster = TryCand.AtTop ? TopCluster : BotCluster;
    if (tryGreater(TryCandCluster && TryCandCluster->contains(TryCand.SU),
                   CandCluster && CandCluster->contains(Cand.SU), TryCand, Cand,
                   Cluster))
      return TryCand.Reason != NoCand;
  }

  if (SameBoundary && !CandWasInvalid) {
    // Weak edges are for clustering and other constraints.
    if (tryLess(getWeakLeft(TryCand.SU, TryCand.AtTop),
                getWeakLeft(Cand.SU, Cand.AtTop), TryCand, Cand, Weak))
      return TryCand.Reason != NoCand;
  }

  if (!CandWasInvalid) {
    // Avoid increasing the max pressure of the entire region.
    if (DAG->isTrackingPressure() &&
        tryPressure(TryCand.RPDelta.CurrentMax, Cand.RPDelta.CurrentMax, TryCand,
                    Cand, RegMax, TRI, DAG->MF))
      return TryCand.Reason != NoCand;
  }

  if (SameBoundary && !CandWasInvalid) {
    // Avoid critical resource consumption and balance the schedule.
    TryCand.initResourceDelta(DAG, SchedModel);
    if (tryLess(TryCand.ResDelta.CritResources, Cand.ResDelta.CritResources,
                TryCand, Cand, ResourceReduce))
      return TryCand.Reason != NoCand;
    if (tryGreater(TryCand.ResDelta.DemandedResources,
                   Cand.ResDelta.DemandedResources, TryCand, Cand,
                   ResourceDemand))
      return TryCand.Reason != NoCand;

    // Avoid serializing long latency dependence chains.
    if (!RegionPolicy.DisableLatencyHeuristic && TryCand.Policy.ReduceLatency &&
        !Rem.IsAcyclicLatencyLimited && tryLatency(TryCand, Cand, *Zone))
      return TryCand.Reason != NoCand;

    // Fall through to original instruction order.
    if ((Zone->isTop() && TryCand.SU->NodeNum < Cand.SU->NodeNum) ||
        (!Zone->isTop() && TryCand.SU->NodeNum > Cand.SU->NodeNum)) {
      TryCand.Reason = NodeOrder;
    }
  }

  // If Cand was invalid, TryCand is the first valid candidate and should be
  // accepted. But we still apply Triton-specific heuristics to potentially
  // improve the reason.
  if (CandWasInvalid) {
    // Apply Triton-specific heuristics even for the first valid candidate
    if (SameBoundary) {
      // For the first candidate, we can't compare with Cand, but we can still
      // mark it with appropriate reasons based on Triton heuristics
      const MachineInstr *TryCandMI = TryCand.SU ? TryCand.SU->getInstr() : nullptr;
      if (TryCandMI && (TryCandMI->mayLoad() || TryCandMI->mayStore())) {
        // If it's a memory operation, prefer it for clustering
        TryCand.Reason = Cluster;
        LLVM_DEBUG(dbgs() << "  Triton: first valid candidate is memory op\n");
      }
    }
    return true;
  }

  // If standard heuristics didn't make a decision, apply Triton-specific ones
  if (TryCand.Reason != NodeOrder && TryCand.Reason != NoCand)
    return true;

  // Triton-specific heuristic: prefer memory clustering
  if (SameBoundary && biasMemoryCluster(Cand, TryCand, *Zone))
    return TryCand.Reason != NoCand;

  // Triton-specific heuristic: prefer high-latency instructions in bottom-up
  if (SameBoundary && biasHighLatency(Cand, TryCand, *Zone))
    return TryCand.Reason != NoCand;

  return TryCand.Reason != NoCand;
}

//===----------------------------------------------------------------------===//
// TritonDAGMutation Implementation
//===----------------------------------------------------------------------===//

void TritonDAGMutation::apply(ScheduleDAGInstrs *DAG) {
  LLVM_DEBUG(dbgs() << "Applying TritonDAGMutation\n");

  if (!DAG)
    return;

  // Add weak edges between related instructions
  addWeakEdges(DAG);
}

void TritonDAGMutation::addWeakEdges(ScheduleDAGInstrs *DAG) {
  // This method adds weak (artificial) edges between related instructions
  // to help the scheduler keep them together without creating hard
  // dependencies.
  //
  // Example use cases:
  // 1. Address computation followed by memory access
  // 2. Related arithmetic operations that benefit from being scheduled together

  for (SUnit &SU : DAG->SUnits) {
    MachineInstr *MI = SU.getInstr();
    if (!MI)
      continue;

    // Look for memory operations and try to find their address producers
    if (MI->mayLoad() || MI->mayStore()) {
      // Check predecessors for potential address computations
      for (SDep &Pred : SU.Preds) {
        if (Pred.getKind() != SDep::Data)
          continue;

        SUnit *PredSU = Pred.getSUnit();
        if (!PredSU || !PredSU->getInstr())
          continue;

        MachineInstr *PredMI = PredSU->getInstr();

        // If the predecessor is an address computation (e.g., ADD, ADDI),
        // we might want to add a weak edge to keep them close
        // For now, we just log this for demonstration
        LLVM_DEBUG(dbgs() << "  Found memory op with data predecessor: "
                          << *PredMI);
      }
    }
  }

  LLVM_DEBUG(dbgs() << "TritonDAGMutation: processed " << DAG->SUnits.size()
                    << " scheduling units\n");
}

//===----------------------------------------------------------------------===//
// Factory Function and Registry
//===----------------------------------------------------------------------===//

ScheduleDAGInstrs *llvm::createTritonMachineScheduler(MachineSchedContext *C) {
  ScheduleDAGMILive *DAG =
      new ScheduleDAGMILive(C, std::make_unique<TritonPreRASchedStrategy>(C));

  // Add the Triton-specific DAG mutation
  DAG->addMutation(std::make_unique<TritonDAGMutation>());

  // Add standard mutations
  DAG->addMutation(createCopyConstrainDAGMutation(DAG->TII, DAG->TRI));

  // Add MacroFusion mutation if available
  const TargetSubtargetInfo &STI = C->MF->getSubtarget();
  const auto &MacroFusions = STI.getMacroFusions();
  if (!MacroFusions.empty())
    DAG->addMutation(createMacroFusionDAGMutation(MacroFusions));

  LLVM_DEBUG(dbgs() << "Created Triton Machine Scheduler\n");
  return DAG;
}

// Register the Triton scheduler with the MachineSchedRegistry
// This allows users to select it via -misched=triton
static MachineSchedRegistry
    TritonSchedRegistry("triton", "Triton custom pre-RA scheduler",
                        createTritonMachineScheduler);
