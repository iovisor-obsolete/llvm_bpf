//===-- BPFISelLowering.cpp - BPF DAG Lowering Implementation  ----------===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This file implements the BPFTargetLowering class.

#define DEBUG_TYPE "bpf-lower"

#include "BPFISelLowering.h"
#include "BPF.h"
#include "BPFTargetMachine.h"
#include "BPFSubtarget.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

BPFTargetLowering::BPFTargetLowering(BPFTargetMachine &tm) :
  TargetLowering(tm, new TargetLoweringObjectFileELF()),
  Subtarget(*tm.getSubtargetImpl()), TM(tm) {

  // Set up the register classes.
  addRegisterClass(MVT::i64, &BPF::GPRRegClass);

  // Compute derived properties from the register classes
  computeRegisterProperties();

  setStackPointerRegisterToSaveRestore(BPF::R11);

  setOperationAction(ISD::BR_CC,             MVT::i64, Custom);
  setOperationAction(ISD::BR_JT,             MVT::Other, Expand);
  setOperationAction(ISD::BRCOND,            MVT::Other, Expand);
  setOperationAction(ISD::SETCC,             MVT::i64, Expand);
  setOperationAction(ISD::SELECT,            MVT::i64, Expand);
  setOperationAction(ISD::SELECT_CC,         MVT::i64, Custom);

//  setCondCodeAction(ISD::SETLT,             MVT::i64, Expand);

  setOperationAction(ISD::GlobalAddress,     MVT::i64, Custom);
  /*setOperationAction(ISD::BlockAddress,      MVT::i64, Custom);
  setOperationAction(ISD::JumpTable,         MVT::i64, Custom);
  setOperationAction(ISD::ConstantPool,      MVT::i64, Custom);*/

  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i64,   Custom);
  setOperationAction(ISD::STACKSAVE,          MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE,       MVT::Other, Expand);

/*  setOperationAction(ISD::VASTART,            MVT::Other, Custom);
  setOperationAction(ISD::VAARG,              MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,             MVT::Other, Expand);
  setOperationAction(ISD::VAEND,              MVT::Other, Expand);*/

//    setOperationAction(ISD::SDIV,            MVT::i64, Expand);
//  setOperationAction(ISD::UDIV,            MVT::i64, Expand);

  setOperationAction(ISD::SDIVREM,           MVT::i64, Expand);
  setOperationAction(ISD::UDIVREM,           MVT::i64, Expand);
  setOperationAction(ISD::SREM,              MVT::i64, Expand);
  setOperationAction(ISD::UREM,              MVT::i64, Expand);

//  setOperationAction(ISD::MUL,             MVT::i64, Expand);

  setOperationAction(ISD::MULHU,             MVT::i64, Expand);
  setOperationAction(ISD::MULHS,             MVT::i64, Expand);
  setOperationAction(ISD::UMUL_LOHI,         MVT::i64, Expand);
  setOperationAction(ISD::SMUL_LOHI,         MVT::i64, Expand);

  setOperationAction(ISD::ADDC, MVT::i64, Expand);
  setOperationAction(ISD::ADDE, MVT::i64, Expand);
  setOperationAction(ISD::SUBC, MVT::i64, Expand);
  setOperationAction(ISD::SUBE, MVT::i64, Expand);

  setOperationAction(ISD::ROTR,              MVT::i64, Expand);
  setOperationAction(ISD::ROTL,              MVT::i64, Expand);
  setOperationAction(ISD::SHL_PARTS,         MVT::i64, Expand);
  setOperationAction(ISD::SRL_PARTS,         MVT::i64, Expand);
  setOperationAction(ISD::SRA_PARTS,         MVT::i64, Expand);

  setOperationAction(ISD::BSWAP,             MVT::i64, Expand);
  setOperationAction(ISD::CTTZ,              MVT::i64, Custom);
  setOperationAction(ISD::CTLZ,              MVT::i64, Custom);
  setOperationAction(ISD::CTTZ_ZERO_UNDEF,   MVT::i64, Custom);
  setOperationAction(ISD::CTLZ_ZERO_UNDEF,   MVT::i64, Custom);
  setOperationAction(ISD::CTPOP,             MVT::i64, Expand);


  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1,   Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8,   Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16,  Expand);
  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32,  Expand);

  // Extended load operations for i1 types must be promoted
  setLoadExtAction(ISD::EXTLOAD,             MVT::i1,   Promote);
  setLoadExtAction(ISD::ZEXTLOAD,            MVT::i1,   Promote);
  setLoadExtAction(ISD::SEXTLOAD,            MVT::i1,   Promote);

  setLoadExtAction(ISD::SEXTLOAD,            MVT::i8,   Expand);
  setLoadExtAction(ISD::SEXTLOAD,            MVT::i16,   Expand);
  setLoadExtAction(ISD::SEXTLOAD,            MVT::i32,   Expand);

  // Function alignments (log2)
  setMinFunctionAlignment(3);
  setPrefFunctionAlignment(3);

