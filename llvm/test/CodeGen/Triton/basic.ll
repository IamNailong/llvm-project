; RUN: llc -mtriple=triton -verify-machineinstrs < %s | FileCheck %s

; Test basic arithmetic instruction selection

define i32 @test_add(i32 %a, i32 %b) {
; CHECK-LABEL: test_add:
; CHECK: add
  %result = add i32 %a, %b
  ret i32 %result
}

define i32 @test_addi(i32 %a) {
; CHECK-LABEL: test_addi:
; CHECK: addi
  %result = add i32 %a, 10
  ret i32 %result
}

define i32 @test_sub(i32 %a, i32 %b) {
; CHECK-LABEL: test_sub:
; CHECK: sub
  %result = sub i32 %a, %b
  ret i32 %result
}

define i32 @test_and(i32 %a, i32 %b) {
; CHECK-LABEL: test_and:
; CHECK: and
  %result = and i32 %a, %b
  ret i32 %result
}

define i32 @test_or(i32 %a, i32 %b) {
; CHECK-LABEL: test_or:
; CHECK: or
  %result = or i32 %a, %b
  ret i32 %result
}

define i32 @test_xor(i32 %a, i32 %b) {
; CHECK-LABEL: test_xor:
; CHECK: xor
  %result = xor i32 %a, %b
  ret i32 %result
}

define i32 @test_shl(i32 %a, i32 %b) {
; CHECK-LABEL: test_shl:
; CHECK: sll
  %result = shl i32 %a, %b
  ret i32 %result
}

define i32 @test_shr(i32 %a, i32 %b) {
; CHECK-LABEL: test_shr:
; CHECK: srl
  %result = lshr i32 %a, %b
  ret i32 %result
}

define i32 @test_ashr(i32 %a, i32 %b) {
; CHECK-LABEL: test_ashr:
; CHECK: sra
  %result = ashr i32 %a, %b
  ret i32 %result
}
