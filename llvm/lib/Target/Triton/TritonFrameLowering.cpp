//===-- TritonFrameLowering.cpp - Triton Frame Information ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier:he-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the Triton implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "TritonFrameLowering.h"
#include "TritonInstrInfo.h"
#include "TritonMachineFunctionInfo.h"
#include "TritonSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/MC/MCDwarf.h"

using namespace llvm;

bool TritonFrameLowering::hasFP(const MachineFunction &MF) const {
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         MF.getFrameInfo().hasVarSizedObjects() ||
         MF.getFrameInfo().isFrameAddressTaken();
}

bool TritonFrameLowering::hasFPImpl(const MachineFunction &MF) const {
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
         RegInfo->hasStackRealignment(MF) || MFI.hasVarSizedObjects() ||
         MFI.isFrameAddressTaken();
}

bool TritonFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  return !MF.getFrameInfo().hasVarSizedObjects();
}

// Eliminate ADJCALLSTACKDOWN, ADJCALLSTACKUP pseudo instructions.
MachineBasicBlock::iterator TritonFrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  Register SPReg = Triton::X2;
  DebugLoc DL = MI->getDebugLoc();

  if (!hasReservedCallFrame(MF)) {
    // If space has not been reserved for a call frame, ADJCALLSTACKDOWN and
    // ADJCALLSTACKUP must be converted to instructions manipulating the stack
    // pointer. This is necessary when there is a variable length stack
    // allocation (e.g. alloca), which means it's not possible to allocate
    // space for outgoing arguments from within the function prologue.
    int64_t Amount = MI->getOperand(0).getImm();

    if (Amount != 0) {
      // Ensure the stack remains aligned after adjustment.
      Amount = alignSPAdjust(Amount);

      if (MI->getOpcode() == Triton::ADJCALLSTACKDOWN)
        Amount = -Amount;

      const TritonInstrInfo *TII = STI.getInstrInfo();
      // Adjust stack pointer - simplified implementation
      if (Amount > 0) {
        BuildMI(MBB, MI, DL, TII->get(Triton::ADDI), SPReg)
            .addReg(SPReg).addImm(-Amount);
      } else {
        BuildMI(MBB, MI, DL, TII->get(Triton::ADDI), SPReg)
            .addReg(SPReg).addImm(-Amount);
      }
    }
  }

  return MBB.erase(MI);
}

void TritonFrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto *TritonFI = MF.getInfo<TritonMachineFunctionInfo>();
  const TritonInstrInfo *TII = STI.getInstrInfo();
  const TritonRegisterInfo *RegInfo = STI.getRegisterInfo();

  MachineBasicBlock::iterator MBBI = MBB.begin();

  Register FPReg = getFPReg(STI);
  Register SPReg = getSPReg(STI);

  // Debug location must be unknown since the first debug location is used
  // to determine the end of the prologue.
  DebugLoc DL;

  // Determine the correct frame layout
  determineFrameLayout(MF);

  // FIXME (note copied from Lanai): This appears to be overallocating.  Needs
  // investigation. Get the number of bytes to allocate from the FrameInfo.
  uint64_t StackSize = MFI.getStackSize();
  uint64_t RealStackSize = StackSize + TritonFI->getLibCallStackSize();
  TritonFI->setVarArgsSaveSize(0);

  // Early exit if there is no need to allocate on the stack
  if (RealStackSize == 0 && !MFI.adjustsStack())
    return;

  // Allocate space on the stack if necessary.
  if (RealStackSize > 0) {
    BuildMI(MBB, MBBI, DL, TII->get(Triton::ADDI), SPReg)
        .addReg(SPReg).addImm(-RealStackSize)
        .setMIFlag(MachineInstr::FrameSetup);
  }

  // Emit ".cfi_def_cfa_offset RealStackSize"
  unsigned CFIIndex = MF.addFrameInst(
      MCCFIInstruction::cfiDefCfaOffset(nullptr, RealStackSize));
  BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
      .addCFIIndex(CFIIndex)
      .setMIFlag(MachineInstr::FrameSetup);

  const auto &CSI = MFI.getCalleeSavedInfo();

  // The frame pointer is callee-save, and code has been generated for us to
  // save it to the stack. We need to skip over the storing of callee-saved
  // registers as the frame pointer must be modified after it has been saved
  // to the stack, not before.
  // FIXME: assumes exactly one instruction is used to save each callee-saved
  // register.
  std::advance(MBBI, CSI.size());

  // Generate new FP.
  if (hasFP(MF)) {
    if (RealStackSize < 2048) {
      BuildMI(MBB, MBBI, DL, TII->get(Triton::ADDI), FPReg)
          .addReg(SPReg)
          .addImm(RealStackSize)
          .setMIFlag(MachineInstr::FrameSetup);
    } else {
      // The frame pointer cannot be generated with a single addi instruction.
      // We need to break this down into one or more instructions.
      TII->movImm(MBB, MBBI, DL, FPReg, RealStackSize,
                  MachineInstr::FrameSetup);
      BuildMI(MBB, MBBI, DL, TII->get(Triton::ADD), FPReg)
          .addReg(SPReg)
          .addReg(FPReg, RegState::Kill)
          .setMIFlag(MachineInstr::FrameSetup);
    }

    // Emit ".cfi_def_cfa $fp, 0"
    unsigned CFIIndex = MF.addFrameInst(MCCFIInstruction::cfiDefCfa(
        nullptr, RegInfo->getDwarfRegNum(FPReg, true), 0));
    BuildMI(MBB, MBBI, DL, TII->get(TargetOpcode::CFI_INSTRUCTION))
        .addCFIIndex(CFIIndex)
        .setMIFlag(MachineInstr::FrameSetup);
  }

  // Emit the second SP adjustment after saving callee saved registers.
  if (hasReservedCallFrame(MF) && RealStackSize > 2048) {
    const uint64_t CalleeStackSize = TritonFI->getLibCallStackSize();
    // Reserve space for outgoing function arguments.
    if (CalleeStackSize > 0) {
      BuildMI(MBB, MBBI, DL, TII->get(Triton::ADDI), SPReg)
          .addReg(SPReg).addImm(-CalleeStackSize)
          .setMIFlag(MachineInstr::FrameSetup);
    }
  }
}

void TritonFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
  const TritonRegisterInfo *RegInfo = STI.getRegisterInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto *TritonFI = MF.getInfo<TritonMachineFunctionInfo>();
  const TritonInstrInfo *TII = STI.getInstrInfo();

  Register FPReg = getFPReg(STI);
  Register SPReg = getSPReg(STI);

  // Get the insert location for the epilogue. If there were no terminators in
  // the block, get the last instruction.
  MachineBasicBlock::iterator MBBI = MBB.end();
  DebugLoc DL;
  if (!MBB.empty()) {
    MBBI = MBB.getLastNonDebugInstr();
    if (MBBI != MBB.end())
      DL = MBBI->getDebugLoc();

    MBBI = MBB.getFirstTerminator();

    // If callee-saved registers are saved via libcall, place stack adjustment
    // before this call.
    while (MBBI != MBB.begin() &&
           std::prev(MBBI)->getFlag(MachineInstr::FrameDestroy))
      --MBBI;
  }

  const auto &CSI = MFI.getCalleeSavedInfo();

  // Skip to before the restores of callee-saved registers
  // FIXME: assumes exactly one instruction is used to restore each
  // callee-saved register.
  auto LastFrameDestroy = MBBI;
  if (!CSI.empty())
    LastFrameDestroy = std::prev(MBBI, CSI.size());

  uint64_t StackSize = MFI.getStackSize();
  uint64_t RealStackSize = StackSize + TritonFI->getLibCallStackSize();

  // Restore the stack pointer using the value of the frame pointer. Only
  // necessary if the stack pointer was modified, meaning the stack size is
  // unknown.
  if (RegInfo->hasStackRealignment(MF) || MFI.hasVarSizedObjects()) {
    assert(hasFP(MF) && "frame pointer should not have been eliminated");
    if (RealStackSize > 0) {
      BuildMI(MBB, LastFrameDestroy, DL, TII->get(Triton::ADDI), SPReg)
          .addReg(FPReg).addImm(-RealStackSize)
          .setMIFlag(MachineInstr::FrameDestroy);
    }
  } else {
    if (RealStackSize != 0) {
      if (RealStackSize > 0) {
        BuildMI(MBB, MBBI, DL, TII->get(Triton::ADDI), SPReg)
            .addReg(SPReg).addImm(RealStackSize)
            .setMIFlag(MachineInstr::FrameDestroy);
      }
    }
  }
}

void TritonFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                               BitVector &SavedRegs,
                                               RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  // Unconditionally spill RA and FP only if the function uses a frame
  // pointer.
  if (hasFP(MF)) {
    SavedRegs.set(Triton::X1);
    SavedRegs.set(Triton::X8);
  }
  // Mark BP as used if function has dedicated base pointer.
  const TritonRegisterInfo *RegInfo = STI.getRegisterInfo();
  if (RegInfo->hasBasePointer(MF))
    SavedRegs.set(RegInfo->getBaseRegister());
}

void TritonFrameLowering::processFunctionBeforeFrameFinalized(
    MachineFunction &MF, RegScavenger *RS) const {
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const TritonInstrInfo *TII = STI.getInstrInfo();
  unsigned FPReg = getFPReg(STI);
  // Base pointer is not used in Triton

  if (!RegInfo->hasStackRealignment(MF))
    return;

  // Find the maximum stack alignment.
  Align MaxStackAlign;
  for (int i = MFI.getObjectIndexBegin(); i != 0; ++i)
    MaxStackAlign = std::max(MaxStackAlign, MFI.getObjectAlign(i));
  for (unsigned i = 0, e = MFI.getNumObjects(); i != e; ++i)
    MaxStackAlign = std::max(MaxStackAlign, MFI.getObjectAlign(i));

  if (hasFP(MF)) {
    // The presence of a frame pointer means that the stack has been realigned
    // and the BP is used as a base pointer for this realigned stack. Unaligned
    // memory accesses are then performed relative to the BP.
    // Stack realignment handling
    int64_t FPOffset = MFI.getStackSize() - getOffsetOfLocalArea();
    FPOffset = alignTo(FPOffset, MaxStackAlign);
    MFI.setOffsetAdjustment(-FPOffset);
  } else {
    // If we don't have a FP, update the offset for stack alignment
    if (!hasFP(MF)) {
      int64_t Offset = MFI.getStackSize();
      Offset = alignTo(Offset, MaxStackAlign);
      MFI.setOffsetAdjustment(-Offset);
    }
  }
}

