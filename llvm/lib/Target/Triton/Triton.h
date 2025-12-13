//===-- Triton.h - Top-level interface for Triton representation --*- C++
//-*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// Triton back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRITON_TRITON_H
#define LLVM_LIB_TARGET_TRITON_TRITON_H

#include "MCTargetDesc/TritonBaseInfo.h"
#include "llvm/CodeGen/ISDOpcodes.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class TritonTargetMachine;
class AsmPrinter;
class FunctionPass;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineOperand;
class PassRegistry;

namespace TritonISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET_GLUE,
  CALL,
  SELECT_CC,
  BR_CC,
  HI,
  ADD_LO,
};
}

void LowerTritonMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                     AsmPrinter &AP);

FunctionPass *createTritonISelDag(TritonTargetMachine &TM,
                                  CodeGenOptLevel OptLevel);

void initializeTritonDAGToDAGISelLegacyPass(PassRegistry &);

} // end namespace llvm

#endif // LLVM_LIB_TARGET_TRITON_TRITON_H
