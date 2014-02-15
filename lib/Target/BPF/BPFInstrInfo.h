//===- BPFInstrInfo.h - BPF Instruction Information ---------*- C++ -*-===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#ifndef BPFINSTRUCTIONINFO_H
#define BPFINSTRUCTIONINFO_H

#include "BPFRegisterInfo.h"
#include "BPFSubtarget.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "BPFGenInstrInfo.inc"

namespace llvm {

class BPFInstrInfo : public BPFGenInstrInfo {
  const BPFRegisterInfo RI;
public:
  BPFInstrInfo();

  virtual const BPFRegisterInfo &getRegisterInfo() const { return RI; }

  virtual void copyPhysReg(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator I, DebugLoc DL,
                           unsigned DestReg, unsigned SrcReg,
                           bool KillSrc) const;

  virtual void storeRegToStackSlot(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MBBI,
                                   unsigned SrcReg, bool isKill, int FrameIndex,
                                   const TargetRegisterClass *RC,
                                   const TargetRegisterInfo *TRI) const;

  virtual void loadRegFromStackSlot(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MBBI,
                                    unsigned DestReg, int FrameIndex,
                                    const TargetRegisterClass *RC,
                                    const TargetRegisterInfo *TRI) const;
  bool AnalyzeBranch(MachineBasicBlock &MBB,
                     MachineBasicBlock *&TBB, MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify) const;

  unsigned RemoveBranch(MachineBasicBlock &MBB) const;
  unsigned InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB,
                        const SmallVectorImpl<MachineOperand> &Cond,
                        DebugLoc DL) const;
};
}

#endif
