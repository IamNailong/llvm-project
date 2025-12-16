; RUN: llc -mtriple=triton -verify-machineinstrs < %s | FileCheck %s

; Test memory operation instruction selection

define i32 @test_load(ptr %p) {
; CHECK-LABEL: test_load:
; CHECK: lw
  %val = load i32, ptr %p
  ret i32 %val
}

define void @test_store(ptr %p, i32 %val) {
; CHECK-LABEL: test_store:
; CHECK: sw
  store i32 %val, ptr %p
  ret void
}

define i8 @test_load_byte(ptr %p) {
; CHECK-LABEL: test_load_byte:
; CHECK: lb
  %val = load i8, ptr %p
  ret i8 %val
}

define void @test_store_byte(ptr %p, i8 %val) {
; CHECK-LABEL: test_store_byte:
; CHECK: sb
  store i8 %val, ptr %p
  ret void
}

define i16 @test_load_half(ptr %p) {
; CHECK-LABEL: test_load_half:
; CHECK: lh
  %val = load i16, ptr %p
  ret i16 %val
}

define void @test_store_half(ptr %p, i16 %val) {
; CHECK-LABEL: test_store_half:
; CHECK: sh
  store i16 %val, ptr %p
  ret void
}

define i32 @test_load_offset(ptr %p) {
; CHECK-LABEL: test_load_offset:
; CHECK: lw {{.*}}, 16({{.*}})
  %addr = getelementptr i8, ptr %p, i32 16
  %val = load i32, ptr %addr
  ret i32 %val
}
