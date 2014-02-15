//===-- BPFMCCodeEmitter.h - Convert BPF code to machine code ---------===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#include "MCTargetDesc/BPFBaseInfo.h"
#include "MCTargetDesc/BPFMCTargetDesc.h"
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

namespace {
class BPFMCCodeEmitter : public MCCodeEmitter {
  BPFMCCodeEmitter(const BPFMCCodeEmitter &);
  void operator=(const BPFMCCodeEmitter &);
  const MCInstrInfo &MCII;
  const MCSubtargetInfo &STI;
  MCContext &Ctx;

public:
  BPFMCCodeEmitter(const MCInstrInfo &mcii, const MCSubtargetInfo &sti,
                    MCContext &ctx)
    : MCII(mcii), STI(sti), Ctx(ctx) {
      Strtab = new std::string;
      Strtab->push_back('\0');
    }

  ~BPFMCCodeEmitter() {delete Strtab;}

  std::string *Strtab;

  int getStrtabIndex(const StringRef Name) const {
    std::string Sym = Name.str();
    Sym.push_back('\0');

    std::string::size_type pos = Strtab->find(Sym);
    if (pos == std::string::npos) {
      Strtab->append(Sym);
      pos = Strtab->find(Sym);
      assert (pos != std::string::npos);
    }
    return pos;
  }

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups) const;

   // getMachineOpValue - Return binary encoding of operand. If the machin
   // operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI,const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups) const;

  uint64_t getMemoryOpValue(const MCInst &MI, unsigned Op,
                            SmallVectorImpl<MCFixup> &Fixups) const;

  void EncodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups) const;
};
}
