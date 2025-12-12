add_llvm_component_library(LLVMTritonAsmParser
  TritonAsmParser.cpp

  LINK_COMPONENTS
  MC
  MCParser
  Support
  TritonDesc
  TritonInfo

  ADD_TO_COMPONENT
  Triton
)
