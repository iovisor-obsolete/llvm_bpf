//=====-- BPFMCAsmInfo.h - BPF asm properties -----------*- C++ -*--====//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.

#ifndef BPF_MCASM_INFO_H
#define BPF_MCASM_INFO_H

#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
  class Target;
  
  class BPFMCAsmInfo : public MCAsmInfo {
  public:
#if LLVM_VERSION_MINOR==4
    explicit BPFMCAsmInfo(StringRef TT) {
#else
    explicit BPFMCAsmInfo(const Target &T, StringRef TT) {
#endif
      PrivateGlobalPrefix         = ".L";
      WeakRefDirective            = "\t.weak\t";

      // BPF assembly requires ".section" before ".bss"
      UsesELFSectionDirectiveForBSS = true;

      HasSingleParameterDotFile = false;
      HasDotTypeDotSizeDirective = false;
    }
  };

}

#endif
