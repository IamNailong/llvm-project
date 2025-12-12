add_llvm_component_group(Triton)

set(LLVM_TARGET_DEFINITIONS Triton.td)

tablegen(LLVM TritonGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM TritonGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM TritonGenMCCodeEmitter.inc -gen-emitter)

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

add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
