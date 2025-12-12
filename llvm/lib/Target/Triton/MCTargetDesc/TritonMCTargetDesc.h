//===-- TritonMCTargetDesc.h - Triton Target Descriptions -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides Triton specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRITON_MCTARGETDESC_TRITONMCTARGETDESC_H
#define LLVM_LIB_TARGET_TRITON_MCTARGETDESC_TRITONMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"
#include <memory>

namespace llvm {

class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class StringRef;
class Target;
class raw_ostream;

extern Target TheTritonTarget;

MCCodeEmitter *createTritonMCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx);

MCAsmBackend *createTritonMCAsmBackend(const Target &T,
                                       const MCSubtargetInfo &STI,
                                       const MCRegisterInfo &MRI,
                                       const MCTargetOptions &Options);
std::unique_ptr<MCObjectTargetWriter>
createTritonObjectWriter(uint8_t OSABI, bool IsLittleEndian);

} // end namespace llvm

// Defines symbolic names for Triton registers.
// This defines a mapping from register name to register number.
#define GET_REGINFO_ENUM
#include "TritonGenRegisterInfo.inc"

// Defines symbolic names for the Triton instructions.
#define GET_INSTRINFO_ENUM
#include "TritonGenInstrInfo.inc"

#endif // LLVM_LIB_TARGET_TRITON_MCTARGETDESC_TRITONMCTARGETDESC_H
