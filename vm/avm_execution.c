#include "avm.h"

// Arithmetic operations
void execute_assign(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv = avm_translate_operand(&instr->arg1, &ax);
    
    assert(lv && ((&avm_stack[AVM_STACKSIZE-1] >= lv && lv > &avm_stack[top]) || lv == &retval));
    assert(rv);
    
    avm_assign(lv, rv);
}

void execute_add(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(lv && ((&avm_stack[AVM_STACKSIZE-1] >= lv && lv > &avm_stack[top]) || lv == &retval));
    assert(rv1 && rv2);
    
    if (rv1->type != number_m || rv2->type != number_m) {
        avm_error("Not a number in arithmetic!");
        executionFinished = 1;
        return;
    }
    
    avm_memcellclear(lv);
    lv->type = number_m;
    lv->data.numVal = rv1->data.numVal + rv2->data.numVal;
}

void execute_sub(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(lv && (((&avm_stack[AVM_STACKSIZE-1] >= lv) && (lv > &avm_stack[top])) || (lv == &retval)));
    assert(rv1 && rv2);
    
    if (rv1->type != number_m || rv2->type != number_m) {
        avm_error("Not a number in arithmetic!");
        executionFinished = 1;
        return;
    }
    
    avm_memcellclear(lv);
    lv->type = number_m;
    lv->data.numVal = rv1->data.numVal - rv2->data.numVal;
}

void execute_mul(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(lv && (((&avm_stack[AVM_STACKSIZE-1] >= lv) && (lv > &avm_stack[top])) || (lv == &retval)));
    assert(rv1 && rv2);
    
    if (rv1->type != number_m || rv2->type != number_m) {
        avm_error("Not a number in arithmetic!");
        executionFinished = 1;
        return;
    }
    
    avm_memcellclear(lv);
    lv->type = number_m;
    lv->data.numVal = rv1->data.numVal * rv2->data.numVal;
}

void execute_div(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(lv && (((&avm_stack[AVM_STACKSIZE-1] >= lv) && (lv > &avm_stack[top])) || (lv == &retval)));
    assert(rv1 && rv2);
    
    if (rv1->type != number_m || rv2->type != number_m) {
        avm_error("Not a number in arithmetic!");
        executionFinished = 1;
        return;
    }
    
    if (rv2->data.numVal == 0) {
        avm_error("Division by zero!");
        executionFinished = 1;
        return;
    }
    
    avm_memcellclear(lv);
    lv->type = number_m;
    lv->data.numVal = rv1->data.numVal / rv2->data.numVal;
}

void execute_mod(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(lv && (((&avm_stack[AVM_STACKSIZE-1] >= lv) && (lv > &avm_stack[top])) || (lv == &retval)));
    assert(rv1 && rv2);
    
    if (rv1->type != number_m || rv2->type != number_m) {
        avm_error("Not a number in arithmetic!");
        executionFinished = 1;
        return;
    }
    
    if (rv2->data.numVal == 0) {
        avm_error("Modulo by zero!");
        executionFinished = 1;
        return;
    }
    
    avm_memcellclear(lv);
    lv->type = number_m;
    lv->data.numVal = ((unsigned)rv1->data.numVal) % ((unsigned)rv2->data.numVal);
}

void execute_uminus(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv = avm_translate_operand(&instr->arg1, &ax);
    
    assert((lv && ((&avm_stack[AVM_STACKSIZE-1] >= lv) && (lv > &avm_stack[top]))) || (lv == &retval));
    assert(rv);
    
    if (rv->type != number_m) {
        avm_error("Not a number in arithmetic!");
        executionFinished = 1;
        return;
    }
    
    avm_memcellclear(lv);
    lv->type = number_m;
    lv->data.numVal = -rv->data.numVal;
}

// Logical operations
void execute_and(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(lv && (((&avm_stack[AVM_STACKSIZE-1] >= lv) && (lv > &avm_stack[top])) || (lv == &retval)));
    assert(rv1 && rv2);
    
    avm_memcellclear(lv);
    lv->type = bool_m;
    lv->data.boolVal = avm_tobool(rv1) && avm_tobool(rv2);
}

void execute_or(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(lv && (((&avm_stack[AVM_STACKSIZE-1] >= lv) && (lv > &avm_stack[top])) || (lv == &retval)));
    assert(rv1 && rv2);
    
    avm_memcellclear(lv);
    lv->type = bool_m;
    lv->data.boolVal = avm_tobool(rv1) || avm_tobool(rv2);
}

void execute_not(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* rv = avm_translate_operand(&instr->arg1, &ax);
    
    assert(lv && (((&avm_stack[AVM_STACKSIZE-1] >= lv) && (lv > &avm_stack[top])) || (lv == &retval)));
    assert(rv);
    
    avm_memcellclear(lv);
    lv->type = bool_m;
    lv->data.boolVal = !avm_tobool(rv);
}

