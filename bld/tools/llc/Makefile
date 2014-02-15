#===- tools/llc/Makefile -----------------------------------*- Makefile -*-===##
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.

LEVEL := ../..
TOOLNAME := llc
ifneq (,$(filter $(shell $(LLVM_CONFIG) --version),3.3 3.4))
LINK_COMPONENTS := asmparser asmprinter codegen bitreader core mc selectiondag support target irreader
else
LINK_COMPONENTS := asmparser asmprinter codegen bitreader core mc selectiondag support target
endif
USEDLIBS := LLVMBPFCodeGen.a LLVMBPFDesc.a LLVMBPFInfo.a LLVMBPFAsmPrinter.a

include $(LEVEL)/Makefile.common

