Alpha Language Compiler & Virtual Machine
BUILDING THE COMPILER AND VM:
Place all VM files (avm.h, avm.c, avm_execution.c, avm_functions.c, avm_tables.c, avm_libfuncs.c) in a "vm/" directory

Build everything:

bash
make
This creates:

compiler.exe (the compiler)
vm.exe (the virtual machine)
RUNNING ALPHA PROGRAMS:
Compile an Alpha source file:

bash
./compiler.exe your_program.asc
This generates binary.abc (bytecode file)

Run the bytecode in the VM:

bash
./vm.exe binary.abc
EXAMPLE:
bash
echo 'print("Hello Alpha!");' > hello.asc
./compiler.exe hello.asc
./vm.exe binary.abc
CLEANING:
bash
make clean
PROJECT STRUCTURE:
src/ - Compiler source code
vm/ - Virtual machine source code
include/ - Header files
tests/ - Test programs
Makefile - Build system
