//===-- TritonMCInstLower.cpp - Convert Triton MachineInstr to MCInst -----===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower Triton MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "Triton.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static MCOperand LowerOperand(const MachineOperand &MO, AsmPrinter &AP) {
  switch (MO.getType()) {
  default:
    llvm_unreachable("unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return MCOperand();
    return MCOperand::createReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm());
  case MachineOperand::MO_MachineBasicBlock:
    return MCOperand::createExpr(
        MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), AP.OutContext));
  case MachineOperand::MO_GlobalAddress:
    return MCOperand::createExpr(
        MCSymbolRefExpr::create(AP.getSymbol(MO.getGlobal()), AP.OutContext));
  case MachineOperand::MO_ExternalSymbol:
    return MCOperand::createExpr(MCSymbolRefExpr::create(
        AP.GetExternalSymbolSymbol(MO.getSymbolName()), AP.OutContext));
  case MachineOperand::MO_RegisterMask:
    // Ignore register masks.
    return MCOperand();
  case MachineOperand::MO_FrameIndex:
    // Frame indices should have been eliminated by this point.
    llvm_unreachable("Frame index operand should have been eliminated");
  }
}

void llvm::LowerTritonMachineInstrToMCInst(const MachineInstr *MI,
                                           MCInst &OutMI, AsmPrinter &AP) {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp = LowerOperand(MO, AP);
    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}
