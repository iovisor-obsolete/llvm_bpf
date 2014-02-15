//===-- BPFISelLowering.h - BPF DAG Lowering Interface -......-*- C++ -*-===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This file defines the interfaces that BPF uses to lower LLVM code into a
// selection DAG.

#ifndef LLVM_TARGET_BPF_ISELLOWERING_H
#define LLVM_TARGET_BPF_ISELLOWERING_H

#include "BPF.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Target/TargetLowering.h"

namespace llvm {
  namespace BPFISD {
    enum {
      FIRST_NUMBER = ISD::BUILTIN_OP_END,

      ADJDYNALLOC,

      /// Return with a flag operand. Operand 0 is the chain operand.
      RET_FLAG,

      /// CALL - These operations represent an abstract call instruction, which
      /// includes a bunch of information.
      CALL,

      /// SELECT_CC - Operand 0 and operand 1 are selection variable, operand 3
      /// is condition code and operand 4 is flag operand.
      SELECT_CC,

      // BR_CC - Used to glue together a l.bf to a l.sfXX
      BR_CC,

      /// Wrapper - A wrapper node for TargetConstantPool, TargetExternalSymbol,
      /// and TargetGlobalAddress.
      Wrapper
    };
  }

  class BPFSubtarget;
  class BPFTargetMachine;

  class BPFTargetLowering : public TargetLowering {
  public:
    explicit BPFTargetLowering(BPFTargetMachine &TM);

    /// LowerOperation - Provide custom lowering hooks for some operations.
    virtual SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const;

    /// getTargetNodeName - This method returns the name of a target specific
    /// DAG node.
    virtual const char *getTargetNodeName(unsigned Opcode) const;

    SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
    SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;

    MachineBasicBlock* EmitInstrWithCustomInserter(MachineInstr *MI,
                                                   MachineBasicBlock *BB) const;

  private:
    const BPFSubtarget &Subtarget;
    const BPFTargetMachine &TM;

    SDValue LowerCallResult(SDValue Chain, SDValue InFlag,
                            CallingConv::ID CallConv, bool isVarArg,
                            const SmallVectorImpl<ISD::InputArg> &Ins,
#if LLVM_VERSION_MINOR==4
                            SDLoc dl,
#else
                            DebugLoc dl,
#endif
                            SelectionDAG &DAG,
                            SmallVectorImpl<SDValue> &InVals) const;

    SDValue LowerCall(TargetLowering::CallLoweringInfo &CLI,
                      SmallVectorImpl<SDValue> &InVals) const;

    SDValue LowerFormalArguments(SDValue Chain,
                                 CallingConv::ID CallConv, bool isVarArg,
                                 const SmallVectorImpl<ISD::InputArg> &Ins,
#if LLVM_VERSION_MINOR==4
                                 SDLoc dl,
#else
                                 DebugLoc dl,
#endif
                                 SelectionDAG &DAG,
                                 SmallVectorImpl<SDValue> &InVals) const;

    SDValue LowerReturn(SDValue Chain,
                        CallingConv::ID CallConv, bool isVarArg,
                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                        const SmallVectorImpl<SDValue> &OutVals,
#if LLVM_VERSION_MINOR==4
                        SDLoc dl,
#else
                        DebugLoc dl,
#endif
                        SelectionDAG &DAG) const;
  };
}

#endif // LLVM_TARGET_BPF_ISELLOWERING_H