#if LLVM_VERSION_MINOR==3 || LLVM_VERSION_MINOR==4
  MaxStoresPerMemcpy = 128;
  MaxStoresPerMemcpyOptSize = 128;
  MaxStoresPerMemset = 128;
#else
  maxStoresPerMemcpy = 128;
  maxStoresPerMemcpyOptSize = 128;
  maxStoresPerMemset = 128;
#endif
}

SDValue BPFTargetLowering::LowerOperation(SDValue Op,
                                          SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::BR_CC:              return LowerBR_CC(Op, DAG);
  case ISD::GlobalAddress:      return LowerGlobalAddress(Op, DAG);
  case ISD::SELECT_CC:          return LowerSELECT_CC(Op, DAG);
  default:
    llvm_unreachable("unimplemented operand");
  }
}

//                      Calling Convention Implementation
#include "BPFGenCallingConv.inc"

SDValue
BPFTargetLowering::LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                                        bool isVarArg,
                                        const SmallVectorImpl<ISD::InputArg>
                                        &Ins,
#if LLVM_VERSION_MINOR==4
                                        SDLoc dl,
#else
                                        DebugLoc dl,
#endif
                                        SelectionDAG &DAG,
                                        SmallVectorImpl<SDValue> &InVals)
                                          const {
  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    break;
  }

/// LowerCCCArguments - transform physical registers into virtual registers and
/// generate load operations for arguments places on the stack.
  MachineFunction &MF = DAG.getMachineFunction();
  MachineRegisterInfo &RegInfo = MF.getRegInfo();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, CC_BPF64);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      // Arguments passed in registers
      EVT RegVT = VA.getLocVT();
      switch (RegVT.getSimpleVT().SimpleTy) {
      default:
        {
#ifndef NDEBUG
          errs() << "LowerFormalArguments Unhandled argument type: "
               << RegVT.getSimpleVT().SimpleTy << "\n";
#endif
          llvm_unreachable(0);
        }
      case MVT::i64:
        unsigned VReg = RegInfo.createVirtualRegister(&BPF::GPRRegClass);
        RegInfo.addLiveIn(VA.getLocReg(), VReg);
        SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, VReg, RegVT);

        // If this is an 8/16/32-bit value, it is really passed promoted to 64
        // bits. Insert an assert[sz]ext to capture this, then truncate to the
        // right size.
        if (VA.getLocInfo() == CCValAssign::SExt)
          ArgValue = DAG.getNode(ISD::AssertSext, dl, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));
        else if (VA.getLocInfo() == CCValAssign::ZExt)
          ArgValue = DAG.getNode(ISD::AssertZext, dl, RegVT, ArgValue,
                                 DAG.getValueType(VA.getValVT()));

        if (VA.getLocInfo() != CCValAssign::Full)
          ArgValue = DAG.getNode(ISD::TRUNCATE, dl, VA.getValVT(), ArgValue);

        InVals.push_back(ArgValue);
      }
    } else {
      assert(VA.isMemLoc());
      errs() << "Function: " << MF.getName() << " ";
      MF.getFunction()->getFunctionType()->dump();
      errs() << "\n";
      report_fatal_error("too many function args");
    }
  }

  if (isVarArg || MF.getFunction()->hasStructRetAttr()) {
    errs() << "Function: " << MF.getName() << " ";
    MF.getFunction()->getFunctionType()->dump();
    errs() << "\n";
    report_fatal_error("functions with VarArgs or StructRet are not supported");
  }

  return Chain;
}

