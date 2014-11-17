//===-- BPFAsmBackend.cpp - BPF Assembler Backend -----------------------===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#include "MCTargetDesc/BPFMCTargetDesc.h"
#include "llvm/MC/MCAsmBackend.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class BPFAsmBackend : public MCAsmBackend {
public:
  BPFAsmBackend(): MCAsmBackend() {}
  virtual ~BPFAsmBackend() {}

  void applyFixup(const MCFixup &Fixup, char *Data, unsigned DataSize,
                  uint64_t Value) const;

  MCObjectWriter *createObjectWriter(raw_ostream &OS) const;

  // No instruction requires relaxation
#if LLVM_VERSION_MINOR==3 || LLVM_VERSION_MINOR==4
  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const { return false; }
#else
  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value, 
                            const MCInstFragment *DF,
                            const MCAsmLayout &Layout) const { return false; }
#endif
  
  unsigned getNumFixupKinds() const { return 1; }

  bool mayNeedRelaxation(const MCInst &Inst) const { return false; }

  void relaxInstruction(const MCInst &Inst, MCInst &Res) const {}

  bool writeNopData(uint64_t Count, MCObjectWriter *OW) const;
};

bool BPFAsmBackend::writeNopData(uint64_t Count, MCObjectWriter *OW) const {
  if ((Count % 8) != 0)
    return false;

  for (uint64_t i = 0; i < Count; i += 8)
    OW->Write64(0x15000000);

  return true;
}

void BPFAsmBackend::applyFixup(const MCFixup &Fixup, char *Data,
                               unsigned DataSize, uint64_t Value) const {

  if (0)
   errs() << "<MCFixup" << " Offset:" << Fixup.getOffset() << " Value:" <<
     *(Fixup.getValue()) << " Kind:" << Fixup.getKind() <<
     " val " << Value << ">\n";

  if (Fixup.getKind() == FK_SecRel_4 || Fixup.getKind() == FK_SecRel_8) {
    assert (Value == 0);
    return;
  }
  assert (Fixup.getKind() == FK_PCRel_2);
  *(uint16_t*)&Data[Fixup.getOffset() + 2] = (uint16_t) ((Value - 8) / 8);
}

MCObjectWriter *BPFAsmBackend::createObjectWriter(raw_ostream &OS) const {
  return createBPFELFObjectWriter(OS, 0);
}

}

MCAsmBackend *llvm::createBPFAsmBackend(const Target &T,
#if LLVM_VERSION_MINOR==4
                                        const MCRegisterInfo &MRI,
#endif
                                        StringRef TT, StringRef CPU) {
  return new BPFAsmBackend();
}
