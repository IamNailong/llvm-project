//===-- TritonSchedule.td - Triton Scheduling Definitions --*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Triton scheduling definitions.
//===----------------------------------------------------------------------===//

// Generic Triton processor model
def TritonModel : SchedMachineModel {
  let MicroOpBufferSize = 0; // Explicitly set to zero since we don't model this.
  let IssueWidth = 1;        // 1 instruction per cycle.
  let LoadLatency = 3;       // Optimistic load latency.
  let MispredictPenalty = 3; // Optimistic branch misprediction penalty.
  let CompleteModel = 0;     // Use the default model otherwise.
}

// Define functional units
def TritonUnitALU    : ProcResource<1>;
def TritonUnitIMul   : ProcResource<1>;
def TritonUnitIDiv   : ProcResource<1>;
def TritonUnitLoad   : ProcResource<1>;
def TritonUnitStore  : ProcResource<1>;
def TritonUnitBranch : ProcResource<1>;

// Integer ALU operations
def : WriteRes<WriteIALU, [TritonUnitALU]> {
  let Latency = 1;
  let SchedModel = TritonModel;
}

// Shift operations
def : WriteRes<WriteShiftImm, [TritonUnitALU]> {
  let Latency = 1;
  let SchedModel = TritonModel;
}

def : WriteRes<WriteShiftReg, [TritonUnitALU]> {
  let Latency = 1;
  let SchedModel = TritonModel;
}

// Load operations
def : WriteRes<WriteLDB, [TritonUnitLoad]> {
  let Latency = 3;
  let SchedModel = TritonModel;
}

def : WriteRes<WriteLDH, [TritonUnitLoad]> {
  let Latency = 3;
  let SchedModel = TritonModel;
}

def : WriteRes<WriteLDW, [TritonUnitLoad]> {
  let Latency = 3;
  let SchedModel = TritonModel;
}

// Store operations
def : WriteRes<WriteSTB, [TritonUnitStore]> {
  let Latency = 1;
  let SchedModel = TritonModel;
}

def : WriteRes<WriteSTH, [TritonUnitStore]> {
  let Latency = 1;
  let SchedModel = TritonModel;
}

def : WriteRes<WriteSTW, [TritonUnitStore]> {
  let Latency = 1;
  let SchedModel = TritonModel;
}

// Branch operations
def : WriteRes<WriteBranch, [TritonUnitBranch]> {
  let Latency = 1;
  let SchedModel = TritonModel;
}

def : WriteRes<WriteJal, [TritonUnitBranch]> {
  let Latency = 1;
  let SchedModel = TritonModel;
}

def : WriteRes<WriteJalr, [TritonUnitBranch]> {
  let Latency = 1;
  let SchedModel = TritonModel;
}

// System operations
def : WriteRes<WriteSystem, [TritonUnitALU]> {
  let Latency = 1;
  let SchedModel = TritonModel;
}
