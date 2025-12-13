//===-- TritonISelLowering.h - Triton DAG Lowering Interface --*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Triton uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_TRITON_TRITONISELLOWERING_H
#define LLVM_LIB_TARGET_TRITON_TRITONISELLOWERING_H

#include "Triton.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
class TritonSubtarget;

class TritonISelLowering : public TargetLowering {
  const TritonSubtarget &Subtarget;

public:
  explicit TritonISelLowering(const TargetMachine &TM,
                              const TritonSubtarget &STI);

  // Provide custom lowering hooks for some operations.
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  // This method returns the name of a target specific DAG node.
  const char *getTargetNodeName(unsigned Opcode) const override;

private:
  // Lower incoming arguments, copy physregs into vregs
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;

  SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  // Custom lowering methods
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
};

} // end namespace llvm

#endif // LLVM_LIB_TARGET_TRITON_TRITONISELLOWERING_H
