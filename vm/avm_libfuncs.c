#define _USE_MATH_DEFINES
#include <math.h>
#include "avm.h"

void libfunc_print(void) {
    unsigned n = avm_totalactuals();
    
    for (unsigned i = 0; i < n; ++i) {
        avm_memcell* arg = avm_getactual(i);
        
        switch (arg->type) {
            case string_m:
                printf("%s", arg->data.strVal);
                break;
            case number_m:
                printf("%g", arg->data.numVal);
                break;
            case bool_m:
                printf("%s", arg->data.boolVal ? "true" : "false");
                break;
            case nil_m:
                printf("nil");
                break;
            case userfunc_m:
                printf("user function %s", userFuncs[arg->data.funcVal].id);
                break;
            case libfunc_m:
                printf("library function %s", arg->data.libfuncVal);
                break;
            case table_m:
                printf("%s", avm_table_tostring(arg->data.tableVal));
                break;
            case undef_m:
                printf("undefined");
                break;
            default:
                assert(0);
        }
    }
}

void libfunc_input(void) {
    char buffer[512];
    if (fgets(buffer, sizeof(buffer), stdin)) {
        // Remove newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n')
            buffer[len-1] = '\0';
        
        // Try to parse as number
        char* endptr;
        double num = strtod(buffer, &endptr);
        if (*endptr == '\0' && endptr != buffer) {
            retval.type = number_m;
            retval.data.numVal = num;
            return;
        }
        
        // Try to parse as boolean
        if (strcmp(buffer, "true") == 0) {
            retval.type = bool_m;
            retval.data.boolVal = 1;
            return;
        }
        if (strcmp(buffer, "false") == 0) {
            retval.type = bool_m;
            retval.data.boolVal = 0;
            return;
        }
        
        // Try to parse as nil
        if (strcmp(buffer, "nil") == 0) {
            retval.type = nil_m;
            return;
        }
        
        // Return as string (remove quotes if present)
        if (buffer[0] == '"' && len > 2 && buffer[len-1] == '"') {
            buffer[len-1] = '\0';
            retval.type = string_m;
            retval.data.strVal = strdup(buffer + 1);
        } else {
            retval.type = string_m;
            retval.data.strVal = strdup(buffer);
        }
    } else {
        retval.type = nil_m;
    }
}

void libfunc_objectmemberkeys(void) {
    if (avm_totalactuals() != 1) {
        avm_error("objectmemberkeys: wrong number of arguments!");
        return;
    }
    
    avm_memcell* arg = avm_getactual(0);
    if (arg->type != table_m) {
        avm_error("objectmemberkeys: argument must be a table!");
        return;
    }
    
    retval.type = table_m;
    retval.data.tableVal = avm_tablenew();
    avm_tableincrefcounter(retval.data.tableVal);
    
    avm_table_traverse(arg->data.tableVal, avm_table_keys_visitor, &retval);
}

void libfunc_objecttotalmembers(void) {
    if (avm_totalactuals() != 1) {
        avm_error("objecttotalmembers: wrong number of arguments!");
        return;
    }
    
    avm_memcell* arg = avm_getactual(0);
    if (arg->type != table_m) {
        avm_error("objecttotalmembers: argument must be a table!");
        return;
    }
    
    retval.type = number_m;
    retval.data.numVal = arg->data.tableVal->total;
}

void libfunc_objectcopy(void) {
    if (avm_totalactuals() != 1) {
        avm_error("objectcopy: wrong number of arguments!");
        return;
    }
    
    avm_memcell* arg = avm_getactual(0);
    if (arg->type != table_m) {
        avm_error("objectcopy: argument must be a table!");
        return;
    }
    
    retval.type = table_m;
    retval.data.tableVal = avm_tablenew();
    avm_tableincrefcounter(retval.data.tableVal);
    
    avm_table_traverse(arg->data.tableVal, avm_table_copy_visitor, &retval);
}

