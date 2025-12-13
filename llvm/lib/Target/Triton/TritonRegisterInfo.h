//===-- TritonRegisterInfo.h - Triton Register Information Impl -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Triton implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRITON_TRITONREGISTERINFO_H
#define LLVM_LIB_TARGET_TRITON_TRITONREGISTERINFO_H

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "TritonGenRegisterInfo.inc"

namespace llvm {

struct TritonRegisterInfo : public TritonGenRegisterInfo {
private:
  unsigned HwMode;

public:
  TritonRegisterInfo(unsigned HwMode);

  const uint32_t *getCallPreservedMask(const MachineFunction &MF,
                                       CallingConv::ID) const override;

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;

  BitVector getReservedRegs(const MachineFunction &MF) const override;

  bool isAsmClobberable(const MachineFunction &MF,
                        MCRegister PhysReg) const override;

  // isConstantPhysReg is final in the generated code, so we can't override it

  const TargetRegisterClass *
  getPointerRegClass(const MachineFunction &MF,
                     unsigned Kind = 0) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  Register getFrameRegister(const MachineFunction &MF) const override;

  bool requiresRegisterScavenging(const MachineFunction &MF) const override {
    return true;
  }

  bool requiresFrameIndexScavenging(const MachineFunction &MF) const override {
    return true;
  }

  bool trackLivenessAfterRegAlloc(const MachineFunction &) const override {
    return true;
  }

  bool hasBasePointer(const MachineFunction &MF) const;
  Register getBaseRegister() const;

  // Debug information queries.
  int64_t getDwarfRegNum(MCRegister RegNum, bool IsEH) const override;
};

} // end namespace llvm

#endif