// Relational operations - helper function
unsigned char avm_equality(avm_memcell* rv1, avm_memcell* rv2) {
    if (rv1->type == undef_m || rv2->type == undef_m) {
        avm_error("'undef' involved in equality!");
        return 0;
    }
    
    if (rv1->type == nil_m || rv2->type == nil_m)
        return rv1->type == nil_m && rv2->type == nil_m;
        
    if (rv1->type == bool_m || rv2->type == bool_m)
        return avm_tobool(rv1) == avm_tobool(rv2);
        
    if (rv1->type != rv2->type) {
        avm_error("Equality of different types!");
        return 0;
    }
    
    switch (rv1->type) {
        case number_m:
            return rv1->data.numVal == rv2->data.numVal;
        case string_m:
            return strcmp(rv1->data.strVal, rv2->data.strVal) == 0;
        case userfunc_m:
            return rv1->data.funcVal == rv2->data.funcVal;
        case libfunc_m:
            return strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal) == 0;
        case table_m:
            return rv1->data.tableVal == rv2->data.tableVal;
        default:
            assert(0);
    }
}

void execute_jeq(instruction* instr) {
    assert(instr->result.type == label_a);
    
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    unsigned char result = 0;
    
    if (rv1->type == undef_m || rv2->type == undef_m)
        avm_error("'undef' involved in equality!");
    else if (rv1->type == nil_m || rv2->type == nil_m)
        result = (rv1->type == nil_m && rv2->type == nil_m);
    else if (rv1->type == bool_m || rv2->type == bool_m)
        result = (avm_tobool(rv1) == avm_tobool(rv2));
    else if (rv1->type != rv2->type)
        result = 0;
    else {
        switch (rv1->type) {
            case number_m:
                result = (rv1->data.numVal == rv2->data.numVal);
                break;
            case string_m:
                result = (strcmp(rv1->data.strVal, rv2->data.strVal) == 0);
                break;
            case userfunc_m:
                result = (rv1->data.funcVal == rv2->data.funcVal);
                break;
            case libfunc_m:
                result = (strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal) == 0);
                break;
            case table_m:
                result = (rv1->data.tableVal == rv2->data.tableVal);
                break;
            default:
                assert(0);
        }
    }
    
    if (!executionFinished && result)
        pc = instr->result.val;
}

void execute_jne(instruction* instr) {
    assert(instr->result.type == label_a);
    
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    unsigned char result = 0;
    
    if (rv1->type == undef_m || rv2->type == undef_m)
        avm_error("'undef' involved in equality!");
    else if (rv1->type == nil_m || rv2->type == nil_m)
        result = !(rv1->type == nil_m && rv2->type == nil_m);
    else if (rv1->type == bool_m || rv2->type == bool_m)
        result = (avm_tobool(rv1) != avm_tobool(rv2));
    else if (rv1->type != rv2->type)
        result = 1;
    else {
        switch (rv1->type) {
            case number_m:
                result = (rv1->data.numVal != rv2->data.numVal);
                break;
            case string_m:
                result = (strcmp(rv1->data.strVal, rv2->data.strVal) != 0);
                break;
            case userfunc_m:
                result = (rv1->data.funcVal != rv2->data.funcVal);
                break;
            case libfunc_m:
                result = (strcmp(rv1->data.libfuncVal, rv2->data.libfuncVal) != 0);
                break;
            case table_m:
                result = (rv1->data.tableVal != rv2->data.tableVal);
                break;
            default:
                assert(0);
        }
    }
    
    if (!executionFinished && result)
        pc = instr->result.val;
}

void execute_jle(instruction* instr) {
    assert(instr->result.type == label_a);
    
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(rv1 && rv2);
    
    if (rv1->type != number_m || rv2->type != number_m) {
        avm_error("Not numbers in relational!");
        executionFinished = 1;
        return;
    }
    
    unsigned char result = rv1->data.numVal <= rv2->data.numVal;
    
    if (!executionFinished && result)
        pc = instr->result.val;
}

void execute_jge(instruction* instr) {
    assert(instr->result.type == label_a);
    
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(rv1 && rv2);
    
    if (rv1->type != number_m || rv2->type != number_m) {
        avm_error("Not numbers in relational!");
        executionFinished = 1;
        return;
    }
    
    unsigned char result = rv1->data.numVal >= rv2->data.numVal;
    
    if (!executionFinished && result)
        pc = instr->result.val;
}

void execute_jlt(instruction* instr) {
    assert(instr->result.type == label_a);
    
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(rv1 && rv2);
    
    if (rv1->type != number_m || rv2->type != number_m) {
        avm_error("Not numbers in relational!");
        executionFinished = 1;
        return;
    }
    
    unsigned char result = rv1->data.numVal < rv2->data.numVal;
    
    if (!executionFinished && result)
        pc = instr->result.val;
}

