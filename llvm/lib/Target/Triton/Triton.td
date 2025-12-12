//===- Triton.td - Describe the Triton Target Machine ------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Triton supported processors.
//===----------------------------------------------------------------------===//
class Proc<string Name, list<SubtargetFeature> Features>
    : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

include "TritonRegisterInfo.td"
include "TritonInstrInfo.td"

def TritonInstrInfo : InstrInfo;

def TritonAsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = 1;
}

def TritonInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
}


//===----------------------------------------------------------------------===//
// Target Declaration
//===----------------------------------------------------------------------===//

def Triton : Target {
  let InstructionSet = TritonInstrInfo;
  let AssemblyParsers = [TritonAsmParser];
  let AssemblyWriters = [TritonInstPrinter];
}
