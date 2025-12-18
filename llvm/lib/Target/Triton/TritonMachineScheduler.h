//===- TritonMachineScheduler.h - Custom Triton MI scheduler --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Custom Triton Machine Instruction Scheduler for Pre-RA scheduling.
// This scheduler extends GenericScheduler to provide Triton-specific
// scheduling heuristics and DAG mutations.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRITON_TRITONMACHINESCHEDULER_H
#define LLVM_LIB_TARGET_TRITON_TRITONMACHINESCHEDULER_H

#include "llvm/CodeGen/MachineScheduler.h"
#include "llvm/CodeGen/ScheduleDAGMutation.h"

namespace llvm {

//===----------------------------------------------------------------------===//
// TritonPreRASchedStrategy - Pre-RA scheduling strategy for Triton
//===----------------------------------------------------------------------===//

/// A MachineSchedStrategy implementation for Triton pre-RA scheduling.
/// This class extends GenericScheduler to add Triton-specific heuristics
/// for instruction scheduling before register allocation.
class TritonPreRASchedStrategy : public GenericScheduler {
public:
  /// Constructor - Initialize with the scheduling context.
  TritonPreRASchedStrategy(const MachineSchedContext *C);

  /// Initialize the scheduling policy for a new region.
  /// This is called before building the DAG for each scheduling region.
  void initPolicy(MachineBasicBlock::iterator Begin,
                  MachineBasicBlock::iterator End,
                  unsigned NumRegionInstrs) override;

  /// Dump the current scheduling policy for debugging.
  void dumpPolicy() const override;

protected:
  /// Compare two scheduling candidates and determine which is better.
  /// Returns true if TryCand is better than Cand.
  /// This method first applies standard GenericScheduler heuristics,
  /// then applies Triton-specific heuristics if no decision was made.
  bool tryCandidate(SchedCandidate &Cand, SchedCandidate &TryCand,
                    SchedBoundary *Zone) const override;

private:
  /// Triton-specific heuristic: prefer clustering memory operations.
  /// This helps improve memory access patterns and cache utilization.
  /// @param Cand The current best candidate
  /// @param TryCand The candidate being evaluated
  /// @param Zone The scheduling boundary (top or bottom)
  /// @return true if TryCand should be preferred for memory clustering
  bool biasMemoryCluster(SchedCandidate &Cand, SchedCandidate &TryCand,
                         SchedBoundary &Zone) const;

  /// Triton-specific heuristic: prefer high-latency instructions early.
  /// In bottom-up scheduling, scheduling high-latency instructions earlier
  /// can help hide latency by allowing other instructions to execute
  /// while waiting for the result.
  /// @param Cand The current best candidate
  /// @param TryCand The candidate being evaluated
  /// @param Zone The scheduling boundary (top or bottom)
  /// @return true if TryCand should be preferred for latency reasons
  bool biasHighLatency(SchedCandidate &Cand, SchedCandidate &TryCand,
                       SchedBoundary &Zone) const;
};

//===----------------------------------------------------------------------===//
// TritonDAGMutation - DAG post-processor for Triton
//===----------------------------------------------------------------------===//

/// A ScheduleDAGMutation implementation for Triton.
/// This mutation can modify the scheduling DAG after it's built,
/// adding target-specific dependencies or weak edges to guide scheduling.
class TritonDAGMutation : public ScheduleDAGMutation {
public:
  /// Apply the mutation to the scheduling DAG.
  /// This method is called after the DAG is built and before scheduling begins.
  /// @param DAG The scheduling DAG to modify
  void apply(ScheduleDAGInstrs *DAG) override;

  /// Get the name of this mutation for debugging purposes.
  /// @return A string identifying this mutation
  StringRef getName() const { return "TritonDAGMutation"; }

private:
  /// Add weak edges between related instructions.
  /// This helps the scheduler keep related instructions together,
  /// such as address computations and their corresponding memory accesses.
  /// @param DAG The scheduling DAG to modify
  void addWeakEdges(ScheduleDAGInstrs *DAG);
};

/// Factory function to create the Triton machine scheduler.
/// This function creates a ScheduleDAGMILive with TritonPreRASchedStrategy
/// and adds the TritonDAGMutation.
/// @param C The machine scheduling context
/// @return A new ScheduleDAGInstrs configured for Triton
ScheduleDAGInstrs *createTritonMachineScheduler(MachineSchedContext *C);

} // end namespace llvm

#endif // LLVM_LIB_TARGET_TRITON_TRITONMACHINESCHEDULER_H
