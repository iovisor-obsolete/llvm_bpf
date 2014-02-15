//===-- BPFELFObjectWriter.cpp - BPF Writer -------------------------===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#include "MCTargetDesc/BPFBaseInfo.h"
#include "MCTargetDesc/BPFMCTargetDesc.h"
#include "MCTargetDesc/BPFMCCodeEmitter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCAsmLayout.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {
class BPFObjectWriter : public MCObjectWriter {
  public:
    BPFObjectWriter(raw_ostream &_OS):
      MCObjectWriter(_OS, true/*isLittleEndian*/) {}
    virtual ~BPFObjectWriter() {}
    virtual void WriteObject(MCAssembler &Asm, const MCAsmLayout &Layout);
    virtual void RecordRelocation(const MCAssembler &Asm,
                                  const MCAsmLayout &Layout,
                                  const MCFragment *Fragment,
                                  const MCFixup &Fixup,
                                  MCValue Target, uint64_t &FixedValue) {}
    virtual void ExecutePostLayoutBinding(MCAssembler &Asm,
                                          const MCAsmLayout &Layout) {}

};
}

static void WriteSectionData(MCAssembler &Asm, const MCSectionData &SD) {
  MCObjectWriter *OW = &Asm.getWriter();
  for (MCSectionData::const_iterator it = SD.begin(),
         ie = SD.end(); it != ie; ++it) {
    const MCFragment &F = *it;
    switch (F.getKind()) {
    case MCFragment::FT_Align:
      continue;
    case MCFragment::FT_Data: {
      const MCDataFragment &DF = cast<MCDataFragment>(F);
      OW->WriteBytes(DF.getContents());
      break;
    }
    case MCFragment::FT_Fill: {
      const MCFillFragment &FF = cast<MCFillFragment>(F);

      assert(FF.getValueSize() && "Invalid virtual align in concrete fragment!");

      for (uint64_t i = 0, e = FF.getSize() / FF.getValueSize(); i != e; ++i) {
        switch (FF.getValueSize()) {
        default: llvm_unreachable("Invalid size!");
        case 1: OW->Write8 (uint8_t (FF.getValue())); break;
        case 2: OW->Write16(uint16_t(FF.getValue())); break;
        case 4: OW->Write32(uint32_t(FF.getValue())); break;
        case 8: OW->Write64(uint64_t(FF.getValue())); break;
        }
      }
      break;
    }
    default:
      errs() << "MCFrag " << F.getKind() << "\n";
    }
  }
}

void BPFObjectWriter::WriteObject(MCAssembler &Asm,
                                  const MCAsmLayout &Layout) {
  bool LicenseSeen = false;
  MCObjectWriter *OW = &Asm.getWriter();
  OW->WriteBytes(StringRef("bpf"), 4);

  BPFMCCodeEmitter *CE = (BPFMCCodeEmitter*)(&Asm.getEmitter());
//  Asm.dump();
  for (MCAssembler::const_iterator i = Asm.begin(), e = Asm.end(); i != e;
       ++i) {
    const MCSectionELF &Section =
      static_cast<const MCSectionELF&>(i->getSection());
    const StringRef SectionName = Section.getSectionName();
    const MCSectionData &SD = Asm.getSectionData(Section);
    int SectionSize = Layout.getSectionAddressSize(&SD);
    if (SectionSize > 0) {
      CE->getStrtabIndex(SectionName);
      if (SectionName == "license")
        LicenseSeen = true;
    }
  }

  if (!LicenseSeen)
      report_fatal_error("BPF source is missing license");

  OW->Write32(CE->Strtab->length());
  OW->WriteBytes(StringRef(*CE->Strtab));

  for (MCAssembler::const_iterator i = Asm.begin(), e = Asm.end(); i != e;
       ++i) {
    const MCSectionELF &Section =
      static_cast<const MCSectionELF&>(i->getSection());
    const StringRef SectionName = Section.getSectionName();
    const MCSectionData &SD = Asm.getSectionData(Section);
    int SectionSize = Layout.getSectionAddressSize(&SD);
    if (SectionSize > 0 &&
        /* ignore .rodata.* for now */
        !(Section.getFlags() & ELF::SHF_STRINGS)) {
      OW->Write32(SectionSize);
      OW->Write32(CE->getStrtabIndex(SectionName));
      WriteSectionData(Asm, SD);
    }
  }
}

MCObjectWriter *llvm::createBPFELFObjectWriter(raw_ostream &OS,
                                               uint8_t OSABI) {
  return new BPFObjectWriter(OS);
}
