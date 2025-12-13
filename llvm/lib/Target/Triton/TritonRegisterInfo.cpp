//===-- TritonRegisterInfo.cpp - Triton Register Information -------------===//
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

#include "TritonRegisterInfo.h"
#include "Triton.h"
#include "TritonFrameLowering.h"
#include "TritonSubtarget.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "TritonGenRegisterInfo.inc"

TritonRegisterInfo::TritonRegisterInfo(unsigned HwMode)
    : TritonGenRegisterInfo(Triton::X1, /*DwarfFlavour*/0, /*EHFlavor*/0,
                            /*PC*/0, HwMode), HwMode(HwMode) {}

const MCPhysReg *
TritonRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  auto &Subtarget = MF->getSubtarget<TritonSubtarget>();

  return CSR_ILP32_SaveList;
}

BitVector TritonRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  const TritonFrameLowering *TFI = getFrameLowering(MF);
  BitVector Reserved(getNumRegs());

  // Mark any registers requested to be reserved as such
  for (size_t Reg = 0; Reg < getNumRegs(); Reg++) {
    if (MF.getSubtarget<TritonSubtarget>().isRegisterReservedByUser(Reg))
      markSuperRegs(Reserved, Reg);
  }

  // Use markSuperRegs to ensure any register aliases are also reserved
  markSuperRegs(Reserved, Triton::X0); // zero
  markSuperRegs(Reserved, Triton::X2); // sp
  markSuperRegs(Reserved, Triton::X3); // gp
  markSuperRegs(Reserved, Triton::X4); // tp
  if (TFI->hasFP(MF))
    markSuperRegs(Reserved, Triton::X8); // fp
  // Reserve the base pointer if we have one.
  if (hasBasePointer(MF))
    markSuperRegs(Reserved, getBaseRegister());

  assert(checkAllSuperRegsMarked(Reserved));
  return Reserved;
}

bool TritonRegisterInfo::isAsmClobberable(const MachineFunction &MF,
                                          MCRegister PhysReg) const {
  return !getReservedRegs(MF).test(PhysReg);
}

// isConstantPhysReg is implemented in the generated code

const TargetRegisterClass *
TritonRegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                       unsigned Kind) const {
  return &Triton::GPRRegClass;
}

bool TritonRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                             int SPAdj, unsigned FIOperandNum,
                                             RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected non-zero SPAdj value");

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TritonInstrInfo *TII = MF.getSubtarget<TritonSubtarget>().getInstrInfo();
  const TritonFrameLowering *TFI = getFrameLowering(MF);
  DebugLoc DL = MI.getDebugLoc();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  Register FrameReg;
  StackOffset StackOffset = TFI->getFrameIndexReference(MF, FrameIndex, FrameReg);
  int Offset = StackOffset.getFixed() + MI.getOperand(FIOperandNum + 1).getImm();

  if (!isInt<12>(Offset)) {
    // The offset won't fit in an immediate, so use a scratch register instead
    // Ideally we'd use a callee-saved register, but in an interrupt
    // handler that might not be available.
    Register ScratchReg = MRI.createVirtualRegister(&Triton::GPRRegClass);
    TII->movImm(*MI.getParent(), II, DL, ScratchReg, Offset);
    BuildMI(*MI.getParent(), II, DL, TII->get(Triton::ADD), ScratchReg)
        .addReg(FrameReg).addReg(ScratchReg, RegState::Kill);
    MI.getOperand(FIOperandNum).ChangeToRegister(ScratchReg, false, false, true);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(0);
  } else {
    MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, false);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
  }
  return false;
}

Register TritonRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TritonFrameLowering *TFI = getFrameLowering(MF);
  return TFI->hasFP(MF) ? Triton::X8 : Triton::X2;
}

const uint32_t *
TritonRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                         CallingConv::ID /*CC*/) const {
  auto &Subtarget = MF.getSubtarget<TritonSubtarget>();

  return CSR_ILP32_RegMask;
}

bool TritonRegisterInfo::hasBasePointer(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();

  // When we need stack realignment, we can't address the stack from the frame
  // pointer.  When we have dynamic allocas or stack-adjusting inline asm, we
  // can't address variables from the stack pointer.  MS inline asm can also
  // change the stack pointer, but it's rare, so we use a heuristic.
  return MFI.hasVarSizedObjects() ||
         (MF.hasInlineAsm() && MFI.hasStackMap());
}

Register TritonRegisterInfo::getBaseRegister() const { return Triton::X9; }

int64_t TritonRegisterInfo::getDwarfRegNum(MCRegister RegNum, bool IsEH) const {
  return RegNum;
}
