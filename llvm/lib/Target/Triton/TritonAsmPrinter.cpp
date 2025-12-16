//===-- TritonAsmPrinter.cpp - Triton LLVM Assembly Printer ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to GAS-format Triton assembly language.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TritonInstPrinter.h"
#include "MCTargetDesc/TritonMCTargetDesc.h"
#include "TargetInfo/TritonTargetInfo.h"
#include "Triton.h"
#include "TritonInstrInfo.h"
#include "TritonTargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "asm-printer"

namespace {
class TritonAsmPrinter : public AsmPrinter {
public:
  explicit TritonAsmPrinter(TargetMachine &TM,
                            std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  StringRef getPassName() const override { return "Triton Assembly Printer"; }

  void emitInstruction(const MachineInstr *MI) override;

  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                   const MachineInstr *MI);
};
} // end anonymous namespace

void TritonAsmPrinter::emitInstruction(const MachineInstr *MI) {
  // Do any auto-generated pseudo lowerings.
  if (emitPseudoExpansionLowering(*OutStreamer, MI))
    return;

  MCInst TmpInst;
  LowerTritonMachineInstrToMCInst(MI, TmpInst, *this);
  EmitToStreamer(*OutStreamer, TmpInst);
}

bool TritonAsmPrinter::emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                                   const MachineInstr *MI) {
  switch (MI->getOpcode()) {
  default:
    return false;
  case Triton::PseudoRET:
    // Expand PseudoRET to JALR x0, x1, 0
    {
      MCInst TmpInst;
      TmpInst.setOpcode(Triton::JALR);
      TmpInst.addOperand(MCOperand::createReg(Triton::X0));
      TmpInst.addOperand(MCOperand::createReg(Triton::X1));
      TmpInst.addOperand(MCOperand::createImm(0));
      EmitToStreamer(OutStreamer, TmpInst);
      return true;
    }
  }
}

// Force static initialization.
extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTritonAsmPrinter() {
  RegisterAsmPrinter<TritonAsmPrinter> X(getTheTritonTarget());
}