void execute_jgt(instruction* instr) {
    assert(instr->result.type == label_a);
    
    avm_memcell* rv1 = avm_translate_operand(&instr->arg1, &ax);
    avm_memcell* rv2 = avm_translate_operand(&instr->arg2, &bx);
    
    assert(rv1 && rv2);
    
    if (rv1->type != number_m || rv2->type != number_m) {
        avm_error("Not numbers in relational!");
        executionFinished = 1;
        return;
    }
    
    unsigned char result = rv1->data.numVal > rv2->data.numVal;
    
    if (!executionFinished && result)
        pc = instr->result.val;
}

// Function operations
void execute_call(instruction* instr) {
    avm_memcell* func = avm_translate_operand(&instr->arg1, &ax);
    assert(func);
    avm_callsaveenvironment();
    
    switch (func->type) {
        case userfunc_m: {
            pc = userFuncs[func->data.funcVal].address;
            assert(pc < AVM_ENDING_PC && pc > 0);
            assert(code[pc].opcode == funcenter_v);
            break;
        }
        case string_m: {
            avm_calllibfunc(func->data.strVal);
            break;
        }
        case libfunc_m: {
            avm_calllibfunc(func->data.libfuncVal);
            break;
        }
        default: {
            char* s = strdup(avm_tostring(func));
            avm_error("Call: cannot bind '%s' to function!", s);
            free(s);
            executionFinished = 1;
        }
    }
}

void execute_pusharg(instruction* instr) {
    avm_memcell* arg = avm_translate_operand(&instr->arg1, &ax);
    assert(arg);
    
    avm_assign(&avm_stack[top], arg);
    ++avm_totalActuals;
    avm_dec_top();
}

void execute_funcenter(instruction* instr) {
    avm_memcell* func = avm_translate_operand(&instr->result, &ax);
    assert(func);
    assert(pc == userFuncs[func->data.funcVal].address);
    
    // Caller's enviroment
    avm_totalActuals = 0;
    userfunc* funcInfo = &userFuncs[func->data.funcVal];
    topsp = top;
    top = top - funcInfo->localSize;
}

void execute_funcexit(instruction* unused) {
    unsigned oldTop = top;
    top = avm_stack[topsp + AVM_SAVEDTOP_OFFSET].data.numVal;
    pc = avm_stack[topsp + AVM_SAVEDPC_OFFSET].data.numVal;
    topsp = avm_stack[topsp + AVM_SAVEDTOPSP_OFFSET].data.numVal;
    
    while (++oldTop <= top)
        avm_memcellclear(&avm_stack[oldTop]);
}

void execute_newtable(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    assert(lv && (((&avm_stack[AVM_STACKSIZE-1] >= lv) && (lv > &avm_stack[top])) || (lv == &retval)));
    
    avm_memcellclear(lv);
    lv->type = table_m;
    lv->data.tableVal = avm_tablenew();
    avm_tableincrefcounter(lv->data.tableVal);
}

void execute_tablegetelem(instruction* instr) {
    avm_memcell* lv = avm_translate_operand(&instr->result, (avm_memcell*)0);
    avm_memcell* t = avm_translate_operand(&instr->arg1, (avm_memcell*)0);
    avm_memcell* i = avm_translate_operand(&instr->arg2, &ax);
    
    assert(lv && ((&avm_stack[AVM_STACKSIZE-1] >= lv) && (lv > &avm_stack[top] || lv == &retval)));
    assert(t && ((&avm_stack[AVM_STACKSIZE-1] >= t) && (t > &avm_stack[top])));
    assert(i);
    
    avm_memcellclear(lv);
    lv->type = nil_m;
    
    if (t->type != table_m) {
        avm_error("Illegal use of type %s as table!", avm_tostring(t));
    } else {
        avm_memcell* content = avm_tablegetelem(t->data.tableVal, i);
        if (content)
            avm_assign(lv, content);
        else {
            char* ts = strdup(avm_tostring(t));
            char* is = strdup(avm_tostring(i));
            avm_warning("Table '%s' has no element '%s'!", ts, is);
            free(ts);
            free(is);
        }
    }
}

void execute_tablesetelem(instruction* instr) {
    avm_memcell* t = avm_translate_operand(&instr->arg1, (avm_memcell*)0);
    avm_memcell* i = avm_translate_operand(&instr->arg2, &ax);
    avm_memcell* c = avm_translate_operand(&instr->result, &bx);
    
    assert(t && ((&avm_stack[AVM_STACKSIZE-1] >= t) && (t > &avm_stack[top])));
    assert(i && c);
    
    if (t->type != table_m) {
        avm_error("Illegal use of type %s as table!", avm_tostring(t));
    } else {
        avm_tablesetelem(t->data.tableVal, i, c);
    }
}

void execute_jump(instruction* instr) {
    assert(instr->result.type == label_a);
    pc = instr->result.val;
}

void execute_nop(instruction* unused) {
    // No operation
}