//===-- BPFTargetMachine.h - Define TargetMachine for BPF --- C++ ---===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This file declares the BPF specific subclass of TargetMachine.

#ifndef BPF_TARGETMACHINE_H
#define BPF_TARGETMACHINE_H

#include "BPFSubtarget.h"
#include "BPFInstrInfo.h"
#include "BPFISelLowering.h"
#include "llvm/Target/TargetSelectionDAGInfo.h"
#include "BPFFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#if !defined(LLVM_VERSION_MINOR)
#error "Uknown version"
#endif
#if LLVM_VERSION_MINOR==3 || LLVM_VERSION_MINOR==4
#include "llvm/IR/DataLayout.h"
#else
#include "llvm/DataLayout.h"
#endif
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class formatted_raw_ostream;

  class BPFTargetMachine : public LLVMTargetMachine {
    BPFSubtarget       Subtarget;
    const DataLayout   DL; // Calculates type size & alignment
    BPFInstrInfo       InstrInfo;
    BPFTargetLowering  TLInfo;
    TargetSelectionDAGInfo TSInfo;
    BPFFrameLowering   FrameLowering;
  public:
    BPFTargetMachine(const Target &T, StringRef TT,
                        StringRef CPU, StringRef FS,
                        const TargetOptions &Options,
                        Reloc::Model RM, CodeModel::Model CM,
                        CodeGenOpt::Level OL);

    virtual const BPFInstrInfo *getInstrInfo() const
    { return &InstrInfo; }

    virtual const TargetFrameLowering *getFrameLowering() const
    { return &FrameLowering; }

    virtual const BPFSubtarget *getSubtargetImpl() const
    { return &Subtarget; }

    virtual const DataLayout *getDataLayout() const
    { return &DL;}

    virtual const BPFRegisterInfo *getRegisterInfo() const
    { return &InstrInfo.getRegisterInfo(); }

    virtual const BPFTargetLowering *getTargetLowering() const
    { return &TLInfo; }

    virtual const TargetSelectionDAGInfo* getSelectionDAGInfo() const
    { return &TSInfo; }

    // Pass Pipeline Configuration
    virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);
  };
}

#endif
