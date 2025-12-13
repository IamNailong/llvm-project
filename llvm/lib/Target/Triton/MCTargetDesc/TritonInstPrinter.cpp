//===-- TritonInstPrinter.cpp - Convert Triton MCInst to asm syntax -----===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class prints an Triton MCInst to a .s file.
//
//===----------------------------------------------------------------------===//

#include "TritonInstPrinter.h"
#include "TritonMCTargetDesc.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Format.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

// Include the auto-generated portion of the assembly writer.
#define PRINT_ALIAS_INSTR
#include "TritonGenAsmWriter.inc"

void TritonInstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                  StringRef Annot, const MCSubtargetInfo &STI,
                                  raw_ostream &O) {
  printInstruction(MI, Address, O);
  printAnnotation(O, Annot);
}

void TritonInstPrinter::printRegName(raw_ostream &O, MCRegister Reg) {
  O << getRegisterName(Reg);
}

void TritonInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                     raw_ostream &O,
                                     const char *Modifier) {
  assert((Modifier == 0 || Modifier[0] == 0) && "No modifiers supported");
  const MCOperand &MO = MI->getOperand(OpNo);

  if (MO.isReg()) {
    printRegName(O, MO.getReg());
    return;
  }

  if (MO.isImm()) {
    O << MO.getImm();
    return;
  }

  assert(MO.isExpr() && "Unknown operand kind in printOperand");
  MAI.printExpr(O, *MO.getExpr());
}

void TritonInstPrinter::printBranchOperand(const MCInst *MI, uint64_t Address,
                                           unsigned OpNo, raw_ostream &O) {
  const MCOperand &MO = MI->getOperand(OpNo);
  if (MO.isImm()) {
    int64_t Imm = MO.getImm();
    O << formatHex(Address + Imm);
  } else {
    assert(MO.isExpr() && "Unknown branch operand kind in printBranchOperand");
    MAI.printExpr(O, *MO.getExpr());
  }
}
