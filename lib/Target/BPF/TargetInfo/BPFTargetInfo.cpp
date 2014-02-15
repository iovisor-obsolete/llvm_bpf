//===-- BPFTargetInfo.cpp - BPF Target Implementation -----------------===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#include "BPF.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheBPFTarget;

extern "C" void LLVMInitializeBPFTargetInfo() {
  RegisterTarget<Triple::x86_64> X(TheBPFTarget, "bpf", "BPF");
}
