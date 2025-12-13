//===-- TritonSubtarget.cpp - Triton Subtarget Information ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the Triton specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "TritonSubtarget.h"
#include "Triton.h"
#include "TritonFrameLowering.h"
#include "TritonISelLowering.h"
#include "TritonInstrInfo.h"
#include "TritonTargetMachine.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/TargetParser/TargetParser.h"

using namespace llvm;

#define DEBUG_TYPE "triton-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "TritonGenSubtargetInfo.inc"

void TritonSubtarget::anchor() {}

TritonSubtarget::TritonSubtarget(const Triple &TT, StringRef CPU,
                                 StringRef TuneCPU, StringRef FS,
                                 StringRef ABIName, const TargetMachine &TM)
    : TritonGenSubtargetInfo(TT, CPU, TuneCPU, FS),
      InstrInfo(*this), RegInfo(getHwMode()), TLInfo(TM, *this),
      FrameLowering(*this) {

  // Parse features string and set the CPU.
  ParseSubtargetFeatures(CPU, TuneCPU, FS);
}