SDValue
BPFTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                              SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG                     = CLI.DAG;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals     = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;

  // BPF target does not support tail call optimization.
  isTailCall = false;

  switch (CallConv) {
  default:
    report_fatal_error("Unsupported calling convention");
  case CallingConv::Fast:
  case CallingConv::C:
    break;
  }

/// LowerCCCCallTo - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeCallOperands(Outs, CC_BPF64);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  // Create local copies for byval args.
  SmallVector<SDValue, 8> ByValArgs;

  if (Outs.size() >= 6) {
    errs() << "too many arguments to a function ";
    Callee.dump();
    report_fatal_error("too many args\n");
  }

  for (unsigned i = 0,  e = Outs.size(); i != e; ++i) {
    ISD::ArgFlagsTy Flags = Outs[i].Flags;
    if (!Flags.isByVal())
      continue;

    Callee.dump();
    report_fatal_error("cannot pass by value");
  }

  Chain = DAG.getCALLSEQ_START(Chain, DAG.getConstant(NumBytes,
                                                      getPointerTy(), true)
#if LLVM_VERSION_MINOR==4
                                                      , CLI.DL
#endif
                                                      );

  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SDValue StackPtr;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, j = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    SDValue Arg = OutVals[i];
    ISD::ArgFlagsTy Flags = Outs[i].Flags;


    // Promote the value if needed.
    switch (VA.getLocInfo()) {
      default: llvm_unreachable("Unknown loc info!");
      case CCValAssign::Full: break;
      case CCValAssign::SExt:
        Arg = DAG.getNode(ISD::SIGN_EXTEND, CLI.DL, VA.getLocVT(), Arg);
        break;
      case CCValAssign::ZExt:
        Arg = DAG.getNode(ISD::ZERO_EXTEND, CLI.DL, VA.getLocVT(), Arg);
        break;
      case CCValAssign::AExt:
        Arg = DAG.getNode(ISD::ANY_EXTEND, CLI.DL, VA.getLocVT(), Arg);
        break;
    }

    // Use local copy if it is a byval arg.
    if (Flags.isByVal())
      Arg = ByValArgs[j++];

    // Arguments that can be passed on register must be kept at RegsToPass
    // vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      llvm_unreachable("call arg pass bug");
    }
  }

  SDValue InFlag;

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, CLI.DL, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), CLI.DL, getPointerTy(), G->getOffset()/*0*/,
                                        0);
  } else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), getPointerTy(),
                                         0);
  }

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  Chain = DAG.getNode(BPFISD::CALL, CLI.DL, NodeTys, &Ops[0], Ops.size());
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain,
                             DAG.getConstant(NumBytes, getPointerTy(), true),
                             DAG.getConstant(0, getPointerTy(), true),
                             InFlag
#if LLVM_VERSION_MINOR==4
                             , CLI.DL
#endif
                             );
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, CLI.DL,
                         DAG, InVals);
}

SDValue
BPFTargetLowering::LowerReturn(SDValue Chain,
                               CallingConv::ID CallConv, bool isVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
#if LLVM_VERSION_MINOR==4
                               SDLoc dl,
#else
                               DebugLoc dl,
#endif
                               SelectionDAG &DAG) const {

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), RVLocs, *DAG.getContext());

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_BPF64);

  // If this is the first return lowered for this function, add the regs to the
  // liveout set for the function.
#if LLVM_VERSION_MINOR==2
  if (DAG.getMachineFunction().getRegInfo().liveout_empty()) {
    for (unsigned i = 0; i != RVLocs.size(); ++i)
      if (RVLocs[i].isRegLoc())
        DAG.getMachineFunction().getRegInfo().addLiveOut(RVLocs[i].getLocReg());
  }
#endif

  SDValue Flag;
#if LLVM_VERSION_MINOR==3 || LLVM_VERSION_MINOR==4
  SmallVector<SDValue, 4> RetOps(1, Chain);
#endif

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(),
                             OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