void libfunc_totalarguments(void) {
    unsigned prev_topsp = avm_stack[topsp + AVM_SAVEDTOPSP_OFFSET].data.numVal;
    if (prev_topsp == AVM_STACKSIZE - 1) {
        // Global scope
        retval.type = nil_m;
    } else {
        retval.type = number_m;
        retval.data.numVal = avm_stack[prev_topsp + AVM_NUMACTUALS_OFFSET].data.numVal;
    }
}

void libfunc_argument(void) {
    if (avm_totalactuals() != 1) {
        avm_error("argument: wrong number of arguments!");
        return;
    }
    
    avm_memcell* arg = avm_getactual(0);
    if (arg->type != number_m) {
        avm_error("argument: argument must be a number!");
        return;
    }
    
    unsigned i = (unsigned)arg->data.numVal;
    unsigned prev_topsp = avm_stack[topsp + AVM_SAVEDTOPSP_OFFSET].data.numVal;
    
    if (prev_topsp == AVM_STACKSIZE - 1) {
        // Global scope
        retval.type = nil_m;
    } else {
        unsigned totalArgs = avm_stack[prev_topsp + AVM_NUMACTUALS_OFFSET].data.numVal;
        if (i >= totalArgs) {
            retval.type = nil_m;
        } else {
            avm_assign(&retval, &avm_stack[prev_topsp + AVM_STACKENV_SIZE + 1 + i]);
        }
    }
}

void libfunc_typeof(void) {
    if (avm_totalactuals() != 1) {
        avm_error("typeof: wrong number of arguments!");
        return;
    }
    
    avm_memcell* arg = avm_getactual(0);
    
    char* typeStr;
    switch (arg->type) {
        case number_m:      typeStr = "number"; break;
        case string_m:      typeStr = "string"; break;
        case bool_m:        typeStr = "boolean"; break;
        case table_m:       typeStr = "table"; break;
        case userfunc_m:    typeStr = "userfunction"; break;
        case libfunc_m:     typeStr = "libraryfunction"; break;
        case nil_m:         typeStr = "nil"; break;
        case undef_m:       typeStr = "undefined"; break;
        default:            typeStr = "unknown"; break;
    }
    
    retval.type = string_m;
    retval.data.strVal = strdup(typeStr);
}

void libfunc_strtonum(void) {
    if (avm_totalactuals() != 1) {
        avm_error("strtonum: wrong number of arguments!");
        return;
    }
    
    avm_memcell* arg = avm_getactual(0);
    if (arg->type != string_m) {
        avm_error("strtonum: argument must be a string!");
        return;
    }
    
    char* endptr;
    double num = strtod(arg->data.strVal, &endptr);
    
    if (*endptr == '\0' && endptr != arg->data.strVal) {
        retval.type = number_m;
        retval.data.numVal = num;
    } else {
        retval.type = nil_m;
    }
}

void libfunc_sqrt(void) {
    if (avm_totalactuals() != 1) {
        avm_error("sqrt: wrong number of arguments!");
        return;
    }
    
    avm_memcell* arg = avm_getactual(0);
    if (arg->type != number_m) {
        avm_error("sqrt: argument must be a number!");
        return;
    }
    
    if (arg->data.numVal < 0) {
        retval.type = nil_m;
    } else {
        retval.type = number_m;
        retval.data.numVal = sqrt(arg->data.numVal);
    }
}

void libfunc_cos(void) {
    if (avm_totalactuals() != 1) {
        avm_error("cos: wrong number of arguments!");
        return;
    }
    
    avm_memcell* arg = avm_getactual(0);
    if (arg->type != number_m) {
        avm_error("cos: argument must be a number!");
        return;
    }
    
    // Convert degrees to radians
    double radians = arg->data.numVal * M_PI / 180.0;
    
    retval.type = number_m;
    retval.data.numVal = cos(radians);
}

void libfunc_sin(void) {
    if (avm_totalactuals() != 1) {
        avm_error("sin: wrong number of arguments!");
        return;
    }
    
    avm_memcell* arg = avm_getactual(0);
    if (arg->type != number_m) {
        avm_error("sin: argument must be a number!");
        return;
    }
    
    // Convert degrees to radians  
    double radians = arg->data.numVal * M_PI / 180.0;
    
    retval.type = number_m;
    retval.data.numVal = sin(radians);
}