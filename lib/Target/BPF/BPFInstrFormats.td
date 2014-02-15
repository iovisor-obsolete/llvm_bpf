//===- BPFInstrFormats.td - BPF Instruction Formats ----*- tablegen -*-===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

class InstBPF<dag outs, dag ins, string asmstr, list<dag> pattern>
  : Instruction {
  field bits<64> Inst;
  field bits<64> SoftFail = 0;
  let Size = 8;

  let Namespace = "BPF";
  let DecoderNamespace = "BPF";

  bits<3> bpfClass;
  let Inst{58-56} = bpfClass;

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString = asmstr;
  let Pattern = pattern;
}

// Pseudo instructions
class Pseudo<dag outs, dag ins, string asmstr, list<dag> pattern>
  : InstBPF<outs, ins, asmstr, pattern> {
  let Inst{63-0} = 0;
  let isPseudo = 1;
}

