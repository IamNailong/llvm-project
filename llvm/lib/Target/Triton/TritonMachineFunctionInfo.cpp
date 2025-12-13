//===-- TritonMachineFunctionInfo.cpp - Triton machine function info -----===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements Triton-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#include "TritonMachineFunctionInfo.h"

using namespace llvm;

MachineFunctionInfo *TritonMachineFunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  return DestMF.cloneInfo<TritonMachineFunctionInfo>(*this);
}
