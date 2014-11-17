//===-- BPFInstrInfo.td - Target Description for BPF Target -------------===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This file describes the BPF instructions in TableGen format.

include "BPFInstrFormats.td"

// Instruction Operands and Patterns

//  These are target-independent nodes, but have target-specific formats.
def SDT_BPFCallSeqStart : SDCallSeqStart<[ SDTCisVT<0, iPTR> ]>;
def SDT_BPFCallSeqEnd   : SDCallSeqEnd<[ SDTCisVT<0, iPTR>,
                                          SDTCisVT<1, iPTR> ]>;
def SDT_BPFCall         : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_BPFSetFlag      : SDTypeProfile<0, 3, [SDTCisSameAs<0, 1>]>;
def SDT_BPFSelectCC     : SDTypeProfile<1, 5, [SDTCisSameAs<1, 2>, SDTCisSameAs<0, 4>,
                                                SDTCisSameAs<4, 5>]>;
def SDT_BPFBrCC         : SDTypeProfile<0, 4, [SDTCisSameAs<0, 1>, SDTCisVT<3, OtherVT>]>;

def SDT_BPFWrapper      : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>,
                                                SDTCisPtrTy<0>]>;
//def SDT_BPFAdjDynAlloc  : SDTypeProfile<1, 1, [SDTCisVT<0, i64>,
//                                                SDTCisVT<1, i64>]>;

def call            : SDNode<"BPFISD::CALL", SDT_BPFCall,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue,
                              SDNPVariadic]>;
def retflag         : SDNode<"BPFISD::RET_FLAG", SDTNone,
                             [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def callseq_start   : SDNode<"ISD::CALLSEQ_START", SDT_BPFCallSeqStart,
                             [SDNPHasChain, SDNPOutGlue]>;
def callseq_end     : SDNode<"ISD::CALLSEQ_END",   SDT_BPFCallSeqEnd,
                             [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
def BPFbrcc        : SDNode<"BPFISD::BR_CC", SDT_BPFBrCC,
                              [SDNPHasChain, SDNPOutGlue, SDNPInGlue]>;

def BPFselectcc    : SDNode<"BPFISD::SELECT_CC", SDT_BPFSelectCC, [SDNPInGlue]>;
def BPFWrapper     : SDNode<"BPFISD::Wrapper", SDT_BPFWrapper>;

//def BPFadjdynalloc : SDNode<"BPFISD::ADJDYNALLOC", SDT_BPFAdjDynAlloc>;

// helper macros to produce 64-bit constant
// 0x11223344 55667788 ->
// reg = 0x11223344
// reg <<= 32
// reg += 0x55667788
//
// 0x11223344 FF667788 ->
// reg = 0x11223345
// reg <<= 32
// reg += (long long)(int)0xFF667788
def LO32 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant((int64_t)(int32_t)(uint64_t)N->getZExtValue(),
                                   MVT::i64);
}]>;
def HI32 : SDNodeXForm<imm, [{
  return CurDAG->getTargetConstant(((int64_t)N->getZExtValue() -
         (int64_t)(int32_t)(uint64_t)N->getZExtValue()) >> 32, MVT::i64);
}]>;


def brtarget : Operand<OtherVT>;
def calltarget : Operand<i64>;

def s32imm   : Operand<i64> {
}

def u64imm   : Operand<i64> {
  let PrintMethod = "printImm64Operand";
}

def immSExt32 : PatLeaf<(imm),
                [{return isInt<32>(N->getSExtValue()); }]>;

// Addressing modes.
def ADDRri : ComplexPattern<i64, 2, "SelectAddr", [frameindex], []>;

// Address operands
def MEMri : Operand<i64> {
  let PrintMethod = "printMemOperand";
  let EncoderMethod = "getMemoryOpValue";
  let DecoderMethod = "todo_decode_memri";
  let MIOperandInfo = (ops GPR, i16imm);
}

// Conditional code predicates - used for pattern matching for SF instructions
def BPF_CC_EQ  : PatLeaf<(imm),
                  [{return (N->getZExtValue() == ISD::SETEQ);}]>;
