//===-- TritonMCObjectWriter.cpp - Triton ELF writer ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TritonMCAsmInfo.h"
#include "MCTargetDesc/TritonMCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

namespace {
class TritonObjectWriter : public MCELFObjectTargetWriter {
public:
  TritonObjectWriter(uint8_t OSABI);

  virtual ~TritonObjectWriter();

protected:
  unsigned getRelocType(const MCFixup &, const MCValue &,
                        bool IsPCRel) const override;
  bool needsRelocateWithSymbol(const MCValue &, unsigned Type) const override;
};
} // namespace

TritonObjectWriter::TritonObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(false, OSABI, ELF::EM_TRITON,
                              /*HasRelocationAddend=*/true) {}

TritonObjectWriter::~TritonObjectWriter() {}

unsigned TritonObjectWriter::getRelocType(const MCFixup &Fixup,
                                          const MCValue &Target,
                                          bool IsPCRel) const {
  uint8_t Specifier = Target.getSpecifier();

  switch ((unsigned)Fixup.getKind()) {
  case FK_Data_4:
    llvm_unreachable("Unexpected fixup kind!");
  default:
    return ELF::R_TRITON_NONE;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createTritonObjectWriter(uint8_t OSABI, bool IsLittleEndian) {
  return std::make_unique<TritonObjectWriter>(OSABI);
}

bool TritonObjectWriter::needsRelocateWithSymbol(const MCValue &,
                                                 unsigned Type) const {
  return false;
}
