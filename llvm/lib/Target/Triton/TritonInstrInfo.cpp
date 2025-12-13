//===-- TritonInstrInfo.cpp - Triton Instruction Information -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------===//
//
// This file contains the Triton implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "TritonInstrInfo.h"
#include "Triton.h"
#include "TritonSubtarget.h"
#include "TritonTargetMachine.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#define GET_INSTRINFO_NAMED_OPS
#include "TritonGenInstrInfo.inc"

TritonInstrInfo::TritonInstrInfo(const TritonSubtarget &STI)
    : TritonGenInstrInfo(Triton::ADJCALLSTACKDOWN, Triton::ADJCALLSTACKUP),
      RI(0), Subtarget(STI) {}

void TritonInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator MBBI,
                                  const DebugLoc &DL, Register DestReg,
                                  Register SrcReg, bool KillSrc,
                                  bool RenamableDest, bool RenamableSrc) const {
  if (Triton::GPRRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MBBI, DL, get(Triton::ADDI), DestReg)
        .addReg(SrcReg, getKillRegState(KillSrc))
        .addImm(0);
    return;
  }

  llvm_unreachable("Impossible reg-to-reg copy");
}

void TritonInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator I,
                                          Register SrcReg, bool IsKill, int FI,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI,
                                          Register VReg,
                                          MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  if (Triton::GPRRegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, I, DL, get(Triton::SW))
        .addReg(SrcReg, getKillRegState(IsKill))
        .addFrameIndex(FI)
        .addImm(0);
    return;
  }

  llvm_unreachable("Can't store this register to stack slot");
}

void TritonInstrInfo::loadRegFromStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator I, Register DestReg,
    int FI, const TargetRegisterClass *RC, const TargetRegisterInfo *TRI,
    Register VReg, MachineInstr::MIFlag Flags) const {
  DebugLoc DL;
  if (I != MBB.end())
    DL = I->getDebugLoc();

  if (Triton::GPRRegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, I, DL, get(Triton::LW), DestReg).addFrameIndex(FI).addImm(0);
    return;
  }

  llvm_unreachable("Can't load this register from stack slot");
}

void TritonInstrInfo::movImm(MachineBasicBlock &MBB,
                             MachineBasicBlock::iterator MBBI,
                             const DebugLoc &DL, Register DstReg, uint64_t Val,
                             MachineInstr::MIFlag Flag) const {
  assert(isInt<32>(Val) && "Can only materialize 32-bit constants");

  if (isInt<12>(Val)) {
    BuildMI(MBB, MBBI, DL, get(Triton::ADDI), DstReg)
        .addReg(Triton::X0)
        .addImm(Val)
        .setMIFlag(Flag);
  } else {
    int64_t Hi20 = ((Val + 0x800) >> 12) & 0xFFFFF;
    int64_t Lo12 = SignExtend64<12>(Val);
    BuildMI(MBB, MBBI, DL, get(Triton::LUI), DstReg)
        .addImm(Hi20)
        .setMIFlag(Flag);
    if (Lo12 != 0) {
      BuildMI(MBB, MBBI, DL, get(Triton::ADDI), DstReg)
          .addReg(DstReg)
          .addImm(Lo12)
          .setMIFlag(Flag);
    }
  }
}

unsigned TritonInstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  if (MI.isInlineAsm()) {
    const MachineFunction *MF = MI.getParent()->getParent();
    const char *AsmStr = MI.getOperand(0).getSymbolName();
    return getInlineAsmLength(AsmStr, *MF->getTarget().getMCAsmInfo());
  }

  return MI.getDesc().getSize();
}

