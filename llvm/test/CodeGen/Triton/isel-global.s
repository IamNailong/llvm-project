	.file	"isel-global.ll"
	.text
	.global	test_load_global                # -- Begin function test_load_global
	.p2align	2
	.type	test_load_global,@function
test_load_global:                       # @test_load_global
	.cfi_startproc
# %bb.0:
	lui	t0, global_var
	lw	a0, global_var(t0)
	jalr	zero, 0(ra)
.Lfunc_end0:
	.size	test_load_global, .Lfunc_end0-test_load_global
	.cfi_endproc
                                        # -- End function
	.global	test_store_global               # -- Begin function test_store_global
	.p2align	2
	.type	test_store_global,@function
test_store_global:                      # @test_store_global
	.cfi_startproc
# %bb.0:
	lui	t0, global_var
	sw	a0, global_var(t0)
	jalr	zero, 0(ra)
.Lfunc_end1:
	.size	test_store_global, .Lfunc_end1-test_store_global
	.cfi_endproc
                                        # -- End function
	.global	test_load_global_array          # -- Begin function test_load_global_array
	.p2align	2
	.type	test_load_global_array,@function
test_load_global_array:                 # @test_load_global_array
	.cfi_startproc
# %bb.0:
	slli	t0, a0, 2
	lui	t1, global_array
	addi	t1, t1, global_array
	add	t0, t1, t0
	lw	a0, 0(t0)
	jalr	zero, 0(ra)
.Lfunc_end2:
	.size	test_load_global_array, .Lfunc_end2-test_load_global_array
	.cfi_endproc
                                        # -- End function
	.type	global_var,@object              # @global_var
	.section	.bss,"aw",@nobits
	.global	global_var
	.p2align	2, 0x0
global_var:
	.long	0                               # 0x0
	.size	global_var, 4

	.type	global_array,@object            # @global_array
	.global	global_array
	.p2align	4, 0x0
global_array:
	.space	40
	.size	global_array, 40

	.section	".note.GNU-stack","",@progbits
