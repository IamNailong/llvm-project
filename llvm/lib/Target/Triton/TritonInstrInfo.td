//===-- TritonInstrInfo.td - Target Description for Triton ---*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes the Triton instructions in TableGen format.
//
//===----------------------------------------------------------------------===//

include "TritonInstrFormats.td"

//===----------------------------------------------------------------------===//
// SDType definitions
//===----------------------------------------------------------------------===//

def SDT_TritonCall         : SDTypeProfile<0, -1, [SDTCisVT<0, XLenVT>]>;
def SDT_TritonCallSeqStart : SDTypeProfile<0, 2, [SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def SDT_TritonCallSeqEnd   : SDTypeProfile<0, 2, [SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;

//===----------------------------------------------------------------------===//
// SDNode definitions
//===----------------------------------------------------------------------===//

def triton_ret_glue : SDNode<"TritonISD::RET_GLUE", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def triton_call     : SDNode<"TritonISD::CALL", SDT_TritonCall,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;
def triton_callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_TritonCallSeqStart,
                                  [SDNPHasChain, SDNPOutGlue]>;
def triton_callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_TritonCallSeqEnd,
                                  [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;


//===----------------------------------------------------------------------===//
// Scheduling Write Types (defined here to be available for instruction definitions)
//===----------------------------------------------------------------------===//

def WriteIALU       : SchedWrite;
def WriteShiftImm   : SchedWrite;
def WriteShiftReg   : SchedWrite;
def WriteLDB        : SchedWrite;
def WriteLDH        : SchedWrite;
def WriteLDW        : SchedWrite;
def WriteSTB        : SchedWrite;
def WriteSTH        : SchedWrite;
def WriteSTW        : SchedWrite;
def WriteBranch     : SchedWrite;
def WriteJal        : SchedWrite;
def WriteJalr       : SchedWrite;
def WriteSystem     : SchedWrite;

// Get the Hi 20 bits from an address. Selected to LUI.
def triton_hi : SDNode<"TritonISD::HI", SDTIntUnaryOp>;

// Add the Lo 12 bits from an address. Selected to ADDI.
def triton_add_lo : SDNode<"TritonISD::ADD_LO", SDTIntBinOp>;

//===----------------------------------------------------------------------===//
// Operand and SDNode transformation definitions.
//===----------------------------------------------------------------------===//

class SImmAsmOperand<int width, string suffix = "">
    : AsmOperandClass {
  let Name = "SImm" # width # suffix;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidSImm" # width # suffix;
}

class UImmAsmOperand<int width, string suffix = "">
    : AsmOperandClass {
  let Name = "UImm" # width # suffix;
  let RenderMethod = "addImmOperands";
  let DiagnosticType = "InvalidUImm" # width # suffix;
}

class TritonUImmOp<int bitsNum> : Operand<XLenVT> {
  let ParserMatchClass = UImmAsmOperand<bitsNum>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeUImmOperand<" # bitsNum # ">";
}

class TritonSImmOp<int bitsNum> : Operand<XLenVT> {
  let ParserMatchClass = SImmAsmOperand<bitsNum>;
  let EncoderMethod = "getImmOpValue";
  let DecoderMethod = "decodeSImmOperand<" # bitsNum # ">";
}

def uimm5 : TritonUImmOp<5>, ImmLeaf<XLenVT, "return isUInt<5>(Imm);">;
def uimm20 : TritonUImmOp<20>;
def simm12 : TritonSImmOp<12>, ImmLeaf<XLenVT, "return isInt<12>(Imm);">;

// A 13-bit signed immediate where the least significant bit is zero.
def simm13_lsb0 : Operand<OtherVT>, ImmLeaf<XLenVT, [{return isShiftedInt<12, 1>(Imm);}]> {
  let ParserMatchClass = SImmAsmOperand<13, "Lsb0">;
  let PrintMethod = "printBranchOperand";
  let EncoderMethod = "getImmOpValueAsr1";
  let DecoderMethod = "decodeSImmOperandAndLsl1<13>";
  let OperandType = "OPERAND_PCREL";
}

// A 21-bit signed immediate where the least significant bit is zero.
def simm21_lsb0 : Operand<OtherVT>, ImmLeaf<XLenVT, [{return isShiftedInt<20, 1>(Imm);}]> {
  let ParserMatchClass = SImmAsmOperand<21, "Lsb0">;
  let PrintMethod = "printBranchOperand";
  let EncoderMethod = "getImmOpValueAsr1";
  let DecoderMethod = "decodeSImmOperandAndLsl1<21>";
  let OperandType = "OPERAND_PCREL";
}

// Memory operands
def GPRMem : RegisterOperand<GPR> {
  let OperandType = "OPERAND_MEMORY";
}

//===----------------------------------------------------------------------===//
// Instruction Class Templates
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class ALU_ri<bits<3> funct3, string opcodestr>
    : TritonInstI<funct3, OPC_OP_IMM, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12),
              opcodestr, "$rd, $rs1, $imm12">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class ALU_rr<bits<7> funct7, bits<3> funct3, string opcodestr>
    : TritonInstR<funct7, funct3, OPC_OP, (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
              opcodestr, "$rd, $rs1, $rs2">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Shift_ri<bit arithshift, bits<3> funct3, string opcodestr>
    : TritonInstIShift<arithshift, funct3, OPC_OP_IMM, (outs GPR:$rd), (ins GPR:$rs1, uimm5:$shamt),
              opcodestr, "$rd, $rs1, $shamt">;

let mayLoad = 1, hasSideEffects = 0, mayStore = 0 in
class Load_ri<bits<3> funct3, string opcodestr>
    : TritonInstI<funct3, OPC_LOAD, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12),
              opcodestr, "$rd, ${imm12}(${rs1})">;

let mayLoad = 0, hasSideEffects = 0, mayStore = 1 in
class Store_rri<bits<3> funct3, string opcodestr>
    : TritonInstS<funct3, OPC_STORE, (outs), (ins GPR:$rs2, GPR:$rs1, simm12:$imm12),
              opcodestr, "$rs2, ${imm12}(${rs1})">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Branch_rri<bits<3> funct3, string opcodestr>
    : TritonInstB<funct3, OPC_BRANCH, (outs), (ins GPR:$rs1, GPR:$rs2, simm13_lsb0:$imm12),
              opcodestr, "$rs1, $rs2, $imm12">;

let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Upper_ri<TritonOpcode opcode, string opcodestr>
    : TritonInstU<opcode, (outs GPR:$rd), (ins uimm20:$imm20),
              opcodestr, "$rd, $imm20">;

let isCall = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class Jump_ri<TritonOpcode opcode, string opcodestr>
    : TritonInstJ<opcode, (outs GPR:$rd), (ins simm21_lsb0:$imm20),
              opcodestr, "$rd, $imm20">;

let isCall = 1, hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
class JumpReg_ri<bits<3> funct3, string opcodestr>
    : TritonInstI<funct3, OPC_JALR, (outs GPR:$rd), (ins GPR:$rs1, simm12:$imm12),
              opcodestr, "$rd, ${imm12}(${rs1})">;

//===----------------------------------------------------------------------===//
// Instructions
//===----------------------------------------------------------------------===//

// Integer Register-Immediate Instructions
def ADDI  : ALU_ri<0b000, "addi">, Sched<[WriteIALU]>;
def SLTI  : ALU_ri<0b010, "slti">, Sched<[WriteIALU]>;
def SLTIU : ALU_ri<0b011, "sltiu">, Sched<[WriteIALU]>;
def XORI  : ALU_ri<0b100, "xori">, Sched<[WriteIALU]>;
def ORI   : ALU_ri<0b110, "ori">, Sched<[WriteIALU]>;
def ANDI  : ALU_ri<0b111, "andi">, Sched<[WriteIALU]>;

// Shift Instructions (Immediate)
def SLLI : Shift_ri<0, 0b001, "slli">, Sched<[WriteShiftImm]>;
def SRLI : Shift_ri<0, 0b101, "srli">, Sched<[WriteShiftImm]>;
def SRAI : Shift_ri<1, 0b101, "srai">, Sched<[WriteShiftImm]>;

// Integer Register-Register Instructions
def ADD  : ALU_rr<0b0000000, 0b000, "add">, Sched<[WriteIALU]>;
def SUB  : ALU_rr<0b0100000, 0b000, "sub">, Sched<[WriteIALU]>;
def SLL  : ALU_rr<0b0000000, 0b001, "sll">, Sched<[WriteShiftReg]>;
def SLT  : ALU_rr<0b0000000, 0b010, "slt">, Sched<[WriteIALU]>;
def SLTU : ALU_rr<0b0000000, 0b011, "sltu">, Sched<[WriteIALU]>;
def XOR  : ALU_rr<0b0000000, 0b100, "xor">, Sched<[WriteIALU]>;
def SRL  : ALU_rr<0b0000000, 0b101, "srl">, Sched<[WriteShiftReg]>;
def SRA  : ALU_rr<0b0100000, 0b101, "sra">, Sched<[WriteShiftReg]>;
def OR   : ALU_rr<0b0000000, 0b110, "or">, Sched<[WriteIALU]>;
def AND  : ALU_rr<0b0000000, 0b111, "and">, Sched<[WriteIALU]>;

// Upper Immediate Instructions
def LUI   : Upper_ri<OPC_LUI, "lui">, Sched<[WriteIALU]>;
def AUIPC : Upper_ri<OPC_AUIPC, "auipc">, Sched<[WriteIALU]>;

// Jump Instructions
def JAL  : Jump_ri<OPC_JAL, "jal">, Sched<[WriteJal]>;
def JALR : JumpReg_ri<0b000, "jalr">, Sched<[WriteJalr]>;

// Branch Instructions
def BEQ  : Branch_rri<0b000, "beq">, Sched<[WriteBranch]>;
def BNE  : Branch_rri<0b001, "bne">, Sched<[WriteBranch]>;
def BLT  : Branch_rri<0b100, "blt">, Sched<[WriteBranch]>;
def BGE  : Branch_rri<0b101, "bge">, Sched<[WriteBranch]>;
def BLTU : Branch_rri<0b110, "bltu">, Sched<[WriteBranch]>;
def BGEU : Branch_rri<0b111, "bgeu">, Sched<[WriteBranch]>;

// Load Instructions
def LB  : Load_ri<0b000, "lb">, Sched<[WriteLDB]>;
def LH  : Load_ri<0b001, "lh">, Sched<[WriteLDH]>;
def LW  : Load_ri<0b010, "lw">, Sched<[WriteLDW]>;
def LBU : Load_ri<0b100, "lbu">, Sched<[WriteLDB]>;
def LHU : Load_ri<0b101, "lhu">, Sched<[WriteLDH]>;

// Store Instructions
def SB : Store_rri<0b000, "sb">, Sched<[WriteSTB]>;
def SH : Store_rri<0b001, "sh">, Sched<[WriteSTH]>;
def SW : Store_rri<0b010, "sw">, Sched<[WriteSTW]>;

// System Instructions
let hasSideEffects = 1, mayLoad = 0, mayStore = 0 in {
def ECALL  : TritonInstI<0b000, OPC_SYSTEM, (outs), (ins), "ecall", "">,
             Sched<[WriteSystem]> {
  let imm12 = 0b000000000000;
  let rs1 = 0b00000;
  let rd = 0b00000;
}

def EBREAK : TritonInstI<0b000, OPC_SYSTEM, (outs), (ins), "ebreak", "">,
             Sched<[WriteSystem]> {
  let imm12 = 0b000000000001;
  let rs1 = 0b00000;
  let rd = 0b00000;
}
}

//===----------------------------------------------------------------------===//
// Operand definitions for pseudo instructions
//===----------------------------------------------------------------------===//

def bare_symbol : Operand<XLenVT>;
def call_symbol : Operand<XLenVT>;
def cond_code : Operand<OtherVT> {
  let OperandType = "OPERAND_IMMEDIATE";
}

//===----------------------------------------------------------------------===//
// Pseudo Instructions
//===----------------------------------------------------------------------===//

let hasSideEffects = 0, mayLoad = 0, mayStore = 0, isCodeGenOnly = 1 in {
// Pseudo instruction for loading 32-bit constants
def PseudoLI : Pseudo<(outs GPR:$rd), (ins simm12:$imm),
                      [(set GPR:$rd, simm12:$imm)]>;

// Pseudo instruction for loading addresses
def PseudoLA : Pseudo<(outs GPR:$rd), (ins bare_symbol:$addr), []>;

// Pseudo instruction for function calls
let isCall = 1 in
def PseudoCALL : Pseudo<(outs), (ins call_symbol:$func),
                        [(triton_call tglobaladdr:$func)]>;

// Pseudo instruction for returns
let isReturn = 1, isTerminator = 1, isBarrier = 1 in
def PseudoRET : Pseudo<(outs), (ins), [(triton_ret_glue)]>;

// Pseudo instruction for unconditional jumps
let isBranch = 1, isTerminator = 1, isBarrier = 1 in
def PseudoBR : Pseudo<(outs), (ins simm21_lsb0:$offset), []>;

// Call stack adjustment pseudo instructions
let Defs = [X2], Uses = [X2], hasSideEffects = 0, mayLoad = 0, mayStore = 0 in {
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              [(triton_callseq_start timm:$amt1, timm:$amt2)]>;
def ADJCALLSTACKUP   : Pseudo<(outs), (ins i32imm:$amt1, i32imm:$amt2),
                              [(triton_callseq_end timm:$amt1, timm:$amt2)]>;
}
def PseudoJUMP : Pseudo<(outs), (ins simm21_lsb0:$offset), []>;

// Pseudo instruction for conditional branches
let isBranch = 1, isTerminator = 1 in
def PseudoBR_CC : Pseudo<(outs), (ins GPR:$lhs, GPR:$rhs, cond_code:$cc, simm13_lsb0:$dst), []>;
}

//===----------------------------------------------------------------------===//
// Pattern Matching
//===----------------------------------------------------------------------===//

// Basic arithmetic patterns
def : Pat<(add GPR:$rs1, simm12:$imm12), (ADDI GPR:$rs1, simm12:$imm12)>;
def : Pat<(add GPR:$rs1, GPR:$rs2), (ADD GPR:$rs1, GPR:$rs2)>;
def : Pat<(sub GPR:$rs1, GPR:$rs2), (SUB GPR:$rs1, GPR:$rs2)>;
def : Pat<(xor GPR:$rs1, simm12:$imm12), (XORI GPR:$rs1, simm12:$imm12)>;
def : Pat<(xor GPR:$rs1, GPR:$rs2), (XOR GPR:$rs1, GPR:$rs2)>;
def : Pat<(or GPR:$rs1, simm12:$imm12), (ORI GPR:$rs1, simm12:$imm12)>;
def : Pat<(or GPR:$rs1, GPR:$rs2), (OR GPR:$rs1, GPR:$rs2)>;
def : Pat<(and GPR:$rs1, simm12:$imm12), (ANDI GPR:$rs1, simm12:$imm12)>;
def : Pat<(and GPR:$rs1, GPR:$rs2), (AND GPR:$rs1, GPR:$rs2)>;

// Shift patterns
def : Pat<(shl GPR:$rs1, uimm5:$shamt), (SLLI GPR:$rs1, uimm5:$shamt)>;
def : Pat<(shl GPR:$rs1, GPR:$rs2), (SLL GPR:$rs1, GPR:$rs2)>;
def : Pat<(srl GPR:$rs1, uimm5:$shamt), (SRLI GPR:$rs1, uimm5:$shamt)>;
def : Pat<(srl GPR:$rs1, GPR:$rs2), (SRL GPR:$rs1, GPR:$rs2)>;
def : Pat<(sra GPR:$rs1, uimm5:$shamt), (SRAI GPR:$rs1, uimm5:$shamt)>;
def : Pat<(sra GPR:$rs1, GPR:$rs2), (SRA GPR:$rs1, GPR:$rs2)>;

// Comparison patterns
def : Pat<(setcc GPR:$lhs, GPR:$rhs, SETLT), (SLT GPR:$lhs, GPR:$rhs)>;
def : Pat<(setcc GPR:$lhs, GPR:$rhs, SETULT), (SLTU GPR:$lhs, GPR:$rhs)>;
def : Pat<(setcc GPR:$lhs, simm12:$rhs, SETLT), (SLTI GPR:$lhs, simm12:$rhs)>;
def : Pat<(setcc GPR:$lhs, simm12:$rhs, SETULT), (SLTIU GPR:$lhs, simm12:$rhs)>;

// Load patterns
def : Pat<(i32 (sextloadi8 (add GPR:$rs1, simm12:$imm12))),
          (LB GPR:$rs1, simm12:$imm12)>;
def : Pat<(i32 (sextloadi16 (add GPR:$rs1, simm12:$imm12))),
          (LH GPR:$rs1, simm12:$imm12)>;
def : Pat<(i32 (load (add GPR:$rs1, simm12:$imm12))),
          (LW GPR:$rs1, simm12:$imm12)>;
def : Pat<(i32 (zextloadi8 (add GPR:$rs1, simm12:$imm12))),
          (LBU GPR:$rs1, simm12:$imm12)>;
def : Pat<(i32 (zextloadi16 (add GPR:$rs1, simm12:$imm12))),
          (LHU GPR:$rs1, simm12:$imm12)>;

// Store patterns
def : Pat<(truncstorei8 GPR:$rs2, (add GPR:$rs1, simm12:$imm12)),
          (SB GPR:$rs2, GPR:$rs1, simm12:$imm12)>;
def : Pat<(truncstorei16 GPR:$rs2, (add GPR:$rs1, simm12:$imm12)),
          (SH GPR:$rs2, GPR:$rs1, simm12:$imm12)>;
def : Pat<(store GPR:$rs2, (add GPR:$rs1, simm12:$imm12)),
          (SW GPR:$rs2, GPR:$rs1, simm12:$imm12)>;

// Immediate loading patterns
def : Pat<(triton_hi tglobaladdr:$in), (LUI tglobaladdr:$in)>;
def : Pat<(triton_add_lo GPR:$hi, tglobaladdr:$lo), (ADDI GPR:$hi, tglobaladdr:$lo)>;

// Branch patterns
def : Pat<(brcond (i32 (setcc GPR:$lhs, GPR:$rhs, SETEQ)), bb:$dst),
          (BEQ GPR:$lhs, GPR:$rhs, bb:$dst)>;
def : Pat<(brcond (i32 (setcc GPR:$lhs, GPR:$rhs, SETNE)), bb:$dst),
          (BNE GPR:$lhs, GPR:$rhs, bb:$dst)>;
def : Pat<(brcond (i32 (setcc GPR:$lhs, GPR:$rhs, SETLT)), bb:$dst),
          (BLT GPR:$lhs, GPR:$rhs, bb:$dst)>;
def : Pat<(brcond (i32 (setcc GPR:$lhs, GPR:$rhs, SETGE)), bb:$dst),
          (BGE GPR:$lhs, GPR:$rhs, bb:$dst)>;
def : Pat<(brcond (i32 (setcc GPR:$lhs, GPR:$rhs, SETULT)), bb:$dst),
          (BLTU GPR:$lhs, GPR:$rhs, bb:$dst)>;
def : Pat<(brcond (i32 (setcc GPR:$lhs, GPR:$rhs, SETUGE)), bb:$dst),
          (BGEU GPR:$lhs, GPR:$rhs, bb:$dst)>;