def BPF_CC_NE  : PatLeaf<(imm),
                  [{return (N->getZExtValue() == ISD::SETNE);}]>;
def BPF_CC_GE  : PatLeaf<(imm),
                  [{return (N->getZExtValue() == ISD::SETGE);}]>;
def BPF_CC_GT  : PatLeaf<(imm),
                  [{return (N->getZExtValue() == ISD::SETGT);}]>;
def BPF_CC_GTU : PatLeaf<(imm),
                  [{return (N->getZExtValue() == ISD::SETUGT);}]>;
def BPF_CC_GEU : PatLeaf<(imm),
                  [{return (N->getZExtValue() == ISD::SETUGE);}]>;

// jump instructions
class JMP_RR<bits<4> br_op, string asmstr, PatLeaf Cond>
  : InstBPF<(outs), (ins GPR:$rA, GPR:$rX, brtarget:$dst),
           !strconcat(asmstr, "\t$rA, $rX goto $dst"),
           [(BPFbrcc (i64 GPR:$rA), (i64 GPR:$rX), Cond, bb:$dst)]> {
  bits<4> op;
  bits<1> src;
  bits<4> rA;
  bits<4> rX;
  bits<16> dst;

  let Inst{63-60} = op;
  let Inst{59} = src;
  let Inst{55-52} = rX;
  let Inst{51-48} = rA;
  let Inst{47-32} = dst;

  let op = br_op;
  let src = 1;
  let bpfClass = 5; // BPF_JMP
}

class JMP_RI<bits<4> br_op, string asmstr, PatLeaf Cond>
  : InstBPF<(outs), (ins GPR:$rA, s32imm:$imm, brtarget:$dst),
           !strconcat(asmstr, "i\t$rA, $imm goto $dst"),
           [(BPFbrcc (i64 GPR:$rA), immSExt32:$imm, Cond, bb:$dst)]> {
  bits<4> op;
  bits<1> src;
  bits<4> rA;
  bits<16> dst;
  bits<32> imm;

  let Inst{63-60} = op;
  let Inst{59} = src;
  let Inst{51-48} = rA;
  let Inst{47-32} = dst;
  let Inst{31-0} = imm;

  let op = br_op;
  let src = 0;
  let bpfClass = 5; // BPF_JMP
}

multiclass J<bits<4> op2Val, string asmstr, PatLeaf Cond> {
  def _rr : JMP_RR<op2Val, asmstr, Cond>;
  def _ri : JMP_RI<op2Val, asmstr, Cond>;
}

let isBranch = 1, isTerminator = 1, hasDelaySlot=0 in {
// cmp+goto instructions
defm JEQ  : J<0x1, "jeq",  BPF_CC_EQ>;
defm JUGT : J<0x2, "jgt", BPF_CC_GTU>;
defm JUGE : J<0x3, "jge", BPF_CC_GEU>;
defm JNE  : J<0x5, "jne",  BPF_CC_NE>;
defm JSGT : J<0x6, "jsgt", BPF_CC_GT>;
defm JSGE : J<0x7, "jsge", BPF_CC_GE>;
}

// ALU instructions
class ALU_RI<bits<4> aluOp, string asmstr, SDNode OpNode>
  : InstBPF<(outs GPR:$rA), (ins GPR:$rS, s32imm:$imm),
            !strconcat(asmstr, "i\t$rA, $imm"),
            [(set GPR:$rA, (OpNode GPR:$rS, immSExt32:$imm))]> {
  bits<4> op;
  bits<1> src;
  bits<4> rA;
  bits<32> imm;

  let Inst{63-60} = op;
  let Inst{59} = src;
  let Inst{51-48} = rA;
  let Inst{31-0} = imm;

  let op = aluOp;
  let src = 0;
  let bpfClass = 7; // BPF_ALU64
}

