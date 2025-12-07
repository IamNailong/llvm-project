add_llvm_component_library(LLVMTritonDesc
  TritonMCTargetDesc.cpp

  LINK_COMPONENTS
  MC
  Support
  TritonInfo

  ADD_TO_COMPONENT
  Triton
 )
