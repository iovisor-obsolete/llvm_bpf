//===- BPFRegisterInfo.h - BPF Register Information Impl ------*- C++ -*-===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This file contains the BPF implementation of the TargetRegisterInfo class.

#ifndef BPFREGISTERINFO_H
#define BPFREGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "BPFGenRegisterInfo.inc"

namespace llvm {

class TargetInstrInfo;
class Type;

struct BPFRegisterInfo : public BPFGenRegisterInfo {
  const TargetInstrInfo &TII;

  BPFRegisterInfo(const TargetInstrInfo &tii);

  /// Code Generation virtual methods...
  const uint16_t *getCalleeSavedRegs(const MachineFunction *MF = 0) const;

  BitVector getReservedRegs(const MachineFunction &MF) const;

  bool requiresRegisterScavenging(const MachineFunction &MF) const;

  // llvm 3.2 defines it here
  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const {
    // Discard ADJCALLSTACKDOWN, ADJCALLSTACKUP instructions.
    MBB.erase(I);
  }

#if LLVM_VERSION_MINOR==3 || LLVM_VERSION_MINOR==4
  void eliminateFrameIndex(MachineBasicBlock::iterator MI,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS = NULL) const;
#else
  void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, RegScavenger *RS = NULL) const;
#endif

  void processFunctionBeforeFrameFinalized(MachineFunction &MF) const;

  bool hasBasePointer(const MachineFunction &MF) const;
  bool needsStackRealignment(const MachineFunction &MF) const;

  // Debug information queries.
  unsigned getRARegister() const;
  unsigned getFrameRegister(const MachineFunction &MF) const;
  unsigned getBaseRegister() const;

  // Exception handling queries.
  unsigned getEHExceptionRegister() const;
  unsigned getEHHandlerRegister() const;
  int getDwarfRegNum(unsigned RegNum, bool isEH) const;
};
}
#endif