class ALU_RR<bits<4> aluOp, string asmstr, SDNode OpNode>
  : InstBPF<(outs GPR:$rA), (ins GPR:$rS, GPR:$rX),
            !strconcat(asmstr, "\t$rA, $rX"),
            [(set GPR:$rA, (OpNode (i64 GPR:$rS), (i64 GPR:$rX)))]> {
  bits<4> op;
  bits<1> src;
  bits<4> rA;
  bits<4> rX;

  let Inst{63-60} = op;
  let Inst{59} = src;
  let Inst{55-52} = rX;
  let Inst{51-48} = rA;

  let op = aluOp;
  let src = 1;
  let bpfClass = 7; // BPF_ALU64
}

multiclass ALU<bits<4> opVal, string asmstr, SDNode OpNode> {
  def _rr : ALU_RR<opVal, asmstr, OpNode>;
  def _ri : ALU_RI<opVal, asmstr, OpNode>;
}

let Constraints = "$rA = $rS" in {
let isAsCheapAsAMove = 1 in {
  defm ADD : ALU<0x0, "add", add>;
  defm SUB : ALU<0x1, "sub", sub>;
  defm OR  : ALU<0x4, "or", or>;
  defm AND : ALU<0x5, "and", and>;
  defm SLL : ALU<0x6, "sll", shl>;
  defm SRL : ALU<0x7, "srl", srl>;
  defm XOR : ALU<0xa, "xor", xor>;
  defm SRA : ALU<0xc, "sra", sra>;
}
  defm MUL : ALU<0x2, "mul", mul>;
  defm DIV : ALU<0x3, "div", udiv>;
}

class MOV_RR<string asmstr>
  : InstBPF<(outs GPR:$rA), (ins GPR:$rX),
            !strconcat(asmstr, "\t$rA, $rX"),
            []> {
  bits<4> op;
  bits<1> src;
  bits<4> rA;
  bits<4> rX;

  let Inst{63-60} = op;
  let Inst{59} = src;
  let Inst{55-52} = rX;
  let Inst{51-48} = rA;

  let op = 0xb;     // BPF_MOV
  let src = 1;      // BPF_X
  let bpfClass = 7; // BPF_ALU64
}

class MOV_RI<string asmstr>
  : InstBPF<(outs GPR:$rA), (ins s32imm:$imm),
            !strconcat(asmstr, "\t$rA, $imm"),
            [(set GPR:$rA, (i64 immSExt32:$imm))]> {
  bits<4> op;
  bits<1> src;
  bits<4> rA;
  bits<32> imm;

  let Inst{63-60} = op;
  let Inst{59} = src;
  let Inst{51-48} = rA;
  let Inst{31-0} = imm;

  let op = 0xb;     // BPF_MOV
  let src = 0;      // BPF_K
  let bpfClass = 7; // BPF_ALU64
}
def MOV_rr : MOV_RR<"mov">;
def MOV_ri : MOV_RI<"mov">;

class LD_IMM64<bits<4> pseudo, string asmstr>
  : InstBPF<(outs GPR:$rA), (ins u64imm:$imm),
            !strconcat(asmstr, "\t$rA, $imm"),
            [(set GPR:$rA, (i64 imm:$imm))]> {

  bits<3> mode;
  bits<2> size;
  bits<4> rA;
  bits<64> imm;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{51-48} = rA;
  let Inst{55-52} = pseudo;
  let Inst{47-32} = 0;
  let Inst{31-0} = imm{31-0};

  let mode = 0;     // BPF_IMM
  let size = 3;     // BPF_DW
  let bpfClass = 0; // BPF_LD
}
def LD_imm64 : LD_IMM64<0, "ld_64">;

// STORE instructions
class STORE<bits<2> sizeOp, string asmstring, list<dag> pattern>
  : InstBPF<(outs), (ins GPR:$rX, MEMri:$addr),
          !strconcat(asmstring, "\t$addr, $rX"), pattern> {
  bits<3> mode;
  bits<2> size;
  bits<4> rX;
  bits<20> addr;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{51-48} = addr{19-16}; // base reg
  let Inst{55-52} = rX;
  let Inst{47-32} = addr{15-0}; // offset

  let mode = 3;     // BPF_MEM
  let size = sizeOp;
  let bpfClass = 3; // BPF_STX
}

