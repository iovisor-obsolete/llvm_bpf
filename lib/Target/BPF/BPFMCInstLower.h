//===-- BPFMCInstLower.h - Lower MachineInstr to MCInst --------*- C++ -*-===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#ifndef BPF_MCINSTLOWER_H
#define BPF_MCINSTLOWER_H

#include "llvm/Support/Compiler.h"

namespace llvm {
  class AsmPrinter;
  class MCContext;
  class MCInst;
  class MCOperand;
  class MCSymbol;
  class MachineInstr;
  class MachineModuleInfoMachO;
  class MachineOperand;
  class Mangler;

  /// BPFMCInstLower - This class is used to lower an MachineInstr
  /// into an MCInst.
class LLVM_LIBRARY_VISIBILITY BPFMCInstLower {
  MCContext &Ctx;
  Mangler &Mang;

  AsmPrinter &Printer;
public:
  BPFMCInstLower(MCContext &ctx, Mangler &mang, AsmPrinter &printer)
    : Ctx(ctx), Mang(mang), Printer(printer) {}
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;

  MCOperand LowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const;

  MCSymbol *GetGlobalAddressSymbol(const MachineOperand &MO) const;
};

}

#endif
