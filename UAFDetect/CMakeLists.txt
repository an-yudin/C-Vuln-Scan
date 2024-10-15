add_library(LLVMUAFDetect MODULE Source.cpp)

set_target_properties(LLVMUAFDetect PROPERTIES COMPILE_FLAGS "-fno-rtti")
