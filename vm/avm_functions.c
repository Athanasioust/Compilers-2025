#include "avm.h"

// Call stack management functions

void avm_callsaveenvironment(void) {
    avm_stack[top].type = number_m;
    avm_stack[top].data.numVal = avm_totalActuals;
    avm_dec_top();
    
    avm_stack[top].type = number_m;
    avm_stack[top].data.numVal = pc + 1;
    avm_dec_top();
    
    avm_stack[top].type = number_m;
    avm_stack[top].data.numVal = top + AVM_STACKENV_SIZE + 2;
    avm_dec_top();
    
    avm_stack[top].type = number_m;
    avm_stack[top].data.numVal = topsp;
    avm_dec_top();
}

void avm_callrestoreenvironment(void) {
    unsigned oldTop = top;
    
    topsp = (unsigned)avm_stack[oldTop + AVM_SAVEDTOPSP_OFFSET].data.numVal;
    top = (unsigned)avm_stack[oldTop + AVM_SAVEDTOP_OFFSET].data.numVal;
    pc = (unsigned)avm_stack[oldTop + AVM_SAVEDPC_OFFSET].data.numVal;
    
    // Clear local variables
    while (++oldTop <= top) {
        avm_memcellclear(&avm_stack[oldTop]);
    }
}

library_func_t avm_getlibraryfunc(char* id) {
    for (unsigned i = 0; i < totalNamedLibfuncs; i++) {
        if (strcmp(namedLibfuncs[i], id) == 0) {
            // Library function dispatch table from avm.c
            if (strcmp(id, "print") == 0) return libfunc_print;
            if (strcmp(id, "input") == 0) return libfunc_input;
            if (strcmp(id, "objectmemberkeys") == 0) return libfunc_objectmemberkeys;
            if (strcmp(id, "objecttotalmembers") == 0) return libfunc_objecttotalmembers;
            if (strcmp(id, "objectcopy") == 0) return libfunc_objectcopy;
            if (strcmp(id, "totalarguments") == 0) return libfunc_totalarguments;
            if (strcmp(id, "argument") == 0) return libfunc_argument;
            if (strcmp(id, "typeof") == 0) return libfunc_typeof;
            if (strcmp(id, "strtonum") == 0) return libfunc_strtonum;
            if (strcmp(id, "sqrt") == 0) return libfunc_sqrt;
            if (strcmp(id, "cos") == 0) return libfunc_cos;
            if (strcmp(id, "sin") == 0) return libfunc_sin;
        }
    }
    return NULL;
}

void avm_calllibfunc(char* funcName) {
    library_func_t f = avm_getlibraryfunc(funcName);
    if (!f) {
        avm_error("Unsupported library function '%s' called!", funcName);
        executionFinished = 1;
    } else {
        // Library functions don't need environment save/restore
        // Just call the function directly
        f();
        
        // Clear the arguments from the stack
        // The arguments were pushed by pusharg instructions
        while (avm_totalActuals > 0) {
            avm_memcellclear(&avm_stack[top + 1]);
            ++top;
            --avm_totalActuals;
        }
    }
}

unsigned avm_totalactuals(void) {
    return avm_totalActuals;
}

avm_memcell* avm_getactual(unsigned i) {
    assert(i < avm_totalactuals());
    return &avm_stack[top + 1 + i];
}

void avm_dec_top(void) {
    if (!top) {
        avm_error("Stack overflow!");
        executionFinished = 1;
    } else {
        --top;
    }
}

void avm_push_envvalue(unsigned val) {
    avm_stack[top].type = number_m;
    avm_stack[top].data.numVal = val;
    avm_dec_top();
}