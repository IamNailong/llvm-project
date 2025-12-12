add_llvm_component_group(Triton)

set(LLVM_TARGET_DEFINITIONS Triton.td)

tablegen(LLVM TritonGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM TritonGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM TritonGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM TritonGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM TritonGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(TritonCommonTableGen)

add_llvm_target(TritonCodeGen
  TritonTargetMachine.cpp

  LINK_COMPONENTS
  CodeGen
  Core
  Support
  Target
  TritonInfo

  ADD_TO_COMPONENT
  Triton
  )

add_subdirectory(AsmParser)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
