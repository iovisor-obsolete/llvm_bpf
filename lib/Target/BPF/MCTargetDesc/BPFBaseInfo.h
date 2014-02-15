//===-- BPFBaseInfo.h - Top level definitions for BPF MC ------*- C++ -*-===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
#ifndef BPFBASEINFO_H
#define BPFBASEINFO_H

#include "BPFMCTargetDesc.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

static inline unsigned getBPFRegisterNumbering(unsigned Reg) {
  switch(Reg) {
    case BPF::R0  : return 0;
    case BPF::R1  : return 1;
    case BPF::R2  : return 2;
    case BPF::R3  : return 3;
    case BPF::R4  : return 4;
    case BPF::R5  : return 5;
    case BPF::R6  : return 6;
    case BPF::R7  : return 7;
    case BPF::R8  : return 8;
    case BPF::R9  : return 9;
    case BPF::R10 : return 10;
    case BPF::R11 : return 11;
    default: llvm_unreachable("Unknown register number!");
  }
}

}
#endif
