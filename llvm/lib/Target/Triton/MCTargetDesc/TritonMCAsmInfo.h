//===-- TritonMCAsmInfo.h - Triton Asm Info --------------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the TritonMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRITON_MCTARGETDESC_TRITONMCASMINFO_H
#define LLVM_LIB_TARGET_TRITON_MCTARGETDESC_TRITONMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class TritonMCAsmInfo : public MCAsmInfoELF {
public:
  explicit TritonMCAsmInfo(const Triple &TT);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_TRITON_MCTARGETDESC_TRITONMCASMINFO_H
