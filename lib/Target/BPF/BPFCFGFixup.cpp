//===-- BPFCFGFixup.cpp - CFG fixup pass -----------------------===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#define DEBUG_TYPE "bpf_cfg"
#include "BPF.h"
#include "BPFInstrInfo.h"
#include "BPFSubtarget.h"
#include "BPFTargetMachine.h"
#include "BPFSubtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"

using namespace llvm;

namespace {

class BPFCFGFixup : public MachineFunctionPass {
 private:
  BPFTargetMachine& QTM;
  const BPFSubtarget &QST;

  void InvertAndChangeJumpTarget(MachineInstr*, MachineBasicBlock*);

 public:
  static char ID;
  BPFCFGFixup(BPFTargetMachine& TM) : MachineFunctionPass(ID),
                                                  QTM(TM),
                                                  QST(*TM.getSubtargetImpl()) {}

  const char *getPassName() const {
    return "BPF RET insn fixup";
  }
  bool runOnMachineFunction(MachineFunction &Fn);
};

char BPFCFGFixup::ID = 0;

bool BPFCFGFixup::runOnMachineFunction(MachineFunction &Fn) {

  // Loop over all of the basic blocks.
  for (MachineFunction::iterator MBBb = Fn.begin(), MBBe = Fn.end();
       MBBb != MBBe; ++MBBb) {
    MachineBasicBlock* MBB = MBBb;

    MachineBasicBlock::iterator MII = MBB->getFirstTerminator();
    if (MII != MBB->end()) {
      /* if last insn of this basic block is RET, make this BB last */
      if (MII->getOpcode() == BPF::RET) {
        MBBe--;
        if (MBB != MBBe)
          MBB->moveAfter(MBBe);
        break;
      }
    }
  }
  return true;
}
}

FunctionPass *llvm::createBPFCFGFixup(BPFTargetMachine &TM) {
  return new BPFCFGFixup(TM);
}
