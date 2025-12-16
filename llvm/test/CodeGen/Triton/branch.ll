; RUN: llc -mtriple=triton -verify-machineinstrs < %s | FileCheck %s

; Test branch instruction selection

define i32 @test_branch_eq(i32 %a, i32 %b) {
; CHECK-LABEL: test_branch_eq:
; CHECK: beq
entry:
  %cmp = icmp eq i32 %a, %b
  br i1 %cmp, label %if.then, label %if.else

if.then:
  ret i32 1

if.else:
  ret i32 0
}

define i32 @test_branch_ne(i32 %a, i32 %b) {
; CHECK-LABEL: test_branch_ne:
; CHECK: bne
entry:
  %cmp = icmp ne i32 %a, %b
  br i1 %cmp, label %if.then, label %if.else

if.then:
  ret i32 1

if.else:
  ret i32 0
}

define i32 @test_branch_lt(i32 %a, i32 %b) {
; CHECK-LABEL: test_branch_lt:
; CHECK: blt
entry:
  %cmp = icmp slt i32 %a, %b
  br i1 %cmp, label %if.then, label %if.else

if.then:
  ret i32 1

if.else:
  ret i32 0
}

define i32 @test_branch_ge(i32 %a, i32 %b) {
; CHECK-LABEL: test_branch_ge:
; CHECK: bge
entry:
  %cmp = icmp sge i32 %a, %b
  br i1 %cmp, label %if.then, label %if.else

if.then:
  ret i32 1

if.else:
  ret i32 0
}
