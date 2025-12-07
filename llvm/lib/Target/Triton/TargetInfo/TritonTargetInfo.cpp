//===-- TritonTargetInfo.cpp - Triton Target Implementation ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/TritonTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheTritonTarget() {
  static Target TheTritonTarget;
  return TheTritonTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTritonTargetInfo() {
  RegisterTarget<Triple::triton> X(getTheTritonTarget(), "triton", "Triton 32",
                                   "TRITON");
}
