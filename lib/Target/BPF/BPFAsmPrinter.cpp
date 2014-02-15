//===-- BPFAsmPrinter.cpp - BPF LLVM assembly writer --------------------===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the BPF assembly language.

#define DEBUG_TYPE "asm-printer"
#include "BPF.h"
#include "BPFInstrInfo.h"
#include "BPFMCInstLower.h"
#include "BPFTargetMachine.h"
#include "InstPrinter/BPFInstPrinter.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
  class BPFAsmPrinter : public AsmPrinter {
  public:
    explicit BPFAsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
      : AsmPrinter(TM, Streamer) {}

    virtual const char *getPassName() const {
      return "BPF Assembly Printer";
    }

    void printOperand(const MachineInstr *MI, int OpNum,
                      raw_ostream &O, const char* Modifier = 0);
    void EmitInstruction(const MachineInstr *MI);
  private:
    void customEmitInstruction(const MachineInstr *MI);
  };
}

void BPFAsmPrinter::printOperand(const MachineInstr *MI, int OpNum,
                                  raw_ostream &O, const char *Modifier) {
  const MachineOperand &MO = MI->getOperand(OpNum);

  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    O << BPFInstPrinter::getRegisterName(MO.getReg());
    break;

  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    break;

  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    break;

  case MachineOperand::MO_GlobalAddress:
#if LLVM_VERSION_MINOR==4
      O << *getSymbol(MO.getGlobal());
#else
      O << *Mang->getSymbol(MO.getGlobal());
#endif
    break;

  default:
    llvm_unreachable("<unknown operand type>");
    O << "bug";
    return;
  }
}

void BPFAsmPrinter::customEmitInstruction(const MachineInstr *MI) {
  BPFMCInstLower MCInstLowering(OutContext, *Mang, *this);

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  OutStreamer.EmitInstruction(TmpInst);
}

void BPFAsmPrinter::EmitInstruction(const MachineInstr *MI) {

  MachineBasicBlock::const_instr_iterator I = MI;
  MachineBasicBlock::const_instr_iterator E = MI->getParent()->instr_end();

  do {
    customEmitInstruction(I++);
  } while ((I != E) && I->isInsideBundle());
}

// Force static initialization.
extern "C" void LLVMInitializeBPFAsmPrinter() {
  RegisterAsmPrinter<BPFAsmPrinter> X(TheBPFTarget);
}
