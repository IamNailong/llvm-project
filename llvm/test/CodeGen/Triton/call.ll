; RUN: llc -mtriple=triton -verify-machineinstrs < %s | FileCheck %s

; Test function call instruction selection

declare i32 @external_func(i32)

define i32 @test_call(i32 %a) {
; CHECK-LABEL: test_call:
; CHECK: call external_func
  %result = call i32 @external_func(i32 %a)
  ret i32 %result
}

define i32 @test_simple_return(i32 %a) {
; CHECK-LABEL: test_simple_return:
; CHECK: ret
  ret i32 %a
}

define void @test_void_return() {
; CHECK-LABEL: test_void_return:
; CHECK: ret
  ret void
}

define i32 @test_call_with_args(i32 %a, i32 %b) {
; CHECK-LABEL: test_call_with_args:
  %sum = add i32 %a, %b
  %result = call i32 @external_func(i32 %sum)
  ret i32 %result
}
