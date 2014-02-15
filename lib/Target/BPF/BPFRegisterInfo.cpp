//===-- BPFRegisterInfo.cpp - BPF Register Information --------*- C++ -*-===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This file contains the BPF implementation of the TargetRegisterInfo class.

#include "BPF.h"
#include "BPFRegisterInfo.h"
#include "BPFSubtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "BPFGenRegisterInfo.inc"
using namespace llvm;

BPFRegisterInfo::BPFRegisterInfo(const TargetInstrInfo &tii)
  : BPFGenRegisterInfo(BPF::R0), TII(tii) {
}

const uint16_t*
BPFRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

BitVector BPFRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(BPF::R10);
  Reserved.set(BPF::R11);
  return Reserved;
}

bool
BPFRegisterInfo::requiresRegisterScavenging(const MachineFunction &MF) const {
  return true;
}

void
#if LLVM_VERSION_MINOR==3 || LLVM_VERSION_MINOR==4
BPFRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, unsigned FIOperandNum,
                           RegScavenger *RS) const {
#else
BPFRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                       int SPAdj, RegScavenger *RS) const {
#endif
  assert(SPAdj == 0 && "Unexpected");

  unsigned i = 0;
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  DebugLoc dl = MI.getDebugLoc();

  while (!MI.getOperand(i).isFI()) {
    ++i;
    assert(i < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  unsigned FrameReg = getFrameRegister(MF);
  int FrameIndex = MI.getOperand(i).getIndex();

  if (MI.getOpcode() == BPF::MOV_rr) {
    int Offset = MF.getFrameInfo()->getObjectOffset(FrameIndex);

    MI.getOperand(i).ChangeToRegister(FrameReg, false);

    MachineBasicBlock &MBB = *MI.getParent();
    unsigned reg = MI.getOperand(i - 1).getReg();
    BuildMI(MBB, ++ II, dl, TII.get(BPF::ADD_ri), reg)
       .addReg(reg).addImm(Offset);
    return;
  }

  int Offset = MF.getFrameInfo()->getObjectOffset(FrameIndex) +
               MI.getOperand(i+1).getImm();

  if (!isInt<32>(Offset)) {
    llvm_unreachable("bug in frame offset");
  }

  MI.getOperand(i).ChangeToRegister(FrameReg, false);
  MI.getOperand(i+1).ChangeToImmediate(Offset);
}

void BPFRegisterInfo::
processFunctionBeforeFrameFinalized(MachineFunction &MF) const {}

bool BPFRegisterInfo::hasBasePointer(const MachineFunction &MF) const {
   return false;
}

bool BPFRegisterInfo::needsStackRealignment(const MachineFunction &MF) const {
  return false;
}

unsigned BPFRegisterInfo::getRARegister() const {
  return BPF::R0;
}

unsigned BPFRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return BPF::R10;
}

unsigned BPFRegisterInfo::getBaseRegister() const {
  llvm_unreachable("What is the base register");
  return 0;
}

unsigned BPFRegisterInfo::getEHExceptionRegister() const {
  llvm_unreachable("What is the exception register");
  return 0;
}

unsigned BPFRegisterInfo::getEHHandlerRegister() const {
  llvm_unreachable("What is the exception handler register");
  return 0;
}
