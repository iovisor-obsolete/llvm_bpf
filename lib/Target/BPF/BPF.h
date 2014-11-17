//===-- BPF.h - Top-level interface for BPF representation ----*- C++ -*-===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#ifndef TARGET_BPF_H
#define TARGET_BPF_H
#include "llvm/Config/config.h"
#undef LLVM_NATIVE_TARGET
#undef LLVM_NATIVE_ASMPRINTER
#undef LLVM_NATIVE_ASMPARSER
#undef LLVM_NATIVE_DISASSEMBLER
#include "MCTargetDesc/BPFBaseInfo.h"
#include "MCTargetDesc/BPFMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class FunctionPass;
class TargetMachine;
class BPFTargetMachine;

/// createBPFISelDag - This pass converts a legalized DAG into a
/// BPF-specific DAG, ready for instruction scheduling.
FunctionPass *createBPFISelDag(BPFTargetMachine &TM);

extern Target TheBPFTarget;
}

#endif
