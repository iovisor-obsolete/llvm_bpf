//===- BPFSubtarget.cpp - BPF Subtarget Information -----------*- C++ -*-=//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#include "BPF.h"
#include "BPFSubtarget.h"
#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "BPFGenSubtargetInfo.inc"
using namespace llvm;

void BPFSubtarget::anchor() { }

BPFSubtarget::BPFSubtarget(const std::string &TT,
                           const std::string &CPU, const std::string &FS)
  : BPFGenSubtargetInfo(TT, CPU, FS)
{
  std::string CPUName = CPU;
  if (CPUName.empty())
    CPUName = "generic";

  ParseSubtargetFeatures(CPUName, FS);
}
