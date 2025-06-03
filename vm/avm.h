#ifndef AVM_H
#define AVM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define AVM_STACKSIZE 4096
#define AVM_WIPEOUT(m) memset(&(m), 0, sizeof(m))
#define AVM_TABLE_HASHSIZE 211


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

// VM argument types (same as target.h)
typedef enum vmarg_t {
    label_a, global_a, formal_a, local_a, number_a,
    string_a, bool_a, nil_a, userfunc_a, libfunc_a, retval_a
} vmarg_t;

// VM argument
typedef struct vmarg {
    vmarg_t type;
    unsigned val;
} vmarg;

// VM instruction
typedef struct instruction {
    vmopcode opcode;
    vmarg result;
    vmarg arg1;
    vmarg arg2;
    unsigned srcLine;
} instruction;

// Memory cell types
typedef enum avm_memcell_t {
    number_m = 0,
    string_m = 1,
    bool_m = 2,
    table_m = 3,
    userfunc_m = 4,
    libfunc_m = 5,
    nil_m = 6,
    undef_m = 7
} avm_memcell_t;

// Forward declarations
struct avm_memcell;
struct avm_table;

// Memory cell
typedef struct avm_memcell {
    avm_memcell_t type;
    union {
        double numVal;
        char* strVal;
        unsigned char boolVal;
        struct avm_table* tableVal;
        unsigned funcVal;
        char* libfuncVal;
    } data;
} avm_memcell;

// Table bucket for hash table implementation
typedef struct avm_table_bucket {
    struct avm_memcell key;
    struct avm_memcell value;
    struct avm_table_bucket* next;
} avm_table_bucket;

// Table structure
typedef struct avm_table {
    unsigned refCounter;
    avm_table_bucket* strIndexed[AVM_TABLE_HASHSIZE];
    avm_table_bucket* numIndexed[AVM_TABLE_HASHSIZE];
    avm_table_bucket* funcIndexed[AVM_TABLE_HASHSIZE];
    unsigned total;
} avm_table;

// User function info
typedef struct userfunc {
    unsigned address;
    unsigned localSize;
    char* id;
} userfunc;

// Library function type
typedef void (*library_func_t)(void);

// Missing function declarations
library_func_t avm_getlibraryfunc(char* id);
void avm_dec_top(void);
void avm_push_envvalue(unsigned val);

// Stack environment offsets
#define AVM_STACKENV_SIZE 4
#define AVM_NUMACTUALS_OFFSET +4
#define AVM_SAVEDPC_OFFSET +3
#define AVM_SAVEDTOP_OFFSET +2
#define AVM_SAVEDTOPSP_OFFSET +1
#define AVM_ENDING_PC codeSize

// Global VM state
extern avm_memcell avm_stack[AVM_STACKSIZE];
extern avm_memcell ax, bx, cx;
extern avm_memcell retval;
extern unsigned top, topsp;
extern unsigned pc;
extern unsigned currLine;
extern unsigned codeSize;
extern instruction* code;
extern unsigned executionFinished;
extern unsigned avm_totalActuals;

// Constant arrays
extern double* numConsts;
extern unsigned totalNumConsts;
extern char** stringConsts;
extern unsigned totalStringConsts;
extern char** namedLibfuncs;
extern unsigned totalNamedLibfuncs;
extern userfunc* userFuncs;
extern unsigned totalUserFuncs;

// Function prototypes
void avm_initialize(void);
void avm_load_program(const char* filename);
void avm_run(void);

// Memory cell operations
void avm_memcellclear(avm_memcell* m);
void avm_assign(avm_memcell* lv, avm_memcell* rv);
avm_memcell* avm_translate_operand(vmarg* arg, avm_memcell* reg);

// Table operations
avm_table* avm_tablenew(void);
void avm_tableincrefcounter(avm_table* t);
void avm_tabledecrefcounter(avm_table* t);
void avm_tablebucketsinit(avm_table_bucket** p);
void avm_tablebucketsdestroy(avm_table_bucket** p);
void avm_tablesetelem(avm_table* table, avm_memcell* index, avm_memcell* content);
avm_memcell* avm_tablegetelem(avm_table* table, avm_memcell* index);
void avm_tabledestroy(avm_table* t);
char* avm_table_tostring(avm_table* table);

// Execution functions
void execute_assign(instruction* instr);
void execute_add(instruction* instr);
void execute_sub(instruction* instr);
void execute_mul(instruction* instr);
void execute_div(instruction* instr);
void execute_mod(instruction* instr);
void execute_uminus(instruction* instr);
void execute_and(instruction* instr);
void execute_or(instruction* instr);
void execute_not(instruction* instr);
void execute_jeq(instruction* instr);
void execute_jne(instruction* instr);
void execute_jle(instruction* instr);
void execute_jge(instruction* instr);
void execute_jlt(instruction* instr);
void execute_jgt(instruction* instr);
void execute_call(instruction* instr);
void execute_pusharg(instruction* instr);
void execute_funcenter(instruction* instr);
void execute_funcexit(instruction* instr);
void execute_newtable(instruction* instr);
void execute_tablegetelem(instruction* instr);
void execute_tablesetelem(instruction* instr);
void execute_jump(instruction* instr);
void execute_nop(instruction* instr);

// Library functions
void libfunc_print(void);
void libfunc_input(void);
void libfunc_objectmemberkeys(void);
void libfunc_objecttotalmembers(void);
void libfunc_objectcopy(void);
void libfunc_totalarguments(void);
void libfunc_argument(void);
void libfunc_typeof(void);
void libfunc_strtonum(void);
void libfunc_sqrt(void);
void libfunc_cos(void);
void libfunc_sin(void);

// Utility functions
void avm_error(const char* format, ...);
void avm_warning(const char* format, ...);
const char* avm_tostring(avm_memcell* m);
unsigned char avm_tobool(avm_memcell* m);
double avm_tonumber(avm_memcell* m);
unsigned char avm_equality(avm_memcell* rv1, avm_memcell* rv2);

// Call stack management
void avm_callsaveenvironment(void);
void avm_callrestoreenvironment(void);
void avm_calllibfunc(char* funcName);
void avm_callsaveactualargs(void);
unsigned avm_totalactuals(void);
avm_memcell* avm_getactual(unsigned i);

// Table traversal
typedef void (*avm_table_visitor_t)(avm_memcell* key, avm_memcell* value, unsigned* index, avm_memcell* result);
void avm_table_traverse(avm_table* table, avm_table_visitor_t visitor, avm_memcell* result);
void avm_table_keys_visitor(avm_memcell* key, avm_memcell* value, unsigned* index, avm_memcell* result);
void avm_table_copy_visitor(avm_memcell* key, avm_memcell* value, unsigned* index, avm_memcell* result);

#endif // AVM_H