bool TritonInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *&TBB,
                                    MachineBasicBlock *&FBB,
                                    SmallVectorImpl<MachineOperand> &Cond,
                                    bool AllowModify) const {
  TBB = FBB = nullptr;
  Cond.clear();

  // If the block has no terminators, it just falls into the block after it.
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end() || !isUnpredicatedTerminator(*I))
    return false;

  // Count the number of terminators and find the first unconditional or
  // indirect branch.
  MachineBasicBlock::iterator FirstUncondOrIndirectBr = MBB.end();
  int NumTerminators = 0;
  for (auto J = I.getReverse(); J != MBB.rend() && isUnpredicatedTerminator(*J);
       J++) {
    NumTerminators++;
    if (J->getDesc().isUnconditionalBranch() ||
        J->getDesc().isIndirectBranch()) {
      FirstUncondOrIndirectBr = J.getReverse();
    }
  }

  // If AllowModify is true, we can erase any terminators after
  // FirstUncondOrIndirectBR.
  if (AllowModify && FirstUncondOrIndirectBr != MBB.end()) {
    while (std::next(FirstUncondOrIndirectBr) != MBB.end()) {
      std::next(FirstUncondOrIndirectBr)->eraseFromParent();
      NumTerminators--;
    }
    I = FirstUncondOrIndirectBr;
  }

  // We can't handle blocks that end in an indirect branch.
  if (I->getDesc().isIndirectBranch())
    return true;

  // We can't handle blocks with more than 2 terminators.
  if (NumTerminators > 2)
    return true;

  // Handle a single unconditional branch.
  if (NumTerminators == 1 && I->getDesc().isUnconditionalBranch()) {
    TBB = getBranchDestBlock(*I);
    return false;
  }

  // Handle a single conditional branch.
  if (NumTerminators == 1 && I->getDesc().isConditionalBranch()) {
    // Block ends with fall-through condbranch.
    parseCondBranch(*I, TBB, Cond);
    return false;
  }

  // Handle a conditional branch followed by an unconditional branch.
  if (NumTerminators == 2 && std::prev(I)->getDesc().isConditionalBranch() &&
      I->getDesc().isUnconditionalBranch()) {
    parseCondBranch(*std::prev(I), TBB, Cond);
    FBB = getBranchDestBlock(*I);
    return false;
  }

  // Otherwise, we can't handle this.
  return true;
}

unsigned TritonInstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {
  if (BytesAdded)
    *BytesAdded = 0;

  // Shouldn't be a fall through.
  assert(TBB && "insertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 3 || Cond.size() == 0) &&
         "Triton branch conditions have two components!");

  // Unconditional branch.
  if (Cond.empty()) {
    MachineInstr &MI = *BuildMI(&MBB, DL, get(Triton::PseudoJUMP)).addMBB(TBB);
    if (BytesAdded)
      *BytesAdded += getInstSizeInBytes(MI);
    return 1;
  }

  // Either a one or two-way conditional branch.
  auto CC = static_cast<ISD::CondCode>(Cond[0].getImm());
  MachineInstr &CondMI =
      *BuildMI(&MBB, DL, getBrCond(CC)).add(Cond[1]).add(Cond[2]).addMBB(TBB);
  if (BytesAdded)
    *BytesAdded += getInstSizeInBytes(CondMI);

  // One-way conditional branch.
  if (!FBB)
    return 1;

  // Two-way conditional branch.
  MachineInstr &MI = *BuildMI(&MBB, DL, get(Triton::PseudoJUMP)).addMBB(FBB);
  if (BytesAdded)
    *BytesAdded += getInstSizeInBytes(MI);
  return 2;
}

unsigned TritonInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                       int *BytesRemoved) const {
  if (BytesRemoved)
    *BytesRemoved = 0;
  MachineBasicBlock::iterator I = MBB.getLastNonDebugInstr();
  if (I == MBB.end())
    return 0;

  if (!I->getDesc().isUnconditionalBranch() &&
      !I->getDesc().isConditionalBranch())
    return 0;

  // Remove the branch.
  if (BytesRemoved)
    *BytesRemoved += getInstSizeInBytes(*I);
  I->eraseFromParent();

  I = MBB.end();

  if (I == MBB.begin())
    return 1;
  --I;
  if (!I->getDesc().isConditionalBranch())
    return 1;

  // Remove the branch.
  if (BytesRemoved)
    *BytesRemoved += getInstSizeInBytes(*I);
  I->eraseFromParent();
  return 2;
}