#if LLVM_VERSION_MINOR==3 || LLVM_VERSION_MINOR==4
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
#endif
  }

  if (DAG.getMachineFunction().getFunction()->hasStructRetAttr()) {
    errs() << "Function: " << DAG.getMachineFunction().getName() << " ";
    DAG.getMachineFunction().getFunction()->getFunctionType()->dump();
    errs() << "\n";
    report_fatal_error("BPF doesn't support struct return");
  }

  unsigned Opc = BPFISD::RET_FLAG;
#if LLVM_VERSION_MINOR==3 || LLVM_VERSION_MINOR==4
  RetOps[0] = Chain;  // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(Opc, dl, MVT::Other, &RetOps[0], RetOps.size());
#else
  if (Flag.getNode())
    return DAG.getNode(Opc, dl, MVT::Other, Chain, Flag);

  // Return Void
  return DAG.getNode(Opc, dl, MVT::Other, Chain);
#endif
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue
BPFTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                   CallingConv::ID CallConv, bool isVarArg,
                                   const SmallVectorImpl<ISD::InputArg> &Ins,
#if LLVM_VERSION_MINOR==4
                                   SDLoc dl,
#else
                                   DebugLoc dl,
#endif
                                   SelectionDAG &DAG,
                                   SmallVectorImpl<SDValue> &InVals) const {

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 getTargetMachine(), RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_BPF64);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    Chain = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(),
                               RVLocs[i].getValVT(), InFlag).getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

static bool NegateCC(SDValue &LHS, SDValue &RHS, ISD::CondCode &CC)
{
  switch (CC) {
  default:
    return false;
  case ISD::SETULT:
    CC = ISD::SETUGT;
    std::swap(LHS, RHS);
    return true;
  case ISD::SETULE:
    CC = ISD::SETUGE;
    std::swap(LHS, RHS);
    return true;
  case ISD::SETLT:
    CC = ISD::SETGT;
    std::swap(LHS, RHS);
    return true;
  case ISD::SETLE:
    CC = ISD::SETGE;
    std::swap(LHS, RHS);
    return true;
  }
}

SDValue BPFTargetLowering::LowerBR_CC(SDValue Op,
                                      SelectionDAG &DAG) const {
  SDValue Chain  = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS   = Op.getOperand(2);
  SDValue RHS   = Op.getOperand(3);
  SDValue Dest  = Op.getOperand(4);
#if LLVM_VERSION_MINOR==4
  SDLoc    dl(Op);
#else
  DebugLoc dl   = Op.getDebugLoc();
#endif

  NegateCC(LHS, RHS, CC);

  return DAG.getNode(BPFISD::BR_CC, dl, Op.getValueType(),
                     Chain, LHS, RHS, DAG.getConstant(CC, MVT::i64), Dest);
}

SDValue BPFTargetLowering::LowerSELECT_CC(SDValue Op,
                                          SelectionDAG &DAG) const {
  SDValue LHS    = Op.getOperand(0);
  SDValue RHS    = Op.getOperand(1);
  SDValue TrueV  = Op.getOperand(2);
  SDValue FalseV = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
#if LLVM_VERSION_MINOR==4
  SDLoc    dl(Op);
#else
  DebugLoc dl    = Op.getDebugLoc();
#endif

  NegateCC(LHS, RHS, CC);

  SDValue TargetCC = DAG.getConstant(CC, MVT::i64);

  SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
  SmallVector<SDValue, 5> Ops;
  Ops.push_back(LHS);
  Ops.push_back(RHS);
  Ops.push_back(TargetCC);
  Ops.push_back(TrueV);
  Ops.push_back(FalseV);

  SDValue sel = DAG.getNode(BPFISD::SELECT_CC, dl, VTs, &Ops[0], Ops.size());
  DEBUG(errs() << "LowerSELECT_CC:\n"; sel.dumpr(); errs() << "\n");
  return sel;
}

const char *BPFTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  default: return NULL;
  case BPFISD::ADJDYNALLOC:        return "BPFISD::ADJDYNALLOC";
  case BPFISD::RET_FLAG:           return "BPFISD::RET_FLAG";
  case BPFISD::CALL:               return "BPFISD::CALL";
  case BPFISD::SELECT_CC:          return "BPFISD::SELECT_CC";
  case BPFISD::BR_CC:              return "BPFISD::BR_CC";
  case BPFISD::Wrapper:            return "BPFISD::Wrapper";
  }
}

