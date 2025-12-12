add_llvm_component_library(LLVMTritonDesc
  TritonMCTargetDesc.cpp
  TritonMCAsmInfo.cpp
  TritonELFObjectWriter.cpp
  TritonMCCodeEmitter.cpp
  TritonAsmBackend.cpp
  TritonInstPrinter.cpp

  LINK_COMPONENTS
  MC
  Support
  TritonInfo

  ADD_TO_COMPONENT
  Triton
 )
