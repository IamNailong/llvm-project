//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/TritonMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace llvm {
class MCObjectTargetWriter;
}
namespace {
class TritonAsmBackend : public MCAsmBackend {
  uint8_t OSABI;
  bool IsLittleEndian;

public:
  TritonAsmBackend(uint8_t osABI, bool isLE)
      : MCAsmBackend(llvm::endianness::little), OSABI(osABI),
        IsLittleEndian(isLE) {}

  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override;
  std::optional<bool> evaluateFixup(const MCFragment &, MCFixup &, MCValue &,
                                    uint64_t &) override;
  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  MutableArrayRef<char> Data, uint64_t Value,
                  bool IsResolved) override;
  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createTritonObjectWriter(OSABI, IsLittleEndian);
  }
};
} // namespace

MCFixupKindInfo TritonAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  // TODO: Implement this
  llvm_unreachable("Unknown fixup kind!");
}

static uint64_t adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                 MCContext &Ctx) {
  unsigned Kind = Fixup.getKind();
  // TODO: Implement this
  llvm_unreachable("Unknown fixup kind!");
}

static unsigned getSize(unsigned Kind) {
  // TODO: Implement this
  llvm_unreachable("Unknown fixup size!");
}

std::optional<bool> TritonAsmBackend::evaluateFixup(const MCFragment &F,
                                                    MCFixup &Fixup, MCValue &,
                                                    uint64_t &Value) {
  // TODO: Implement this
  llvm_unreachable("Unknown fixup kind!");
  return {};
}

void TritonAsmBackend::applyFixup(const MCFragment &F, const MCFixup &Fixup,
                                  const MCValue &Target,
                                  MutableArrayRef<char> Data, uint64_t Value,
                                  bool IsResolved) {
  maybeAddReloc(F, Fixup, Target, Value, IsResolved);
  MCContext &Ctx = getContext();
  MCFixupKindInfo Info = getFixupKindInfo(Fixup.getKind());

  Value = adjustFixupValue(Fixup, Value, Ctx);

  // Shift the value into position.
  Value <<= Info.TargetOffset;

  if (!Value)
    return; // Doesn't change encoding.

  unsigned Offset = Fixup.getOffset();
  unsigned FullSize = getSize(Fixup.getKind());

  for (unsigned i = 0; i != FullSize; ++i) {
    Data[Offset + i] |= uint8_t((Value >> (i * 8)) & 0xff);
  }
}

bool TritonAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                    const MCSubtargetInfo *STI) const {
  uint64_t NumNops24b = Count / 3;

  for (uint64_t i = 0; i != NumNops24b; ++i) {
    // Currently just little-endian machine supported,
    // but probably big-endian will be also implemented in future
    if (IsLittleEndian) {
      OS.write("\xf0", 1);
      OS.write("\x20", 1);
      OS.write("\0x00", 1);
    } else {
      report_fatal_error("Big-endian mode currently is not supported!");
    }
    Count -= 3;
  }

  // TODO maybe function should return error if (Count > 0)
  switch (Count) {
  default:
    break;
  case 1:
    OS.write("\0", 1);
    break;
  case 2:
    // NOP.N instruction
    OS.write("\x3d", 1);
    OS.write("\xf0", 1);
    break;
  }

  return true;
}

MCAsmBackend *llvm::createTritonMCAsmBackend(const Target &T,
                                             const MCSubtargetInfo &STI,
                                             const MCRegisterInfo &MRI,
                                             const MCTargetOptions &Options) {
  uint8_t OSABI =
      MCELFObjectTargetWriter::getOSABI(STI.getTargetTriple().getOS());
  return new TritonAsmBackend(OSABI, true);
}
