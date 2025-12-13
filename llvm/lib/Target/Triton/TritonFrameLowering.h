//===-- TritonFrameLowering.h - Define frame lowering for Triton -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class implements Triton-specific bits of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRITON_TRITONFRAMELOWERING_H
#define LLVM_LIB_TARGET_TRITON_TRITONFRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {
class TritonSubtarget;

class TritonFrameLowering : public TargetFrameLowering {
  const TritonSubtarget &STI;

public:
  explicit TritonFrameLowering(const TritonSubtarget &STI)
      : TargetFrameLowering(StackGrowsDown,
                            /*StackAlignment=*/Align(4),
                            /*LocalAreaOffset=*/0),
        STI(STI) {}

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;

  bool hasFP(const MachineFunction &MF) const;

  void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs,
                            RegScavenger *RS = nullptr) const override;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF,
                                           RegScavenger *RS) const override;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 ArrayRef<CalleeSavedInfo> CSI,
                                 const TargetRegisterInfo *TRI) const override;

  bool
  restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator MI,
                              MutableArrayRef<CalleeSavedInfo> CSI,
                              const TargetRegisterInfo *TRI) const override;

  // Return true if the specified function should have a dedicated frame
  // pointer register.
  bool hasFPImpl(const MachineFunction &MF) const override;

  bool hasReservedCallFrame(const MachineFunction &MF) const override;

  StackOffset getFrameIndexReference(const MachineFunction &MF, int FI,
                                     Register &FrameReg) const override;

  void determineFrameLayout(MachineFunction &MF) const;

  static Register getFPReg(const TritonSubtarget &STI);
  static Register getSPReg(const TritonSubtarget &STI);

private:
  int64_t alignSPAdjust(int64_t SPAdj) const;
};

} // End llvm namespace

#endif
