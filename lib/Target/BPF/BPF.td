//===- BPF.td - Describe the BPF Target Machine --------*- tablegen -*-===//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

// Target-independent interfaces which we are implementing
include "llvm/Target/Target.td"

// BPF Subtarget features.
include "BPFRegisterInfo.td"
include "BPFCallingConv.td"
include "BPFInstrInfo.td"

def BPFInstrInfo : InstrInfo;

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def BPFInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

// Declare the target which we are implementing
def BPF : Target {
  let InstructionSet = BPFInstrInfo;
  let AssemblyWriters = [BPFInstPrinter];
}
