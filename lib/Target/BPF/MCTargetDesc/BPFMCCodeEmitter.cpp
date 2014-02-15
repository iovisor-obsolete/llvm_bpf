//===-- BPFMCCodeEmitter.cpp - Convert BPF code to machine code ---------===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#define DEBUG_TYPE "mccodeemitter"
#include "MCTargetDesc/BPFBaseInfo.h"
#include "MCTargetDesc/BPFMCTargetDesc.h"
#include "MCTargetDesc/BPFMCCodeEmitter.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");

MCCodeEmitter *llvm::createBPFMCCodeEmitter(const MCInstrInfo &MCII,
                                             const MCRegisterInfo &MRI,
                                             const MCSubtargetInfo &STI,
                                             MCContext &Ctx) {
  return new BPFMCCodeEmitter(MCII, STI, Ctx);
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned BPFMCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups) const {
  if (MO.isReg())
    return getBPFRegisterNumbering(MO.getReg());
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());
  
  assert(MO.isExpr());

  const MCExpr *Expr = MO.getExpr();
  MCExpr::ExprKind Kind = Expr->getKind();

/*  if (Kind == MCExpr::Binary) {
    Expr = static_cast<const MCBinaryExpr*>(Expr)->getLHS();
    Kind = Expr->getKind();
  }*/

  assert (Kind == MCExpr::SymbolRef);

  if (MI.getOpcode() == BPF::JAL) {
    /* func call name */
//    Fixups.push_back(MCFixup::Create(0, MO.getExpr(), FK_SecRel_4));
    const MCSymbolRefExpr *SRE = dyn_cast<MCSymbolRefExpr>(Expr);
    return getStrtabIndex(SRE->getSymbol().getName());

  } else {
    /* bb label */
    Fixups.push_back(MCFixup::Create(0, MO.getExpr(), FK_PCRel_2));
    return 0;
  }
}

// Emit one byte through output stream
void EmitByte(unsigned char C, unsigned &CurByte, raw_ostream &OS) {
  OS << (char)C;
  ++CurByte;
}

// Emit a series of bytes (little endian)
void EmitLEConstant(uint64_t Val, unsigned Size, unsigned &CurByte,
                    raw_ostream &OS) {
  assert(Size <= 8 && "size too big in emit constant");

  for (unsigned i = 0; i != Size; ++i) {
    EmitByte(Val & 255, CurByte, OS);
    Val >>= 8;
  }
}

// Emit a series of bytes (big endian)
void EmitBEConstant(uint64_t Val, unsigned Size, unsigned &CurByte,
                    raw_ostream &OS) {
  assert(Size <= 8 && "size too big in emit constant");

  for (int i = (Size-1)*8; i >= 0; i-=8)
    EmitByte((Val >> i) & 255, CurByte, OS);
}

void BPFMCCodeEmitter::EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                                         SmallVectorImpl<MCFixup> &Fixups) const {
//  unsigned Opcode = MI.getOpcode();
//  const MCInstrDesc &Desc = MCII.get(Opcode);
  // Keep track of the current byte being emitted
  unsigned CurByte = 0;

  // Get instruction encoding and emit it
  ++MCNumEmitted;       // Keep track of the number of emitted insns.
  uint64_t Value = getBinaryCodeForInstr(MI, Fixups);
  EmitByte(Value >> 56, CurByte, OS);
  EmitByte((Value >> 48) & 0xff, CurByte, OS);
  EmitLEConstant((Value >> 32) & 0xffff, 2, CurByte, OS);
  EmitLEConstant(Value & 0xffffFFFF, 4, CurByte, OS);
}

// Encode BPF Memory Operand
uint64_t BPFMCCodeEmitter::getMemoryOpValue(const MCInst &MI, unsigned Op,
                                            SmallVectorImpl<MCFixup> &Fixups) const {
  uint64_t encoding;
  const MCOperand op1 = MI.getOperand(1);
  assert(op1.isReg() && "First operand is not register.");
  encoding = getBPFRegisterNumbering(op1.getReg());
  encoding <<= 16;
  MCOperand op2 = MI.getOperand(2);
  assert(op2.isImm() && "Second operand is not immediate.");
  encoding |= op2.getImm() & 0xffff;
  return encoding;
}

#include "BPFGenMCCodeEmitter.inc"
