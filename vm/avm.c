#include "avm.h"
#include <stdarg.h>

// Global VM state
avm_memcell avm_stack[AVM_STACKSIZE];
avm_memcell ax, bx, cx;
avm_memcell retval;
unsigned top, topsp;
unsigned pc = 0;
unsigned currLine = 0;
unsigned codeSize = 0;
instruction* code = NULL;
unsigned executionFinished = 0;
unsigned avm_totalActuals = 0;

// Constant arrays
double* numConsts = NULL;
unsigned totalNumConsts = 0;
char** stringConsts = NULL;
unsigned totalStringConsts = 0;
char** namedLibfuncs = NULL;
unsigned totalNamedLibfuncs = 0;
userfunc* userFuncs = NULL;
unsigned totalUserFuncs = 0;

// Library function dispatch table
library_func_t libraryFuncs[] = {
    libfunc_print,
    libfunc_input, 
    libfunc_objectmemberkeys,
    libfunc_objecttotalmembers,
    libfunc_objectcopy,
    libfunc_totalarguments,
    libfunc_argument,
    libfunc_typeof,
    libfunc_strtonum,
    libfunc_sqrt,
    libfunc_cos,
    libfunc_sin
};

// Execution function dispatch table
typedef void (*execute_func_t)(instruction*);

execute_func_t executeFuncs[] = {
    execute_assign,
    execute_add,
    execute_sub, 
    execute_mul,
    execute_div,
    execute_mod,
    execute_uminus,
    execute_and,
    execute_or,
    execute_not,
    execute_jeq,
    execute_jne,
    execute_jle,
    execute_jge,
    execute_jlt,
    execute_jgt,
    execute_call,
    execute_pusharg,
    execute_funcenter,
    execute_funcexit,
    execute_newtable,
    execute_tablegetelem,
    execute_tablesetelem,
    execute_jump,
    execute_nop
};

void avm_initialize(void) {
    AVM_WIPEOUT(avm_stack);
    AVM_WIPEOUT(ax);
    AVM_WIPEOUT(bx); 
    AVM_WIPEOUT(cx);
    AVM_WIPEOUT(retval);
    
    top = AVM_STACKSIZE - 1;
    topsp = AVM_STACKSIZE - 1;
    pc = 0;
    currLine = 0;
    executionFinished = 0;
}

void avm_load_program(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        avm_error("Cannot open binary file: %s", filename);
        return;
    }
    
    // Read magic number
    unsigned magic;
    fread(&magic, sizeof(unsigned), 1, file);
    if (magic != 0xA1FA) {
        avm_error("Invalid binary file format");
        fclose(file);
        return;
    }
    
    // Read string constants
    fread(&totalStringConsts, sizeof(unsigned), 1, file);
    if (totalStringConsts > 0) {
        stringConsts = malloc(totalStringConsts * sizeof(char*));
        for (unsigned i = 0; i < totalStringConsts; i++) {
            unsigned len;
            fread(&len, sizeof(unsigned), 1, file);
            stringConsts[i] = malloc(len);
            fread(stringConsts[i], sizeof(char), len, file);
        }
    }
    
    // Read number constants
    fread(&totalNumConsts, sizeof(unsigned), 1, file);
    if (totalNumConsts > 0) {
        numConsts = malloc(totalNumConsts * sizeof(double));
        fread(numConsts, sizeof(double), totalNumConsts, file);
    }
    
    // Read library functions
    fread(&totalNamedLibfuncs, sizeof(unsigned), 1, file);
    if (totalNamedLibfuncs > 0) {
        namedLibfuncs = malloc(totalNamedLibfuncs * sizeof(char*));
        for (unsigned i = 0; i < totalNamedLibfuncs; i++) {
            unsigned len;
            fread(&len, sizeof(unsigned), 1, file);
            namedLibfuncs[i] = malloc(len);
            fread(namedLibfuncs[i], sizeof(char), len, file);
        }
    }
    
    // Read user functions
    fread(&totalUserFuncs, sizeof(unsigned), 1, file);
    if (totalUserFuncs > 0) {
        userFuncs = malloc(totalUserFuncs * sizeof(userfunc));
        for (unsigned i = 0; i < totalUserFuncs; i++) {
            fread(&userFuncs[i].address, sizeof(unsigned), 1, file);
            fread(&userFuncs[i].localSize, sizeof(unsigned), 1, file);
            unsigned len;
            fread(&len, sizeof(unsigned), 1, file);
            userFuncs[i].id = malloc(len);
            fread(userFuncs[i].id, sizeof(char), len, file);
        }
    }
    
    // Read instructions
    fread(&codeSize, sizeof(unsigned), 1, file);
    if (codeSize > 0) {
        code = malloc(codeSize * sizeof(instruction));
        fread(code, sizeof(instruction), codeSize, file);
    }
    
    fclose(file);
    
    // Initialize stack to accommodate global variables
    top = AVM_STACKSIZE - 10;
    topsp = top;
}

