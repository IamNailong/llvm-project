add_llvm_component_group(Triton)

set(LLVM_TARGET_DEFINITIONS Triton.td)

tablegen(LLVM TritonGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM TritonGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM TritonGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM TritonGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM TritonGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM TritonGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM TritonGenCallingConv.inc -gen-callingconv)
tablegen(LLVM TritonGenDAGISel.inc -gen-dag-isel)

add_public_tablegen_target(TritonCommonTableGen)

add_llvm_target(TritonCodeGen
  TritonTargetMachine.cpp
  TritonSubtarget.cpp
  TritonInstrInfo.cpp
  TritonISelLowering.cpp
  TritonISelDAGToDAG.cpp
  TritonFrameLowering.cpp
  TritonRegisterInfo.cpp
  TritonMachineFunctionInfo.cpp
  TritonAsmPrinter.cpp
  TritonMCInstLower.cpp

  LINK_COMPONENTS
  AsmPrinter
  CodeGen
  Core
  MC
  SelectionDAG
  Support
  Target
  TritonDesc
  TritonInfo

  ADD_TO_COMPONENT
  Triton
  )

add_subdirectory(AsmParser)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