SDValue BPFTargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
//  Op.dump();
#if LLVM_VERSION_MINOR==4
  SDLoc    dl(Op);
#else
  DebugLoc dl = Op.getDebugLoc();
#endif
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  SDValue GA = DAG.getTargetGlobalAddress(GV, dl, MVT::i64);

  return DAG.getNode(BPFISD::Wrapper, dl, MVT::i64, GA);
}

MachineBasicBlock*
BPFTargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
                                               MachineBasicBlock *BB) const {
  unsigned Opc = MI->getOpcode();

  const TargetInstrInfo &TII = *getTargetMachine().getInstrInfo();
  DebugLoc dl = MI->getDebugLoc();

  assert(Opc == BPF::Select && "Unexpected instr type to insert");

  // To "insert" a SELECT instruction, we actually have to insert the diamond
  // control-flow pattern.  The incoming instruction knows the destination vreg
  // to set, the condition code register to branch on, the true/false values to
  // select between, and a branch opcode to use.
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator I = BB;
  ++I;

  //  thisMBB:
  //  ...
  //   TrueVal = ...
  //   jmp_XX r1, r2 goto copy1MBB
  //   fallthrough --> copy0MBB
  MachineBasicBlock *thisMBB = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *copy1MBB = F->CreateMachineBasicBlock(LLVM_BB);

  F->insert(I, copy0MBB);
  F->insert(I, copy1MBB);
  // Update machine-CFG edges by transferring all successors of the current
  // block to the new block which will contain the Phi node for the select.
  copy1MBB->splice(copy1MBB->begin(), BB,
                   llvm::next(MachineBasicBlock::iterator(MI)),
                   BB->end());
  copy1MBB->transferSuccessorsAndUpdatePHIs(BB);
  // Next, add the true and fallthrough blocks as its successors.
  BB->addSuccessor(copy0MBB);
  BB->addSuccessor(copy1MBB);

  // Insert Branch if Flag
  unsigned LHS = MI->getOperand(1).getReg();
  unsigned RHS = MI->getOperand(2).getReg();
  int CC  = MI->getOperand(3).getImm();
  switch (CC) {
  case ISD::SETGT:
    BuildMI(BB, dl, TII.get(BPF::JSGT_rr))
      .addReg(LHS).addReg(RHS).addMBB(copy1MBB);
    break;
  case ISD::SETUGT:
    BuildMI(BB, dl, TII.get(BPF::JUGT_rr))
      .addReg(LHS).addReg(RHS).addMBB(copy1MBB);
    break;
  case ISD::SETGE:
    BuildMI(BB, dl, TII.get(BPF::JSGE_rr))
      .addReg(LHS).addReg(RHS).addMBB(copy1MBB);
    break;
  case ISD::SETUGE:
    BuildMI(BB, dl, TII.get(BPF::JUGE_rr))
      .addReg(LHS).addReg(RHS).addMBB(copy1MBB);
    break;
  case ISD::SETEQ:
    BuildMI(BB, dl, TII.get(BPF::JEQ_rr))
      .addReg(LHS).addReg(RHS).addMBB(copy1MBB);
    break;
  case ISD::SETNE:
    BuildMI(BB, dl, TII.get(BPF::JNE_rr))
      .addReg(LHS).addReg(RHS).addMBB(copy1MBB);
    break;
  default:
    report_fatal_error("unimplemented select CondCode " + Twine(CC));
  }

  //  copy0MBB:
  //   %FalseValue = ...
  //   # fallthrough to copy1MBB
  BB = copy0MBB;

  // Update machine-CFG edges
  BB->addSuccessor(copy1MBB);

  //  copy1MBB:
  //   %Result = phi [ %FalseValue, copy0MBB ], [ %TrueValue, thisMBB ]
  //  ...
  BB = copy1MBB;
  BuildMI(*BB, BB->begin(), dl, TII.get(BPF::PHI),
          MI->getOperand(0).getReg())
    .addReg(MI->getOperand(5).getReg()).addMBB(copy0MBB)
    .addReg(MI->getOperand(4).getReg()).addMBB(thisMBB);

  MI->eraseFromParent();   // The pseudo instruction is gone now.
  return BB;
}

