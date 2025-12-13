//===- TritonMachineFunctionInfo.h - Triton machine function info -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares Triton-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRITON_TRITONMACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_TRITON_TRITONMACHINEFUNCTIONINFO_H

#include "TritonSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// TritonMachineFunctionInfo - This class is derived from MachineFunctionInfo
/// and contains private Triton-specific information for each MachineFunction.
class TritonMachineFunctionInfo : public MachineFunctionInfo {
private:
  /// FrameIndex for start of varargs area
  int VarArgsFrameIndex = 0;
  /// Size of the save area used for varargs
  int VarArgsSaveSize = 0;
  /// FrameIndex used for transferring values between 64-bit FPRs and a pair
  /// of 32-bit GPRs via the stack.
  int MoveF64FrameIndex = -1;
  /// Size of any opaque stack adjustment due to save/restore libcalls.
  unsigned LibCallStackSize = 0;

public:
  TritonMachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI)
      : MachineFunctionInfo() {}

  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
  void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

  unsigned getVarArgsSaveSize() const { return VarArgsSaveSize; }
  void setVarArgsSaveSize(int Size) { VarArgsSaveSize = Size; }

  int getMoveF64FrameIndex(MachineFunction &MF) {
    if (MoveF64FrameIndex == -1)
      MoveF64FrameIndex = MF.getFrameInfo().CreateStackObject(8, Align(8), false);
    return MoveF64FrameIndex;
  }

  unsigned getLibCallStackSize() const { return LibCallStackSize; }
  void setLibCallStackSize(unsigned Size) { LibCallStackSize = Size; }

  bool useSaveRestoreLibCalls(const MachineFunction &MF) const {
    // We cannot use fixed locations for the callee saved spill slots if the
    // function uses a varargs save area, or is an interrupt handler.
    return MF.getSubtarget<TritonSubtarget>().enableSaveRestore() &&
           VarArgsSaveSize == 0 && !MF.getFrameInfo().hasTailCall() &&
           !MF.getFunction().hasFnAttribute("interrupt");
  }
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_TRITON_TRITONMACHINEFUNCTIONINFO_H