class STOREi64<bits<2> subOp, string asmstring, PatFrag opNode>
  : STORE<subOp, asmstring, [(opNode (i64 GPR:$rX), ADDRri:$addr)]>;

def STW : STOREi64<0x0, "stw", truncstorei32>;
def STH : STOREi64<0x1, "sth", truncstorei16>;
def STB : STOREi64<0x2, "stb", truncstorei8>;
def STD : STOREi64<0x3, "std", store>;

// LOAD instructions
class LOAD<bits<2> sizeOp, string asmstring, list<dag> pattern>
  : InstBPF<(outs GPR:$rA), (ins MEMri:$addr),
           !strconcat(asmstring, "\t$rA, $addr"), pattern> {
  bits<3> mode;
  bits<2> size;
  bits<4> rA;
  bits<20> addr;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{51-48} = rA;
  let Inst{55-52} = addr{19-16};
  let Inst{47-32} = addr{15-0};

  let mode = 3;     // BPF_MEM
  let size = sizeOp;
  let bpfClass = 1; // BPF_LDX
}

class LOADi64<bits<2> sizeOp, string asmstring, PatFrag opNode>
  : LOAD<sizeOp, asmstring, [(set (i64 GPR:$rA), (opNode ADDRri:$addr))]>;

def LDW : LOADi64<0x0, "ldw", zextloadi32>;
def LDH : LOADi64<0x1, "ldh", zextloadi16>;
def LDB : LOADi64<0x2, "ldb", zextloadi8>;
def LDD : LOADi64<0x3, "ldd", load>;

//def LDBS : LOADi64<0x2, "ldbs", sextloadi8>;
//def LDHS : LOADi64<0x1, "ldhs", sextloadi16>;
//def LDWS : LOADi64<0x0, "ldws", sextloadi32>;

class BRANCH<bits<4> subOp, string asmstring, list<dag> pattern>
  : InstBPF<(outs), (ins brtarget:$dst),
           !strconcat(asmstring, "\t$dst"), pattern> {
  bits<4> op;
  bits<16> dst;
  bits<1> src;

  let Inst{63-60} = op;
  let Inst{59} = src;
  let Inst{47-32} = dst;

  let op = subOp;
  let src = 0;
  let bpfClass = 5; // BPF_JMP
}

class CALL<string asmstring>
  : InstBPF<(outs), (ins calltarget:$dst),
           !strconcat(asmstring, "\t$dst"), []> {
  bits<4> op;
  bits<32> dst;
  bits<1> src;

  let Inst{63-60} = op;
  let Inst{59} = src;
  let Inst{31-0} = dst;

  let op = 8;       // BPF_CALL
  let src = 0;
  let bpfClass = 5; // BPF_JMP
}

// Jump always
let isBranch = 1, isTerminator = 1, hasDelaySlot=0, isBarrier = 1 in {
  def JMP : BRANCH<0x0, "jmp", [(br bb:$dst)]>;
}

// Jump and link
let isCall=1, hasDelaySlot=0,
    Uses = [R11],
    // Potentially clobbered registers
    Defs = [R0, R1, R2, R3, R4, R5] in {
  def JAL  : CALL<"call">;
}

class NOP_I<string asmstr>
  : InstBPF<(outs), (ins i32imm:$imm),
           !strconcat(asmstr, "\t$imm"), []> {
  // mov r0, r0 == nop
  bits<4> op;
  bits<1> src;
  bits<4> rA;
  bits<4> rX;

  let Inst{63-60} = op;
  let Inst{59} = src;
  let Inst{55-52} = rX;
  let Inst{51-48} = rA;

  let op = 0xb;     // BPF_MOV
  let src = 1;      // BPF_X
  let bpfClass = 7; // BPF_ALU64
  let rX = 0;       // R0
  let rA = 0;       // R0
}

let neverHasSideEffects = 1 in
  def NOP : NOP_I<"nop">;

class RET<string asmstring>
  : InstBPF<(outs), (ins),
           !strconcat(asmstring, ""), [(retflag)]> {
  bits<4> op;

  let Inst{63-60} = op;
  let Inst{59} = 0;
  let Inst{31-0} = 0;

  let op = 9;       // BPF_EXIT
  let bpfClass = 5; // BPF_JMP
}

