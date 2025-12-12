//===-- TritonMCTargetDesc.cpp - Triton target descriptions ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#include "TritonMCTargetDesc.h"
#include "TargetInfo/TritonTargetInfo.h"
#include "TritonInstPrinter.h"
#include "TritonMCAsmInfo.h"
#include "TritonMCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_INSTRINFO_MC_DESC
#include "TritonGenInstrInfo.inc"

#define GET_REGINFO_MC_DESC
#include "TritonGenRegisterInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "TritonGenSubtargetInfo.inc"

using namespace llvm;

static MCAsmInfo *createTritonMCAsmInfo(const MCRegisterInfo &MRI,
                                        const Triple &TT,
                                        const MCTargetOptions &Options) {
  MCAsmInfo *MAI = new TritonMCAsmInfo(TT);
  return MAI;
}

static MCInstrInfo *createTritonMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitTritonMCInstrInfo(X);
  return X;
}

static MCInstPrinter *createTritonMCInstPrinter(const Triple &TT,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI) {
  return new TritonInstPrinter(MAI, MII, MRI);
}

static MCRegisterInfo *createTritonMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitTritonMCRegisterInfo(X, Triton::X0);
  return X;
}

static MCSubtargetInfo *
createTritonMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createTritonMCSubtargetInfoImpl(TT, CPU, CPU, FS);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTritonTargetMC() {
  // Register the MCAsmInfo.
  TargetRegistry::RegisterMCAsmInfo(getTheTritonTarget(),
                                    createTritonMCAsmInfo);

  // Register the MCCodeEmitter.
  TargetRegistry::RegisterMCCodeEmitter(getTheTritonTarget(),
                                        createTritonMCCodeEmitter);

  // Register the MCInstrInfo.
  TargetRegistry::RegisterMCInstrInfo(getTheTritonTarget(),
                                      createTritonMCInstrInfo);

  TargetRegistry::RegisterMCInstPrinter(getTheTritonTarget(),
                                        createTritonMCInstPrinter);
  // Register the MCRegisterInfo.
  TargetRegistry::RegisterMCRegInfo(getTheTritonTarget(),
                                    createTritonMCRegisterInfo);

  // Register the MCSubtargetInfo.
  TargetRegistry::RegisterMCSubtargetInfo(getTheTritonTarget(),
                                          createTritonMCSubtargetInfo);

  // Register the MCAsmBackend.
  TargetRegistry::RegisterMCAsmBackend(getTheTritonTarget(),
                                       createTritonMCAsmBackend);
}
