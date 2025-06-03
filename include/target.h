#ifndef _TARGET_H_
#define _TARGET_H_

#include "structs.h"
#include "stack.h"

// VM Opcodes
typedef enum vmopcode {
    assign_v,
    add_v,
    sub_v,
    mul_v,
    div_v,
    mod_v,
    uminus_v,
    and_v,
    or_v,
    not_v,
    jeq_v,
    jne_v,
    jle_v,
    jge_v,
    jlt_v,
    jgt_v,
    jump_v,
    call_v,
    pusharg_v,
    funcenter_v,
    funcexit_v,
    newtable_v,
    tablegetelem_v,
    tablesetelem_v,
    nop_v
} vmopcode;

// VM Argument types
typedef enum vmarg_t {
    label_a,
    global_a,
    formal_a,
    local_a,
    number_a,
    string_a,
    bool_a,
    nil_a,
    userfunc_a,
    libfunc_a,
    retval_a
} vmarg_t;

// VM Argument
typedef struct vmarg {
    vmarg_t type;
    unsigned val;
} vmarg;

// VM Instruction
typedef struct instruction {
    vmopcode opcode;
    vmarg result;
    vmarg arg1;
    vmarg arg2;
    unsigned srcLine;
} instruction;

// Incomplete jump list
typedef struct incomplete_jump {
    unsigned instrNo;
    unsigned iaddress;
    struct incomplete_jump* next;
} incomplete_jump;

// Function stack for nested functions
typedef struct func_stack {
    SymbolTableEntry* func;
    incomplete_jump* returnList;  
    struct func_stack* next;
} func_stack;

// Global variables for code generation
extern instruction* instructions;
extern unsigned total_instructions;
extern unsigned curr_instruction;

extern incomplete_jump* ij_head;
extern unsigned ij_total;

extern func_stack* funcstack;

// Constant arrays
extern double* numConsts;
extern unsigned totalNumConsts;
extern unsigned currNumConst;

extern char** stringConsts;
extern unsigned totalStringConsts;
extern unsigned currStringConst;

extern char** namedLibfuncs;
extern unsigned totalNamedLibfuncs;
extern unsigned currNamedLibfunc;

extern SymbolTableEntry** userFuncs;
extern unsigned totalUserFuncs;
extern unsigned currUserFunc;
extern stack_T funcJumpStack;

// Macro for instruction expansion
#define EXPAND_SIZE_INSTR 1024
#define CURR_SIZE_INSTR (total_instructions * sizeof(instruction))
#define NEW_SIZE_INSTR (EXPAND_SIZE_INSTR * sizeof(instruction) + CURR_SIZE_INSTR)

// Code generation functions
void generate(void);
void generate_ADD(quad*);
void generate_SUB(quad*);
void generate_MUL(quad*);
void generate_DIV(quad*);
void generate_MOD(quad*);
void generate_UMINUS(quad*);
void generate_NEWTABLE(quad*);
void generate_TABLEGETELEM(quad*);
void generate_TABLESETELEM(quad*);
void generate_ASSIGN(quad*);
void generate_NOP(void);
void generate_JUMP(quad*);
void generate_IF_EQ(quad*);
void generate_IF_NOTEQ(quad*);
void generate_IF_GREATER(quad*);
void generate_IF_GREATEREQ(quad*);
void generate_IF_LESS(quad*);
void generate_IF_LESSEQ(quad*);
void generate_NOT(quad*);
void generate_OR(quad*);
void generate_AND(quad*);
void generate_PARAM(quad*);
void generate_CALL(quad*);
void generate_GETRETVAL(quad*);
void generate_FUNCSTART(quad*);
void generate_RETURN(quad*);
void generate_FUNCEND(quad*);

// Helper functions
void make_operand(Expr*, vmarg*);
void make_numberoperand(vmarg*, double);
void make_booloperand(vmarg*, unsigned);
void make_retvaloperand(vmarg*);

void emit_instruction(instruction*);
void expand_instructions(void);

unsigned consts_newstring(char*);
unsigned consts_newnumber(double);
unsigned libfuncs_newused(char*);
unsigned userfuncs_newfunc(SymbolTableEntry*);

void add_incomplete_jump(unsigned, unsigned);
void patch_incomplete_jumps(void);

void push_funcstack(SymbolTableEntry*);
func_stack* pop_funcstack(void);

void backpatch_returns(incomplete_jump*);

void reset_operand(vmarg*);

// Target code output
void print_target_code(void);
void print_binary_file(const char*);
void print_const_tables(void);

void generate(void);
void print_target_code(void);
void print_binary_file(const char* filename);

#endif // _TARGET_H_