let isReturn = 1, isTerminator = 1, hasDelaySlot=0, isBarrier = 1, isNotDuplicable = 1 in {
  def RET : RET<"ret">;
}

// ADJCALLSTACKDOWN/UP pseudo insns
let Defs = [R11], Uses = [R11] in {
def ADJCALLSTACKDOWN : Pseudo<(outs), (ins i64imm:$amt),
                              "#ADJCALLSTACKDOWN $amt",
                              [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : Pseudo<(outs), (ins i64imm:$amt1, i64imm:$amt2),
                              "#ADJCALLSTACKUP $amt1 $amt2",
                              [(callseq_end timm:$amt1, timm:$amt2)]>;
}


//let Defs = [R11], Uses = [R11] in {
//  def ADJDYNALLOC : Pseudo<(outs GPR:$dst), (ins GPR:$src),
//                    "#ADJDYNALLOC $dst $src",
//                    [(set GPR:$dst, (BPFadjdynalloc GPR:$src))]>;
//}


let usesCustomInserter = 1 in {
  def Select : Pseudo<(outs GPR:$dst), (ins GPR:$lhs, GPR:$rhs, s32imm:$imm, GPR:$src, GPR:$src2),
                       "# Select PSEUDO $dst = $lhs $imm $rhs ? $src : $src2",
                       [(set (i64 GPR:$dst),
                        (BPFselectcc (i64 GPR:$lhs), (i64 GPR:$rhs), (i64 imm:$imm), (i64 GPR:$src), (i64 GPR:$src2)))]>;
}

// load 64-bit global addr into register
def : Pat<(BPFWrapper tglobaladdr:$in), (LD_imm64 tglobaladdr:$in)>;

// arbitrary immediate
//def : Pat<(i64 imm:$imm), (ADD_ri (SLL_ri (MOV_ri (HI32 imm:$imm)), 32), (LO32 imm:$imm))>;

// 0xffffFFFF doesn't fit into simm32, optimize common case
def : Pat<(i64 (and (i64 GPR:$src), 0xffffFFFF)), (SRL_ri (SLL_ri (i64 GPR:$src), 32), 32)>;

// Calls
def : Pat<(call tglobaladdr:$dst), (JAL tglobaladdr:$dst)>;
//def : Pat<(call texternalsym:$dst), (JAL texternalsym:$dst)>;
//def : Pat<(call (i32 imm:$dst)), (JAL (i32 imm:$dst))>;
def : Pat<(call imm:$dst), (JAL imm:$dst)>;

// Loads
def : Pat<(extloadi8  ADDRri:$src), (i64 (LDB ADDRri:$src))>;
def : Pat<(extloadi16 ADDRri:$src), (i64 (LDH ADDRri:$src))>;
def : Pat<(extloadi32 ADDRri:$src), (i64 (LDW ADDRri:$src))>;

// Atomics
class XADD<bits<2> sizeOp, string asmstr, PatFrag opNode>
  : InstBPF<(outs GPR:$dst), (ins MEMri:$addr, GPR:$val),
            !strconcat(asmstr, "\t$dst, $addr, $val"),
            [(set GPR:$dst, (opNode ADDRri:$addr, GPR:$val))]> {
  bits<3> mode;
  bits<2> size;
  bits<4> rX;
  bits<20> addr;

  let Inst{63-61} = mode;
  let Inst{60-59} = size;
  let Inst{51-48} = addr{19-16}; // base reg
  let Inst{55-52} = rX;
  let Inst{47-32} = addr{15-0}; // offset

  let mode = 6;     // BPF_XADD
  let size = sizeOp;
  let bpfClass = 3; // BPF_STX
}

let Constraints = "$dst = $val" in {
def XADD32 : XADD<0, "xadd32", atomic_load_add_32>;
def XADD16 : XADD<1, "xadd16", atomic_load_add_16>;
def XADD8  : XADD<2, "xadd8", atomic_load_add_8>;
def XADD64 : XADD<3, "xadd64", atomic_load_add_64>;
}
