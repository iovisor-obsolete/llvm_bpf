//===-- BPFFrameLowering.h - Define frame lowering for BPF ---*- C++ -*--===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#ifndef BPF_FRAMEINFO_H
#define BPF_FRAMEINFO_H

#include "BPF.h"
#include "BPFSubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
class BPFSubtarget;

class BPFFrameLowering : public TargetFrameLowering {
public:
  explicit BPFFrameLowering(const BPFSubtarget &sti)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 8, 0) {
  }

  void emitPrologue(MachineFunction &MF) const;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

  bool hasFP(const MachineFunction &MF) const;
  virtual void processFunctionBeforeCalleeSavedScan(MachineFunction &MF,
                                                    RegScavenger *RS) const;

  // llvm 3.3 defines it here
  void eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator MI) const {
    MBB.erase(MI);
  }
};
}
#endif
