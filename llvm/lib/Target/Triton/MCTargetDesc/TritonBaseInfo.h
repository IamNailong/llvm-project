//===-- TritonBaseInfo.h - Top level definitions for Triton MC -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone helper functions and enum definitions for
// the Triton target useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRITON_MCTARGETDESC_TRITONBASEINFO_H
#define LLVM_LIB_TARGET_TRITON_MCTARGETDESC_TRITONBASEINFO_H

#include "TritonMCTargetDesc.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCInstrDesc.h"
#include "llvm/MC/MCTargetOptions.h"

namespace llvm {

// TritonII - This namespace holds all of the target specific flags that
// instruction info tracks. All definitions must match TritonInstrFormats.td.
namespace TritonII {
enum {
  InstFormatPseudo = 0,
  InstFormatR = 1,
  InstFormatI = 2,
  InstFormatS = 3,
  InstFormatB = 4,
  InstFormatU = 5,
  InstFormatJ = 6,
  InstFormatOther = 31,

  InstFormatMask = 31,
  InstFormatShift = 0,
};

// enum OperandType : unsigned {
//   OPERAND_FIRST_TRITON_IMM = MCOI::OPERAND_FIRST_TARGET,
//   OPERAND_UIMM4,
//   OPERAND_UIMM5,
//   OPERAND_UIMM12,
//   OPERAND_UIMM20,
//   OPERAND_SIMM12,
//   OPERAND_SIMM13_LSB0,
//   OPERAND_SIMM21_LSB0,
//   OPERAND_LAST_TRITON_IMM = OPERAND_SIMM21_LSB0
// };

} // namespace TritonII

namespace TritonOp {
enum OperandType : unsigned {
  OPERAND_FIRST_TRITON_IMM = MCOI::OPERAND_FIRST_TARGET,
  OPERAND_UIMM4 = OPERAND_FIRST_TRITON_IMM,
  OPERAND_UIMM5,
  OPERAND_UIMM12,
  OPERAND_UIMM20,
  OPERAND_SIMM12,
  OPERAND_SIMM13_LSB0,
  OPERAND_SIMM21_LSB0,
  OPERAND_LAST_TRITON_IMM = OPERAND_SIMM21_LSB0
};
} // namespace TritonOp

// Describes the predecessor/successor bits used in FENCE instruction.
namespace TritonFenceField {
enum FenceField : unsigned {
  I = 8,
  O = 4,
  R = 2,
  W = 1
};
}

// Describes the supported floating point rounding modes.
namespace TritonFRM {
enum RoundingMode : unsigned {
  RNE = 0,
  RTZ = 1,
  RDN = 2,
  RUP = 3,
  RMM = 4,
  DYN = 7,
  InvalidRM = 8
};

inline static StringRef roundingModeToString(RoundingMode RM) {
  switch (RM) {
  default:
    llvm_unreachable("Unknown floating point rounding mode");
  case TritonFRM::RNE:
    return "rne";
  case TritonFRM::RTZ:
    return "rtz";
  case TritonFRM::RDN:
    return "rdn";
  case TritonFRM::RUP:
    return "rup";
  case TritonFRM::RMM:
    return "rmm";
  case TritonFRM::DYN:
    return "dyn";
  }
}

inline static RoundingMode stringToRoundingMode(StringRef Str) {
  return StringSwitch<RoundingMode>(Str)
      .Case("rne", TritonFRM::RNE)
      .Case("rtz", TritonFRM::RTZ)
      .Case("rdn", TritonFRM::RDN)
      .Case("rup", TritonFRM::RUP)
      .Case("rmm", TritonFRM::RMM)
      .Case("dyn", TritonFRM::DYN)
      .Default(TritonFRM::InvalidRM);
}

inline static bool isValidRoundingMode(unsigned Mode) {
  switch (Mode) {
  default:
    return false;
  case TritonFRM::RNE:
  case TritonFRM::RTZ:
  case TritonFRM::RDN:
  case TritonFRM::RUP:
  case TritonFRM::RMM:
  case TritonFRM::DYN:
    return true;
  }
}
} // namespace TritonFRM

} // namespace llvm

#endif
