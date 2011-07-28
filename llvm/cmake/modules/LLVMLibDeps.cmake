set(MSVC_LIB_DEPS_LLVMARMAsmParser LLVMARMDesc LLVMARMInfo LLVMMC LLVMMCParser LLVMSupport)
set(MSVC_LIB_DEPS_LLVMARMAsmPrinter LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMARMCodeGen LLVMARMAsmPrinter LLVMARMDesc LLVMARMInfo LLVMAnalysis LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMC LLVMSelectionDAG LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMARMDesc LLVMARMInfo LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMARMDisassembler LLVMARMDesc LLVMARMInfo LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMARMInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMAlphaCodeGen LLVMAlphaDesc LLVMAlphaInfo LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMC LLVMSelectionDAG LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMAlphaDesc LLVMAlphaInfo LLVMMC)
set(MSVC_LIB_DEPS_LLVMAlphaInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMAnalysis LLVMCore LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMArchive LLVMBitReader LLVMCore LLVMSupport)
set(MSVC_LIB_DEPS_LLVMAsmParser LLVMCore LLVMSupport)
set(MSVC_LIB_DEPS_LLVMAsmPrinter LLVMAnalysis LLVMCodeGen LLVMCore LLVMMC LLVMMCParser LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMBitReader LLVMCore LLVMSupport)
set(MSVC_LIB_DEPS_LLVMBitWriter LLVMCore LLVMSupport)
set(MSVC_LIB_DEPS_LLVMBlackfinCodeGen LLVMAsmPrinter LLVMBlackfinDesc LLVMBlackfinInfo LLVMCodeGen LLVMCore LLVMMC LLVMSelectionDAG LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMBlackfinDesc LLVMBlackfinInfo LLVMMC)
set(MSVC_LIB_DEPS_LLVMBlackfinInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMCBackend LLVMAnalysis LLVMCBackendInfo LLVMCodeGen LLVMCore LLVMMC LLVMScalarOpts LLVMSupport LLVMTarget LLVMTransformUtils)
set(MSVC_LIB_DEPS_LLVMCBackendInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMCellSPUCodeGen LLVMAsmPrinter LLVMCellSPUDesc LLVMCellSPUInfo LLVMCodeGen LLVMCore LLVMMC LLVMSelectionDAG LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMCellSPUDesc LLVMCellSPUInfo LLVMMC)
set(MSVC_LIB_DEPS_LLVMCellSPUInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMCodeGen LLVMAnalysis LLVMCore LLVMMC LLVMScalarOpts LLVMSupport LLVMTarget LLVMTransformUtils)
set(MSVC_LIB_DEPS_LLVMCore LLVMSupport)
set(MSVC_LIB_DEPS_LLVMCppBackend LLVMCore LLVMCppBackendInfo LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMCppBackendInfo LLVMMC LLVMTarget)
set(MSVC_LIB_DEPS_LLVMExecutionEngine LLVMCore LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMInstCombine LLVMAnalysis LLVMCore LLVMSupport LLVMTarget LLVMTransformUtils)
set(MSVC_LIB_DEPS_LLVMInstrumentation LLVMAnalysis LLVMCore LLVMSupport LLVMTransformUtils)
set(MSVC_LIB_DEPS_LLVMInterpreter LLVMCodeGen LLVMCore LLVMExecutionEngine LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMJIT LLVMCodeGen LLVMCore LLVMExecutionEngine LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMLinker LLVMArchive LLVMBitReader LLVMCore LLVMSupport LLVMTransformUtils)
set(MSVC_LIB_DEPS_LLVMMBlazeAsmParser LLVMMBlazeInfo LLVMMC LLVMMCParser LLVMSupport)
set(MSVC_LIB_DEPS_LLVMMBlazeAsmPrinter LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMMBlazeCodeGen LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMBlazeAsmPrinter LLVMMBlazeDesc LLVMMBlazeInfo LLVMMC LLVMSelectionDAG LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMMBlazeDesc LLVMMBlazeInfo LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMMBlazeDisassembler LLVMMBlazeDesc LLVMMBlazeInfo LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMMBlazeInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMMCDisassembler LLVMARMAsmParser LLVMARMDesc LLVMARMDisassembler LLVMARMInfo LLVMAlphaDesc LLVMAlphaInfo LLVMBlackfinDesc LLVMBlackfinInfo LLVMCBackendInfo LLVMCellSPUDesc LLVMCellSPUInfo LLVMCppBackendInfo LLVMMBlazeAsmParser LLVMMBlazeDesc LLVMMBlazeDisassembler LLVMMBlazeInfo LLVMMC LLVMMCParser LLVMMSP430Desc LLVMMSP430Info LLVMMipsDesc LLVMMipsInfo LLVMPTXDesc LLVMPTXInfo LLVMPowerPCDesc LLVMPowerPCInfo LLVMSparcDesc LLVMSparcInfo LLVMSupport LLVMSystemZDesc LLVMSystemZInfo LLVMTarget LLVMX86AsmParser LLVMX86Desc LLVMX86Disassembler LLVMX86Info LLVMXCoreDesc LLVMXCoreInfo)
set(MSVC_LIB_DEPS_LLVMMCJIT LLVMCore LLVMExecutionEngine LLVMRuntimeDyld LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMMCParser LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMMSP430AsmPrinter LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMMSP430CodeGen LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMC LLVMMSP430AsmPrinter LLVMMSP430Desc LLVMMSP430Info LLVMSelectionDAG LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMMSP430Desc LLVMMC LLVMMSP430Info)
set(MSVC_LIB_DEPS_LLVMMSP430Info LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMMipsAsmPrinter LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMMipsCodeGen LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMC LLVMMipsAsmPrinter LLVMMipsDesc LLVMMipsInfo LLVMSelectionDAG LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMMipsDesc LLVMMC LLVMMipsInfo LLVMSupport)
set(MSVC_LIB_DEPS_LLVMMipsInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMObject LLVMSupport)
set(MSVC_LIB_DEPS_LLVMPTXCodeGen LLVMAnalysis LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMC LLVMPTXDesc LLVMPTXInfo LLVMSelectionDAG LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMPTXDesc LLVMMC LLVMPTXInfo LLVMSupport)
set(MSVC_LIB_DEPS_LLVMPTXInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMPowerPCAsmPrinter LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMPowerPCCodeGen LLVMAnalysis LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMC LLVMPowerPCAsmPrinter LLVMPowerPCDesc LLVMPowerPCInfo LLVMSelectionDAG LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMPowerPCDesc LLVMMC LLVMPowerPCInfo LLVMSupport)
set(MSVC_LIB_DEPS_LLVMPowerPCInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMRuntimeDyld LLVMObject LLVMSupport)
set(MSVC_LIB_DEPS_LLVMScalarOpts LLVMAnalysis LLVMCore LLVMInstCombine LLVMSupport LLVMTarget LLVMTransformUtils)
set(MSVC_LIB_DEPS_LLVMSelectionDAG LLVMAnalysis LLVMCodeGen LLVMCore LLVMMC LLVMSupport LLVMTarget LLVMTransformUtils)
set(MSVC_LIB_DEPS_LLVMSparcCodeGen LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMC LLVMSelectionDAG LLVMSparcDesc LLVMSparcInfo LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMSparcDesc LLVMMC LLVMSparcInfo LLVMSupport)
set(MSVC_LIB_DEPS_LLVMSparcInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMSupport )
set(MSVC_LIB_DEPS_LLVMSystemZCodeGen LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMC LLVMSelectionDAG LLVMSupport LLVMSystemZDesc LLVMSystemZInfo LLVMTarget)
set(MSVC_LIB_DEPS_LLVMSystemZDesc LLVMMC LLVMSystemZInfo)
set(MSVC_LIB_DEPS_LLVMSystemZInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMTarget LLVMCore LLVMMC LLVMSupport)
set(MSVC_LIB_DEPS_LLVMTransformUtils LLVMAnalysis LLVMCore LLVMSupport LLVMTarget LLVMipa)
set(MSVC_LIB_DEPS_LLVMX86AsmParser LLVMMC LLVMMCParser LLVMSupport LLVMX86Desc LLVMX86Info)
set(MSVC_LIB_DEPS_LLVMX86AsmPrinter LLVMMC LLVMSupport LLVMX86Utils)
set(MSVC_LIB_DEPS_LLVMX86CodeGen LLVMAnalysis LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMC LLVMSelectionDAG LLVMSupport LLVMTarget LLVMX86AsmPrinter LLVMX86Desc LLVMX86Info LLVMX86Utils)
set(MSVC_LIB_DEPS_LLVMX86Desc LLVMMC LLVMSupport LLVMX86AsmPrinter LLVMX86Info)
set(MSVC_LIB_DEPS_LLVMX86Disassembler LLVMMC LLVMSupport LLVMX86Info)
set(MSVC_LIB_DEPS_LLVMX86Info LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMX86Utils LLVMCore LLVMSupport)
set(MSVC_LIB_DEPS_LLVMXCoreCodeGen LLVMAsmPrinter LLVMCodeGen LLVMCore LLVMMC LLVMSelectionDAG LLVMSupport LLVMTarget LLVMXCoreDesc LLVMXCoreInfo)
set(MSVC_LIB_DEPS_LLVMXCoreDesc LLVMMC LLVMXCoreInfo)
set(MSVC_LIB_DEPS_LLVMXCoreInfo LLVMMC LLVMSupport LLVMTarget)
set(MSVC_LIB_DEPS_LLVMipa LLVMAnalysis LLVMCore LLVMSupport)
set(MSVC_LIB_DEPS_LLVMipo LLVMAnalysis LLVMCore LLVMScalarOpts LLVMSupport LLVMTarget LLVMTransformUtils LLVMipa)
