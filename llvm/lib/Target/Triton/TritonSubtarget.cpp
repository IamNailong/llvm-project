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
#include "llvm/CodeGen/MachineScheduler.h"
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

void TritonSubtarget::overrideSchedPolicy(MachineSchedPolicy &Policy,
                                          unsigned NumRegionInstrs) const {
  // Enable bidirectional scheduling for better scheduling quality.
  // This allows the scheduler to work from both top and bottom of the
  // scheduling region, converging in the middle for optimal results.
  Policy.OnlyTopDown = false;
  Policy.OnlyBottomUp = false;

  // Enable register pressure tracking to help the scheduler make
  // decisions that minimize spilling. This is important for Triton
  // as register pressure can significantly impact performance.
  Policy.ShouldTrackPressure = true;

  LLVM_DEBUG(dbgs() << "Triton scheduling policy: bidirectional, "
                    << "pressure tracking enabled, "
                    << "region size = " << NumRegionInstrs << "\n");
}
