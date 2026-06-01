# Alpha Language Compiler & Virtual Machine

A complete compiler and virtual machine for the **Alpha** language, built as part of the HY-340 Languages & Compilers course at the University of Crete.

## Building

Requires `gcc`, `flex`, and `bison`.

```bash
make
```

This produces:
- `compiler.exe` — the Alpha compiler
- `vm.exe` — the Alpha virtual machine

To clean build artifacts:

```bash
make clean
```

## Usage

**Compile** an Alpha source file (`.asc`):

```bash
./compiler.exe your_program.asc
```

This generates `binary.abc` (bytecode).

**Run** the bytecode:

```bash
./vm.exe binary.abc
```

**Example:**

```bash
echo 'print("Hello Alpha!\n");' > hello.asc
./compiler.exe hello.asc
./vm.exe binary.abc
```

## Project Structure

```
src/        Compiler source (lexer, parser, semantic analysis, code generation)
vm/         Virtual machine source
include/    Shared header files
tests/      Test programs
Makefile    Top-level build
```

## Pipeline

```
Source (.asc) → Lexer (Flex) → Parser (Bison) → Semantic Analysis
             → Intermediate Code (Quads) → Code Generation → Bytecode (.abc)
             → Virtual Machine (AVM)
```

## Implemented Features

- Numerical calculations and arithmetic (including `%` with correct signed semantics)
- All library functions: `print`, `input`, `typeof`, `objectmemberkeys`, `objecttotalmembers`, `objectcopy`, `totalarguments`, `argument`, `strtonum`, `sqrt`, `cos`, `sin`
- User-defined functions (including anonymous and nested functions)
- Recursive functions
- Simple tables (associative arrays)
- Logic statements (`and`, `or`, `not`)
- `while` loops and `for` loops
- `break` / `continue`
- Scope rules (`local`, `::` global access)

## Known Limitations

- Complex nested table operations and the table-as-functor pattern (`obj(args)` calling via `"()"` key) are not fully supported
- Some advanced table-heavy test files (`tables1.asc`, `delegation.asc`) do not run completely

## Bug Fixes

The following bugs were identified and fixed after the initial implementation:

| # | Location | Description |
|---|----------|-------------|
| 1 | `vm/avm_functions.c` | `avm_callsaveenvironment` stored a wrong `savedTop` value, corrupting the caller's stack frame on every user-function return |
| 2 | `vm/avm_execution.c` | `execute_mod` cast operands to `unsigned` before `%`, producing wrong results for negative values; fixed with `fmod` |
| 3 | `vm/avm_execution.c` | `execute_pusharg` reset `avm_totalActuals` by inspecting `code[pc-1]` (physically adjacent instruction), which is unreliable after jumps; combined with a proper reset in `execute_funcexit` |
| 4 | `src/rule_handler.c` | `countDigits(0)` returned 0, causing `malloc(2)` for the string `"$0\0"` (3 bytes) — heap overflow on the first anonymous function |
| 5 | `vm/avm_execution.c` | `execute_mod` used unsigned cast instead of `fmod`, giving wrong results for negative operands |
| 6 | `vm/avm.c` | `avm_translate_operand` called `strdup` for `string_a` operands without first calling `avm_memcellclear`, leaking one heap allocation per string-constant read |
| 7 | `vm/avm_execution.c` | `execute_call` had a `string_m` case that silently dispatched any string value as a library function name; replaced with a proper type error |
| 8 | `src/target.c` | `consts_newstring`, `consts_newnumber`, and `libfuncs_newused` did not check `realloc`'s return value, leaking memory and crashing on allocation failure |
| 9 | `src/rule_handler.c` | Relational-operator error messages indexed `str_iopcodeName[]` (opcode names) with an `ExprType` value; fixed by introducing a dedicated `str_ExprTypeName[]` array |
| 10 | `vm/avm_execution.c` | `execute_funcenter` contained a no-op `if (avm_totalActuals == 0) avm_totalActuals = 0` branch |
