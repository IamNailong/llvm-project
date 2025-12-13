//===-- TritonCallingConv.td - Calling Conventions for Triton --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for Triton architecture.
//===----------------------------------------------------------------------===//

/// CCIfSubtarget - Match if the current subtarget has a feature F.
class CCIfSubtarget<string F, CCAction A>:
  CCIf<!strconcat("State.getTarget().getSubtarget<TritonSubtarget>().", F), A>;

// The RISC-V calling convention is handled with custom code in
// TritonISelLowering.cpp (CC_Triton).

def CSR_ILP32_LP64 : CalleeSavedRegs<(add X1, X3, X4, X8, X9, (sequence "X%u", 18, 27))>;

// Interrupt handler needs to save/restore all registers that are used,
// both Caller and Callee saved registers.
def CSR_Interrupt : CalleeSavedRegs<(add X1, (sequence "X%u", 3, 31))>;

// For now, we don't have floating point registers, so these are the same as CSR_Interrupt
def CSR_XLEN_F32_Interrupt : CalleeSavedRegs<(add CSR_Interrupt)>;
def CSR_XLEN_F64_Interrupt : CalleeSavedRegs<(add CSR_Interrupt)>;

// Calling convention for Triton
def CC_Triton : CallingConv<[
  // Promote i8/i16 arguments to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // The first 8 integer arguments are passed in integer registers a0-a7.
  CCIfType<[i32], CCAssignToReg<[X10, X11, X12, X13, X14, X15, X16, X17]>>,

  // Integer values get stored in stack slots that are 4 bytes in size and
  // 4-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def RetCC_Triton : CallingConv<[
  // Integer return values up to the size of i32 are returned in registers
  // a0 and a1.
  CCIfType<[i32], CCAssignToReg<[X10, X11]>>
]>;

// Callee-saved register lists.
def CSR_ILP32 : CalleeSavedRegs<(add X1, X3, X4, X8, X9, (sequence "X%u", 18, 27))>;
def CSR : CalleeSavedRegs<(add CSR_ILP32)>;
