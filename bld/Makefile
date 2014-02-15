#===- ./Makefile -------------------------------------------*- Makefile -*--===#
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.

ifndef LLVM_CONFIG
LLVM_CONFIG := llvm-config-3.2
export LLVM_CONFIG
endif

LEVEL := .

DIRS := lib tools

include $(LEVEL)/Makefile.config

# Include the main makefile machinery.
include $(LLVM_SRC_ROOT)/Makefile.rules

# NOTE: This needs to remain as the last target definition in this file so
# that it gets executed last.
all::
	$(Echo) '*****' Completed $(BuildMode) Build

# declare all targets at this level to be serial:
.NOTPARALLEL:

