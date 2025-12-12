//===-- TritonRegisterInfo.td - Triton Register defs -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the Triton register file
//===----------------------------------------------------------------------===//

let Namespace = "Triton" in {
  def sub_32 : SubRegIndex<32>;

  class TritonReg32<bits<5> Enc, string n, list<string> alt = []> : Register<n> {
    let HWEncoding{4-0} = Enc;
    let AltNames = alt;
  }

  let FallbackRegAltNameIndex = NoRegAltName in
  def ABIRegAltName : RegAltNameIndex;
}

// Integer registers
let RegAltNameIndices = [ABIRegAltName] in {
  def X0    : TritonReg32<0, "x0", ["zero"]>, DwarfRegNum<[0]>;
  def X1    : TritonReg32<1, "x1", ["ra"]>, DwarfRegNum<[1]>;
  def X2    : TritonReg32<2, "x2", ["sp"]>, DwarfRegNum<[2]>;
  def X3    : TritonReg32<3, "x3", ["gp"]>, DwarfRegNum<[3]>;
  def X4    : TritonReg32<4, "x4", ["tp"]>, DwarfRegNum<[4]>;
  def X5    : TritonReg32<5, "x5", ["t0"]>, DwarfRegNum<[5]>;
  def X6    : TritonReg32<6, "x6", ["t1"]>, DwarfRegNum<[6]>;
  def X7    : TritonReg32<7, "x7", ["t2"]>, DwarfRegNum<[7]>;
  def X8    : TritonReg32<8, "x8", ["s0"]>, DwarfRegNum<[8]>;
  def X9    : TritonReg32<9, "x9", ["s1"]>, DwarfRegNum<[9]>;
  def X10   : TritonReg32<10,"x10", ["a0"]>, DwarfRegNum<[10]>;
  def X11   : TritonReg32<11,"x11", ["a1"]>, DwarfRegNum<[11]>;
  def X12   : TritonReg32<12,"x12", ["a2"]>, DwarfRegNum<[12]>;
  def X13   : TritonReg32<13,"x13", ["a3"]>, DwarfRegNum<[13]>;
  def X14   : TritonReg32<14,"x14", ["a4"]>, DwarfRegNum<[14]>;
  def X15   : TritonReg32<15,"x15", ["a5"]>, DwarfRegNum<[15]>;
  def X16   : TritonReg32<16,"x16", ["a6"]>, DwarfRegNum<[16]>;
  def X17   : TritonReg32<17,"x17", ["a7"]>, DwarfRegNum<[17]>;
  def X18   : TritonReg32<18,"x18", ["s2"]>, DwarfRegNum<[18]>;
  def X19   : TritonReg32<19,"x19", ["s3"]>, DwarfRegNum<[19]>;
  def X20   : TritonReg32<20,"x20", ["s4"]>, DwarfRegNum<[20]>;
  def X21   : TritonReg32<21,"x21", ["s5"]>, DwarfRegNum<[21]>;
  def X22   : TritonReg32<22,"x22", ["s6"]>, DwarfRegNum<[22]>;
  def X23   : TritonReg32<23,"x23", ["s7"]>, DwarfRegNum<[23]>;
  def X24   : TritonReg32<24,"x24", ["s8"]>, DwarfRegNum<[24]>;
  def X25   : TritonReg32<25,"x25", ["s9"]>, DwarfRegNum<[25]>;
  def X26   : TritonReg32<26,"x26", ["s10"]>, DwarfRegNum<[26]>;
  def X27   : TritonReg32<27,"x27", ["s11"]>, DwarfRegNum<[27]>;
  def X28   : TritonReg32<28,"x28", ["t3"]>, DwarfRegNum<[28]>;
  def X29   : TritonReg32<29,"x29", ["t4"]>, DwarfRegNum<[29]>;
  def X30   : TritonReg32<30,"x30", ["t5"]>, DwarfRegNum<[30]>;
  def X31   : TritonReg32<31,"x31", ["t6"]>, DwarfRegNum<[31]>;
}


def XLenVT : ValueTypeByHwMode<[DefaultMode], [i32]>;

// TODO: once codegen is implemented, registers should be listed in an order
// reflecting the preferred register allocation sequence.
def GPR : RegisterClass<"Triton", [i32], 32, (add
  (sequence "X%u", 0, 31)
)>;

def GPRNoZero : RegisterClass<"Triton", [i32], 32, (add
  (sequence "X%u", 1, 31)
)>;
