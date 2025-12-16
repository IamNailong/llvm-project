; RUN: llc -mtriple=triton -verify-machineinstrs < %s | FileCheck %s

; Test global address instruction selection

@global_var = global i32 0

define i32 @test_load_global() {
; CHECK-LABEL: test_load_global:
; CHECK: lui
; CHECK: addi
; CHECK: lw
  %val = load i32, ptr @global_var
  ret i32 %val
}

define void @test_store_global(i32 %val) {
; CHECK-LABEL: test_store_global:
; CHECK: lui
; CHECK: addi
; CHECK: sw
  store i32 %val, ptr @global_var
  ret void
}

@global_array = global [10 x i32] zeroinitializer

define i32 @test_load_global_array(i32 %idx) {
; CHECK-LABEL: test_load_global_array:
; CHECK: lui
  %ptr = getelementptr [10 x i32], ptr @global_array, i32 0, i32 %idx
  %val = load i32, ptr %ptr
  ret i32 %val
}
