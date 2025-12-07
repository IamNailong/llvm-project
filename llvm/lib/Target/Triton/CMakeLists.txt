add_llvm_component_group(Triton)

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
