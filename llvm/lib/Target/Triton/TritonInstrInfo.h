//===-- TritonInstrInfo.h - Triton Instruction Information ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Triton implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRITON_TRITONINSTRINFO_H
#define LLVM_LIB_TARGET_TRITON_TRITONINSTRINFO_H

#include "TritonRegisterInfo.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#define GET_INSTRINFO_OPERAND_ENUM
#include "TritonGenInstrInfo.inc"

namespace llvm {

class TritonSubtarget;

class TritonInstrInfo : public TritonGenInstrInfo {
  const TritonRegisterInfo RI;
  const TritonSubtarget &Subtarget;

public:
  explicit TritonInstrInfo(const TritonSubtarget &STI);

  // getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
  // such, whenever a client has an instance of instruction info, it should
  // always be able to get register info as well (through this method).
  const TritonRegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
                   const DebugLoc &DL, Register DestReg, Register SrcReg,
                   bool KillSrc, bool RenamableDest = false,
                   bool RenamableSrc = false) const override;

  void storeRegToStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI, Register SrcReg,
      bool IsKill, int FrameIndex, const TargetRegisterClass *RC,
      const TargetRegisterInfo *TRI, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  void loadRegFromStackSlot(
      MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
      Register DestReg, int FrameIndex, const TargetRegisterClass *RC,
      const TargetRegisterInfo *TRI, Register VReg,
      MachineInstr::MIFlag Flags = MachineInstr::NoFlags) const override;

  // Materializes the given integer Val into DstReg.
  void movImm(MachineBasicBlock &MBB, MachineBasicBlock::iterator MBBI,
              const DebugLoc &DL, Register DstReg, uint64_t Val,
              MachineInstr::MIFlag Flag = MachineInstr::NoFlags) const;

  unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const override;

  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;

  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;

  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;

  bool isBranchOffsetInRange(unsigned BranchOpc,
                             int64_t BrOffset) const override;

protected:
  const TritonSubtarget &getSubtarget() const { return Subtarget; }

private:
  // Helper functions for branch analysis
  void parseCondBranch(MachineInstr &LastInst, MachineBasicBlock *&Target,
                       SmallVectorImpl<MachineOperand> &Cond) const;
  ISD::CondCode getBranchCondition(MachineInstr &MI) const;
  const MCInstrDesc &getBrCond(ISD::CondCode CC) const;
  ISD::CondCode getOppositeBranchCondition(ISD::CondCode CC) const;
};

} // end namespace llvm
#endif // LLVM_LIB_TARGET_TRITON_TRITONINSTRINFO_H