// Not preserve stack space within prologue for outgoing variables when the
// function contains variable size objects and let eliminateCallFramePseudoInstr
// preserve stack space for it.
bool TritonFrameLowering::spillCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    ArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return true;

  MachineFunction *MF = MBB.getParent();
  const TritonInstrInfo &TII = *STI.getInstrInfo();
  DebugLoc DL;
  if (MI != MBB.end() && !MI->isDebugInstr())
    DL = MI->getDebugLoc();

  for (auto &CS : CSI) {
    // Insert the spill to the stack frame.
    Register Reg = CS.getReg();
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.storeRegToStackSlot(MBB, MI, Reg, true, CS.getFrameIdx(), RC, TRI, 0);
  }

  return true;
}

bool TritonFrameLowering::restoreCalleeSavedRegisters(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
    MutableArrayRef<CalleeSavedInfo> CSI, const TargetRegisterInfo *TRI) const {
  if (CSI.empty())
    return true;

  MachineFunction *MF = MBB.getParent();
  const TritonInstrInfo &TII = *STI.getInstrInfo();
  DebugLoc DL;
  if (MI != MBB.end() && !MI->isDebugInstr())
    DL = MI->getDebugLoc();

  for (auto &CS : reverse(CSI)) {
    Register Reg = CS.getReg();
    const TargetRegisterClass *RC = TRI->getMinimalPhysRegClass(Reg);
    TII.loadRegFromStackSlot(MBB, MI, Reg, CS.getFrameIdx(), RC, TRI, 0);
    assert(MI != MBB.begin() && "loadRegFromStackSlot didn't insert any code!");
  }

  return true;
}

void TritonFrameLowering::determineFrameLayout(MachineFunction &MF) const {
  MachineFrameInfo &MFI = MF.getFrameInfo();
  auto *TritonFI = MF.getInfo<TritonMachineFunctionInfo>();

  // Get the number of bytes to allocate from the FrameInfo.
  uint64_t FrameSize = MFI.getStackSize();

  // Get the alignment.
  Align StackAlign = getStackAlign();
  if (MFI.getMaxAlign() > StackAlign)
    StackAlign = MFI.getMaxAlign();

  // Set Max Call Frame Size
  uint64_t MaxCallSize = alignTo(MFI.getMaxCallFrameSize(), StackAlign);
  MFI.setMaxCallFrameSize(MaxCallSize);

  // Make sure the frame is aligned.
  FrameSize = alignTo(FrameSize, StackAlign);

  // Update frame info.
  MFI.setStackSize(FrameSize);
}

StackOffset TritonFrameLowering::getFrameIndexReference(const MachineFunction &MF,
                                                         int FI,
                                                         Register &FrameReg) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  const TargetRegisterInfo *RI = MF.getSubtarget().getRegisterInfo();
  const auto *TritonFI = MF.getInfo<TritonMachineFunctionInfo>();

  // Callee-saved registers should be referenced relative to the stack
  // pointer (positive offset), otherwise use the frame pointer (negative
  // offset).
  const auto &CSI = MFI.getCalleeSavedInfo();
  int MinCSFI = 0;
  int MaxCSFI = -1;

  int64_t Offset = MFI.getObjectOffset(FI) - getOffsetOfLocalArea() +
                   MFI.getOffsetAdjustment();

  if (CSI.size()) {
    MinCSFI = CSI[0].getFrameIdx();
    MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
  }

  if (FI >= MinCSFI && FI <= MaxCSFI) {
    FrameReg = Triton::X2;
    Offset += MFI.getStackSize();
  } else if (RI->hasStackRealignment(MF) && !MFI.isFixedObjectIndex(FI)) {
    // If the stack was realigned, the frame pointer is set in order to allow
    // SP to be restored back to the original value, but offsets containing
    // objects that extend the stack are not scaled back.
    // Scale the offset by the realignment factor.
    assert(hasFP(MF) && "frame pointer should not have been eliminated");
    FrameReg = getFPReg(STI);
    Offset += TritonFI->getLibCallStackSize();
  } else {
    FrameReg = RI->getFrameRegister(MF);
    if (hasFP(MF))
      Offset += TritonFI->getLibCallStackSize();
  }

  return StackOffset::getFixed(Offset);
}

Register TritonFrameLowering::getFPReg(const TritonSubtarget &STI) {
  return Triton::X8;
}

Register TritonFrameLowering::getSPReg(const TritonSubtarget &STI) {
  return Triton::X2;
}

int64_t TritonFrameLowering::alignSPAdjust(int64_t SPAdj) const {
  // Round up to next multiple of stack alignment.
  return alignTo(SPAdj, getStackAlign());
}