bool TritonInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  assert((Cond.size() == 3) && "Invalid branch condition!");
  auto CC = static_cast<ISD::CondCode>(Cond[0].getImm());
  Cond[0].setImm(getOppositeBranchCondition(CC));
  return false;
}

MachineBasicBlock *
TritonInstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  assert(MI.getDesc().isBranch() && "Unexpected opcode!");
  // The branch target is always the last operand.
  int NumOp = MI.getNumOperands();
  return MI.getOperand(NumOp - 1).getMBB();
}

bool TritonInstrInfo::isBranchOffsetInRange(unsigned BranchOp,
                                            int64_t BrOffset) const {
  // Ideally we could determine the supported branch offset from the
  // RISC-V ELF psABI, but this isn't defined at present.
  switch (BranchOp) {
  default:
    llvm_unreachable("Unexpected opcode!");
  case Triton::BEQ:
  case Triton::BNE:
  case Triton::BLT:
  case Triton::BGE:
  case Triton::BLTU:
  case Triton::BGEU:
    return isIntN(13, BrOffset);
  case Triton::JAL:
  case Triton::PseudoJUMP:
    return isIntN(21, BrOffset);
  }
}

// Helper functions for branch analysis
void TritonInstrInfo::parseCondBranch(
    MachineInstr &LastInst, MachineBasicBlock *&Target,
    SmallVectorImpl<MachineOperand> &Cond) const {
  // Block ends with fall-through condbranch.
  assert(LastInst.getDesc().isConditionalBranch() &&
         "Unknown conditional branch");
  Target = LastInst.getOperand(2).getMBB();
  Cond.push_back(MachineOperand::CreateImm(getBranchCondition(LastInst)));
  Cond.push_back(LastInst.getOperand(0));
  Cond.push_back(LastInst.getOperand(1));
}

ISD::CondCode TritonInstrInfo::getBranchCondition(MachineInstr &MI) const {
  switch (MI.getOpcode()) {
  default:
    llvm_unreachable("Unknown conditional branch");
  case Triton::BEQ:
    return ISD::SETEQ;
  case Triton::BNE:
    return ISD::SETNE;
  case Triton::BLT:
    return ISD::SETLT;
  case Triton::BGE:
    return ISD::SETGE;
  case Triton::BLTU:
    return ISD::SETULT;
  case Triton::BGEU:
    return ISD::SETUGE;
  }
}

const MCInstrDesc &TritonInstrInfo::getBrCond(ISD::CondCode CC) const {
  switch (CC) {
  default:
    llvm_unreachable("Unknown condition code!");
  case ISD::SETEQ:
    return get(Triton::BEQ);
  case ISD::SETNE:
    return get(Triton::BNE);
  case ISD::SETLT:
    return get(Triton::BLT);
  case ISD::SETGE:
    return get(Triton::BGE);
  case ISD::SETULT:
    return get(Triton::BLTU);
  case ISD::SETUGE:
    return get(Triton::BGEU);
  }
}

ISD::CondCode
TritonInstrInfo::getOppositeBranchCondition(ISD::CondCode CC) const {
  switch (CC) {
  default:
    llvm_unreachable("Unrecognized conditional branch");
  case ISD::SETEQ:
    return ISD::SETNE;
  case ISD::SETNE:
    return ISD::SETEQ;
  case ISD::SETLT:
    return ISD::SETGE;
  case ISD::SETGE:
    return ISD::SETLT;
  case ISD::SETULT:
    return ISD::SETUGE;
  case ISD::SETUGE:
    return ISD::SETULT;
  }
}
