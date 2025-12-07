add_llvm_component_library(LLVMTritonInfo
  TritonTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  Triton
  )
