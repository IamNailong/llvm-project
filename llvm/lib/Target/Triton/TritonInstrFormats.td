//===-- TritonInstrFormats.td - Triton Instruction Formats ---*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
//
//  Declarations that describe the Triton instruction formats
//===----------------------------------------------------------------------===//

// Format specifies the encoding used by the instruction.
class InstFormat<bits<5> val> {
  bits<5> Value = val;
}
def InstFormatPseudo : InstFormat<0>;
def InstFormatR      : InstFormat<1>;
def InstFormatI      : InstFormat<2>;
def InstFormatS      : InstFormat<3>;
def InstFormatB      : InstFormat<4>;
def InstFormatU      : InstFormat<5>;
def InstFormatJ      : InstFormat<6>;
def InstFormatOther  : InstFormat<31>;

class TritonOpcode<bits<7> val> {
  bits<7> Value = val;
}

def OPC_LOAD      : TritonOpcode<0b0000011>;
def OPC_LOAD_FP   : TritonOpcode<0b0000111>;
def OPC_MISC_MEM  : TritonOpcode<0b0001111>;
def OPC_OP_IMM    : TritonOpcode<0b0010011>;
def OPC_AUIPC     : TritonOpcode<0b0010111>;
def OPC_OP_IMM_32 : TritonOpcode<0b0011011>;
def OPC_STORE     : TritonOpcode<0b0100011>;
def OPC_STORE_FP  : TritonOpcode<0b0100111>;
def OPC_AMO       : TritonOpcode<0b0101111>;
def OPC_OP        : TritonOpcode<0b0110011>;
def OPC_LUI       : TritonOpcode<0b0110111>;
def OPC_OP_32     : TritonOpcode<0b0111011>;
def OPC_MADD      : TritonOpcode<0b1000011>;
def OPC_MSUB      : TritonOpcode<0b1000111>;
def OPC_NMSUB     : TritonOpcode<0b1001011>;
def OPC_NMADD     : TritonOpcode<0b1001111>;
def OPC_OP_FP     : TritonOpcode<0b1010011>;
def OPC_BRANCH    : TritonOpcode<0b1100011>;
def OPC_JALR      : TritonOpcode<0b1100111>;
def OPC_JAL       : TritonOpcode<0b1101111>;
def OPC_SYSTEM    : TritonOpcode<0b1110011>;

class TritonInst<dag outs, dag ins, string opcodestr, string argstr,
             list<dag> pattern, InstFormat format>
    : Instruction {
  field bits<32> Inst;
  let Size = 4;

  bits<7> Opcode = 0;

  let Inst{6-0} = Opcode;

  let Namespace = "Triton";

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = opcodestr # !if(!empty(argstr), "", "\t" # argstr);
  let Pattern = pattern;

  let TSFlags{4-0} = format.Value;
}

// Pseudo instructions
class Pseudo<dag outs, dag ins, list<dag> pattern>
    : TritonInst<outs, ins, "", "", pattern, InstFormatPseudo> {
  let isPseudo = 1;
  let isCodeGenOnly = 1;
}

// Instruction formats are listed in the order they appear in the Triton
// instruction set manual (R, I, S, B, U, J) with sub-formats (e.g. RVInstR4,
// RVInstRAtomic) sorted alphabetically.

class TritonInstR<bits<7> funct7, bits<3> funct3, TritonOpcode opcode, dag outs,
              dag ins, string opcodestr, string argstr>
    : TritonInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> rs2;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-25} = funct7;
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class TritonInstI<bits<3> funct3, TritonOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : TritonInst<outs, ins, opcodestr, argstr, [], InstFormatI> {
  bits<12> imm12;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31-20} = imm12;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class TritonInstIShift<bit arithshift, bits<3> funct3, TritonOpcode opcode,
                   dag outs, dag ins, string opcodestr, string argstr>
    : TritonInst<outs, ins, opcodestr, argstr, [], InstFormatI> {
  bits<5> shamt;
  bits<5> rs1;
  bits<5> rd;

  let Inst{31} = 0;
  let Inst{30} = arithshift;
  let Inst{29-25} = 0;
  let Inst{24-20} = shamt;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class TritonInstS<bits<3> funct3, TritonOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : TritonInst<outs, ins, opcodestr, argstr, [], InstFormatS> {
  bits<12> imm12;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31-25} = imm12{11-5};
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-7} = imm12{4-0};
  let Opcode = opcode.Value;
}

class TritonInstB<bits<3> funct3, TritonOpcode opcode, dag outs, dag ins,
              string opcodestr, string argstr>
    : TritonInst<outs, ins, opcodestr, argstr, [], InstFormatB> {
  bits<12> imm12;
  bits<5> rs2;
  bits<5> rs1;

  let Inst{31} = imm12{11};
  let Inst{30-25} = imm12{9-4};
  let Inst{24-20} = rs2;
  let Inst{19-15} = rs1;
  let Inst{14-12} = funct3;
  let Inst{11-8} = imm12{3-0};
  let Inst{7} = imm12{10};
  let Opcode = opcode.Value;
}

class TritonInstU<TritonOpcode opcode, dag outs, dag ins, string opcodestr,
              string argstr>
    : TritonInst<outs, ins, opcodestr, argstr, [], InstFormatU> {
  bits<20> imm20;
  bits<5> rd;

  let Inst{31-12} = imm20;
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}

class TritonInstJ<TritonOpcode opcode, dag outs, dag ins, string opcodestr,
              string argstr>
    : TritonInst<outs, ins, opcodestr, argstr, [], InstFormatJ> {
  bits<20> imm20;
  bits<5> rd;

  let Inst{31} = imm20{19};
  let Inst{30-21} = imm20{9-0};
  let Inst{20} = imm20{10};
  let Inst{19-12} = imm20{18-11};
  let Inst{11-7} = rd;
  let Opcode = opcode.Value;
}
