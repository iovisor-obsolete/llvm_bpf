LLVM BPF backend:
lib/Target/BPF/*.cpp

Links with LLVM 3.2, 3.3 and 3.4

prerequisites:
apt-get install clang llvm-3.[234]-dev

To build:
$cd bld
$make
if 'llvm-config-3.2' is not found in PATH, build with:
$make -j4 LLVM_CONFIG=/path_to/llvm-config

To run:
$clang -O2 -emit-llvm -c file.c -o -|./bld/Debug+Asserts/bin/llc -o file.bpf

'clang' - is unmodified clang used to build x86 code
'llc' - llvm bit-code to BPF compiler
file.bpf - BPF binary image, see include/linux/bpf_jit.h

$clang -O2 -emit-llvm -c file.c -o -|llc -filetype=asm -o file.s
will emit human readable BPF assembler instead.
