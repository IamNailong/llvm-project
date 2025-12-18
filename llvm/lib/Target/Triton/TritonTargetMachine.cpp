//===- TritonTargetMachine.cpp - Define TargetMachine for Triton ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about Triton target spec.
//
//===----------------------------------------------------------------------===//

#include "TritonTargetMachine.h"
#include "TargetInfo/TritonTargetInfo.h"
#include "Triton.h"
#include "TritonMachineFunctionInfo.h"
#include "TritonMachineScheduler.h"
#include "TritonSubtarget.h"
#include "llvm/CodeGen/CodeGenTargetMachineImpl.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Transforms/Scalar.h"
#include <optional>

using namespace llvm;

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeTritonTarget() {
  // Register the target.
  RegisterTargetMachine<TritonTargetMachine> A(getTheTritonTarget());
}

static std::string computeDataLayout(const Triple &TT, StringRef CPU,
                                     const TargetOptions &Options,
                                     bool IsLittle) {
  std::string Ret = "e-m:e-p:32:32-i8:8:32-i16:16:32-i64:64-n32";
  return Ret;
}

static Reloc::Model getEffectiveRelocModel(bool JIT,
                                           std::optional<Reloc::Model> RM) {
  if (!RM || JIT)
    return Reloc::Static;
  return *RM;
}

TritonTargetMachine::TritonTargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT,
                                         bool IsLittle)
    : CodeGenTargetMachineImpl(T, computeDataLayout(TT, CPU, Options, IsLittle),
                               TT, CPU, FS, Options,
                               getEffectiveRelocModel(JIT, RM),
                               getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()) {
  initAsmInfo();
}

TritonTargetMachine::TritonTargetMachine(const Target &T, const Triple &TT,
                                         StringRef CPU, StringRef FS,
                                         const TargetOptions &Options,
                                         std::optional<Reloc::Model> RM,
                                         std::optional<CodeModel::Model> CM,
                                         CodeGenOptLevel OL, bool JIT)
    : TritonTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT, true) {}

namespace {
class TritonPassConfig : public TargetPassConfig {
public:
  TritonPassConfig(TritonTargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}

  TritonTargetMachine &getTritonTargetMachine() const {
    return getTM<TritonTargetMachine>();
  }

  bool addInstSelector() override {
    addPass(createTritonISelDag(getTritonTargetMachine(), getOptLevel()));
    return false;
  }
};
} // end anonymous namespace

const TritonSubtarget *
TritonTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute TuneAttr = F.getFnAttribute("tune-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU =
      CPUAttr.isValid() ? CPUAttr.getValueAsString().str() : TargetCPU;
  std::string TuneCPU =
      TuneAttr.isValid() ? TuneAttr.getValueAsString().str() : CPU;
  std::string FS =
      FSAttr.isValid() ? FSAttr.getValueAsString().str() : TargetFS;

  std::string Key = CPU + TuneCPU + FS;
  auto &I = SubtargetMap[Key];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = std::make_unique<TritonSubtarget>(TargetTriple, CPU, TuneCPU, FS,
                                          /*ABIName=*/"", *this);
  }
  return I.get();
}

TargetPassConfig *TritonTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new TritonPassConfig(*this, PM);
}

MachineFunctionInfo *TritonTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return TritonMachineFunctionInfo::create<TritonMachineFunctionInfo>(Allocator,
                                                                      F, STI);
}

ScheduleDAGInstrs *
TritonTargetMachine::createMachineScheduler(MachineSchedContext *C) const {
  // Use the Triton-specific scheduler factory function
  return createTritonMachineScheduler(C);
}