void avm_run(void) {
    if (!code) {
        avm_error("No program loaded");
        return;
    }
    
    instruction* instr;
    
    while (!executionFinished && pc < codeSize) {
        instr = &code[pc];
        currLine = instr->srcLine;
        
        if (instr->opcode < 0 || instr->opcode >= sizeof(executeFuncs)/sizeof(executeFuncs[0])) {
            avm_error("Invalid opcode: %d", instr->opcode);
            executionFinished = 1;
            break;
        }
        
        executeFuncs[instr->opcode](instr);
        ++pc;
    }
}

// Memory cell operations
void avm_memcellclear(avm_memcell* m) {
    if (m->type != undef_m) {
        switch (m->type) {
            case string_m:
                free(m->data.strVal);
                break;
            case table_m:
                assert(m->data.tableVal);
                avm_tabledecrefcounter(m->data.tableVal);
                break;
            default:
                break;
        }
    }
    m->type = undef_m;
}

void avm_assign(avm_memcell* lv, avm_memcell* rv) {
    if (lv == rv) return;
    
    if (lv->type == table_m && rv->type == table_m && lv->data.tableVal == rv->data.tableVal)
        return;
        
    if (rv->type == undef_m)
        avm_warning("Assigning from undefined content!");
        
    avm_memcellclear(lv);
    
    switch (rv->type) {
        case number_m:
        case bool_m:
        case nil_m:
        case userfunc_m:
        case libfunc_m:
            *lv = *rv;
            break;
            
        case string_m:
            lv->type = string_m;
            lv->data.strVal = strdup(rv->data.strVal);
            break;
            
        case table_m:
            lv->type = table_m;
            lv->data.tableVal = rv->data.tableVal;
            avm_tableincrefcounter(lv->data.tableVal);
            break;
            
        default:
            assert(0);
    }
}

avm_memcell* avm_translate_operand(vmarg* arg, avm_memcell* reg) {
    switch (arg->type) {
        case global_a:
            return &avm_stack[AVM_STACKSIZE-1-arg->val];
            
        case local_a:
            return &avm_stack[topsp-arg->val];
            
        case formal_a:
            return &avm_stack[topsp+AVM_STACKENV_SIZE+1+arg->val];
            
        case retval_a:
            return &retval;
            
        case number_a:
            reg->type = number_m;
            reg->data.numVal = numConsts[arg->val];
            return reg;
            
        case string_a:
            reg->type = string_m;
            reg->data.strVal = strdup(stringConsts[arg->val]);
            return reg;
            
        case bool_a:
            reg->type = bool_m;
            reg->data.boolVal = arg->val;
            return reg;
            
        case nil_a:
            reg->type = nil_m;
            return reg;
            
        case userfunc_a:
            reg->type = userfunc_m;
            reg->data.funcVal = arg->val;
            return reg;
            
        case libfunc_a:
            reg->type = libfunc_m;
            reg->data.libfuncVal = namedLibfuncs[arg->val];
            return reg;
            
        default:
            assert(0);
    }
}

// Error handling
void avm_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Runtime error (line %u): ", currLine);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    executionFinished = 1;
}

void avm_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "Runtime warning (line %u): ", currLine);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
}

// Utility functions
unsigned char avm_tobool(avm_memcell* m) {
    assert(m);
    
    switch (m->type) {
        case number_m:
            return m->data.numVal != 0;
        case string_m:
            return m->data.strVal[0] != 0;
        case bool_m:
            return m->data.boolVal;
        case table_m:
            return 1;
        case userfunc_m:
            return 1;
        case libfunc_m:
            return 1;
        case nil_m:
            return 0;
        case undef_m:
            avm_error("'undef' used in boolean!");
            return 0;
        default:
            assert(0);
    }
}

double avm_tonumber(avm_memcell* m) {
    assert(m);
    
    switch (m->type) {
        case number_m:
            return m->data.numVal;
        case bool_m:
            return (double)m->data.boolVal;
        case nil_m:
            return 0.0;
        default:
            avm_error("Cannot convert to number!");
            return 0.0;
    }
}

const char* avm_tostring(avm_memcell* m) {
    assert(m);
    static char buffer[256];
    
    switch (m->type) {
        case string_m:
            return m->data.strVal;
        case number_m:
            snprintf(buffer, sizeof(buffer), "%.3f", m->data.numVal);
            return buffer;
        case bool_m:
            return m->data.boolVal ? "true" : "false";
        case nil_m:
            return "nil";
        case userfunc_m:
            snprintf(buffer, sizeof(buffer), "user function %s", userFuncs[m->data.funcVal].id);
            return buffer;
        case libfunc_m:
            snprintf(buffer, sizeof(buffer), "library function %s", m->data.libfuncVal);
            return buffer;
        case table_m:
            return "table";
        case undef_m:
            return "undefined";
        default:
            assert(0);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }
    
    avm_initialize();
    avm_load_program(argv[1]);
    
    if (!executionFinished) {
        avm_run();
    }
    
    return 0;
}