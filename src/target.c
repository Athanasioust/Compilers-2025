#include "../include/target.h"
#include "../include/structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Global variables
instruction* instructions = NULL;
unsigned total_instructions = 0;
unsigned curr_instruction = 0;

incomplete_jump* ij_head = NULL;
unsigned ij_total = 0;

func_stack* funcstack = NULL;

// Constant arrays
double* numConsts = NULL;
unsigned totalNumConsts = 0;
unsigned currNumConst = 0;

char** stringConsts = NULL;
unsigned totalStringConsts = 0;
unsigned currStringConst = 0;

char** namedLibfuncs = NULL;
unsigned totalNamedLibfuncs = 0;
unsigned currNamedLibfunc = 0;

SymbolTableEntry** userFuncs = NULL;
unsigned totalUserFuncs = 0;
unsigned currUserFunc = 0;

// Typedef for generate functions
typedef void (*generator_func_t)(quad*);

// Generator dispatch table
generator_func_t generators[] = {
    generate_ASSIGN,
    generate_JUMP,
    generate_MUL,
    generate_UMINUS,
    generate_NOT,
    generate_IF_LESSEQ,
    generate_IF_GREATER,
    generate_RETURN,
    generate_FUNCEND,
    generate_TABLEGETELEM,
    generate_ADD,
    generate_DIV,
    generate_AND,
    generate_IF_EQ,
    generate_IF_GREATEREQ,
    generate_CALL,
    generate_GETRETVAL,
    generate_NEWTABLE,
    generate_TABLESETELEM,
    generate_SUB,
    generate_MOD,
    generate_OR,
    generate_IF_NOTEQ,
    generate_IF_LESS,
    generate_PARAM,
    generate_FUNCSTART
};

// Expand instruction array
void expand_instructions(void) {
    assert(total_instructions == curr_instruction);
    instruction* new_instructions = (instruction*)malloc(NEW_SIZE_INSTR);
    if (instructions) {
        memcpy(new_instructions, instructions, CURR_SIZE_INSTR);
        free(instructions);
    }
    instructions = new_instructions;
    total_instructions += EXPAND_SIZE_INSTR;
}

// Emit a new instruction
void emit_instruction(instruction* instr) {
    if (curr_instruction == total_instructions) {
        expand_instructions();
    }
    
    unsigned instr_num = curr_instruction; 
    instructions[curr_instruction] = *instr;
    curr_instruction++;
    
    
    incomplete_jump* ij = ij_head;
    while (ij && ij->instrNo == instr_num - 1) { 
        ij->instrNo = instr_num; 
        break;
    }
}

// Reset operand
void reset_operand(vmarg* operand) {
    operand->type = -1;
    operand->val = 0;
}

// Make operand from expression
void make_operand(Expr* e, vmarg* arg) {
    if (!e) {
        arg->type = nil_a;
        return;
    }
    switch (e->type) {
        case var_e:
        case tableitem_e:
        case arithmexpr_e:
        case boolexpr_e:
        case assignexpr_e:
        case newtable_e:
            if (!e->sym) {
                fprintf(stderr, "Error: Expression type %d has NULL symbol\n", e->type);
                fprintf(stderr, "This usually means a temporary wasn't properly created\n");
                exit(1);
            }
            arg->val = e->sym->offset;
            
            switch (e->sym->space) {
                case programvar:
                    arg->type = global_a;
                    break;
                case functionlocal:
                    arg->type = local_a;
                    break;
                case formalarg:
                    arg->type = formal_a;
                    break;
                default:
                    fprintf(stderr, "Error: Unknown symbol space %d\n", e->sym->space);
                    exit(1);
            }
            break;
            
        case constbool_e:
            arg->type = bool_a;
            arg->val = e->boolConst;
            break;
            
        case conststring_e:
            arg->type = string_a;
            if (e->strConst) {
                arg->val = consts_newstring(e->strConst);
            } else {
                arg->val = 0;  // or handle error
            }
            break;
            
        case constnum_e:
            arg->type = number_a;
            arg->val = consts_newnumber(e->numConst);
            break;
            
        case nil_e:
            arg->type = nil_a;
            break;
            
        case programfunc_e:
            arg->type = userfunc_a;
            arg->val = userfuncs_newfunc(e->sym);
            break;
            
        case libraryfunc_e:
            arg->type = libfunc_a;
            arg->val = libfuncs_newused(e->sym->name);
            break;
            
        default:
            assert(0);
    }
}

// Make number operand
void make_numberoperand(vmarg* arg, double val) {
    arg->type = number_a;
    arg->val = consts_newnumber(val);
}

// Make boolean operand
void make_booloperand(vmarg* arg, unsigned val) {
    arg->type = bool_a;
    arg->val = val;
}

// Make retval operand
void make_retvaloperand(vmarg* arg) {
    arg->type = retval_a;
}

// Constants management
unsigned consts_newstring(char* s) {
    // Check if string already exists
    for (unsigned i = 0; i < currStringConst; i++) {
        if (strcmp(stringConsts[i], s) == 0) {
            return i;
        }
    }
    
    // Add new string
    if (currStringConst == totalStringConsts) {
        totalStringConsts += 256;
        stringConsts = (char**)realloc(stringConsts, totalStringConsts * sizeof(char*));
    }
    
    stringConsts[currStringConst] = strdup(s);
    return currStringConst++;
}

unsigned consts_newnumber(double n) {
    // Check if number already exists
    for (unsigned i = 0; i < currNumConst; i++) {
        if (numConsts[i] == n) {
            return i;
        }
    }
    
    // Add new number
    if (currNumConst == totalNumConsts) {
        totalNumConsts += 256;
        numConsts = (double*)realloc(numConsts, totalNumConsts * sizeof(double));
    }
    
    numConsts[currNumConst] = n;
    return currNumConst++;
}

unsigned libfuncs_newused(char* s) {
    // Check if libfunc already exists
    for (unsigned i = 0; i < currNamedLibfunc; i++) {
        if (strcmp(namedLibfuncs[i], s) == 0) {
            return i;
        }
    }
    
    // Add new libfunc
    if (currNamedLibfunc == totalNamedLibfuncs) {
        totalNamedLibfuncs += 256;
        namedLibfuncs = (char**)realloc(namedLibfuncs, totalNamedLibfuncs * sizeof(char*));
    }
    
    namedLibfuncs[currNamedLibfunc] = strdup(s);
    return currNamedLibfunc++;
}

unsigned userfuncs_newfunc(SymbolTableEntry* sym) {
    
    // Check if user function already exists
    for (unsigned i = 0; i < currUserFunc; i++) {
        if (userFuncs[i] == sym) {
            return i;
        }
    }
    
    // Add new user function
    if (currUserFunc == totalUserFuncs) {
        totalUserFuncs += 256;
        userFuncs = (SymbolTableEntry**)realloc(userFuncs, totalUserFuncs * sizeof(SymbolTableEntry*));
    }
    
    userFuncs[currUserFunc] = sym;
    return currUserFunc++;
}

// Incomplete jumps
void add_incomplete_jump(unsigned instrNo, unsigned iaddress) {
    incomplete_jump* ij = (incomplete_jump*)malloc(sizeof(incomplete_jump));
    ij->instrNo = instrNo;
    ij->iaddress = iaddress;
    ij->next = ij_head;
    ij_head = ij;
    ij_total++;
}

void patch_incomplete_jumps(void) {
    incomplete_jump* ij = ij_head;
    while (ij) {
        unsigned target_address;
        
        if (ij->iaddress >= currQuad) {
            // Jump beyond program - go to end
            target_address = curr_instruction;
        } else if (ij->iaddress == 0) {
            // Jump to start
            target_address = 0;
        } else {
            // Normal case - get target quad's instruction address
            target_address = quads[ij->iaddress].taddress;
        }
        
        // Validate instruction number and target
        if (ij->instrNo < curr_instruction && target_address <= curr_instruction) {
            instructions[ij->instrNo].result.val = target_address;
        } else {
            fprintf(stderr, "Warning: Invalid jump patch - instr %d -> target %d\n", 
                    ij->instrNo, target_address);
        }
        
        incomplete_jump* del = ij;
        ij = ij->next;
        free(del);
    }
    ij_head = NULL;
    ij_total = 0;
}

// Function stack
void push_funcstack(SymbolTableEntry* f) {
    func_stack* fs = (func_stack*)malloc(sizeof(func_stack));
    fs->func = f;
    fs->returnList = NULL;
    fs->next = funcstack;
    funcstack = fs;
}

func_stack* pop_funcstack(void) {
    assert(funcstack);
    func_stack* fs = funcstack;
    funcstack = funcstack->next;
    return fs;
}

// Generate NOP
void generate_NOP(void) {
    instruction t;
    t.opcode = nop_v;
    reset_operand(&t.arg1);
    reset_operand(&t.arg2);
    reset_operand(&t.result);
    emit_instruction(&t);
}

void generate_relational(vmopcode op, quad* q) {
    instruction t;
    t.opcode = op;
    t.srcLine = q->line;
    
    make_operand(q->arg1, &t.arg1);
    make_operand(q->arg2, &t.arg2);
    t.result.type = label_a;
    
    
    t.result.val = 0;
    
    unsigned instr_num = curr_instruction;
    emit_instruction(&t);
    add_incomplete_jump(instr_num, q->label);
}

// Generate arithmetic operations
void generate_arithmetic(vmopcode op, quad* q) {
    instruction t;
    t.opcode = op;
    t.srcLine = q->line;
    
    make_operand(q->arg1, &t.arg1);
    make_operand(q->arg2, &t.arg2);
    make_operand(q->result, &t.result);
    
    emit_instruction(&t);
}

// Specific generators
void generate_ADD(quad* q) { generate_arithmetic(add_v, q); }
void generate_SUB(quad* q) { generate_arithmetic(sub_v, q); }
void generate_MUL(quad* q) { generate_arithmetic(mul_v, q); }
void generate_DIV(quad* q) { generate_arithmetic(div_v, q); }
void generate_MOD(quad* q) { generate_arithmetic(mod_v, q); }

void generate_UMINUS(quad* q) {
    instruction t;
    t.opcode = uminus_v;
    t.srcLine = q->line;
    
    make_operand(q->arg1, &t.arg1);
    reset_operand(&t.arg2);
    make_operand(q->result, &t.result);
    
    emit_instruction(&t);
}

void generate_NEWTABLE(quad* q) {
    instruction t;
    t.opcode = newtable_v;
    t.srcLine = q->line;
    
    reset_operand(&t.arg1);
    reset_operand(&t.arg2);
    make_operand(q->result, &t.result);
    
    emit_instruction(&t);
}

void generate_TABLEGETELEM(quad* q) {
    instruction t;
    t.opcode = tablegetelem_v;
    t.srcLine = q->line;
    
    make_operand(q->arg1, &t.arg1);
    make_operand(q->arg2, &t.arg2);
    make_operand(q->result, &t.result);
    
    emit_instruction(&t);
}

void generate_TABLESETELEM(quad* q) {
    instruction t;
    t.opcode = tablesetelem_v;
    t.srcLine = q->line;
    
    make_operand(q->arg1, &t.arg1);
    make_operand(q->arg2, &t.arg2);
    make_operand(q->result, &t.result);
    
    emit_instruction(&t);
}

void generate_ASSIGN(quad* q) {
    instruction t;
    t.opcode = assign_v;
    t.srcLine = q->line;
    
    make_operand(q->arg1, &t.arg1);
    reset_operand(&t.arg2);
    make_operand(q->result, &t.result);
    
    emit_instruction(&t);
}

void generate_JUMP(quad* q) {
    instruction t;
    t.opcode = jump_v;
    t.srcLine = q->line;
    
    reset_operand(&t.arg1);
    reset_operand(&t.arg2);
    t.result.type = label_a;
    
    
    t.result.val = 0;
    
    unsigned instr_num = curr_instruction;
    emit_instruction(&t);
    add_incomplete_jump(instr_num, q->label);
}



void generate_IF_EQ(quad* q) { generate_relational(jeq_v, q); }
void generate_IF_NOTEQ(quad* q) { generate_relational(jne_v, q); }
void generate_IF_GREATER(quad* q) { generate_relational(jgt_v, q); }
void generate_IF_GREATEREQ(quad* q) { generate_relational(jge_v, q); }
void generate_IF_LESS(quad* q) { generate_relational(jlt_v, q); }
void generate_IF_LESSEQ(quad* q) { generate_relational(jle_v, q); }

// Logical operations
void generate_NOT(quad* q) {
    // jeq arg1, false, +3
    instruction t1;
    t1.opcode = jeq_v;
    t1.srcLine = q->line;
    make_operand(q->arg1, &t1.arg1);
    make_booloperand(&t1.arg2, 0);
    t1.result.type = label_a;
    t1.result.val = curr_instruction + 3;
    emit_instruction(&t1);
    
    // assign false, result
    instruction t2;
    t2.opcode = assign_v;
    t2.srcLine = q->line;
    make_booloperand(&t2.arg1, 0);
    reset_operand(&t2.arg2);
    make_operand(q->result, &t2.result);
    emit_instruction(&t2);
    
    // jump +2
    instruction t3;
    t3.opcode = jump_v;
    t3.srcLine = q->line;
    reset_operand(&t3.arg1);
    reset_operand(&t3.arg2);
    t3.result.type = label_a;
    t3.result.val = curr_instruction + 2;
    emit_instruction(&t3);
    
    // assign true, result
    instruction t4;
    t4.opcode = assign_v;
    t4.srcLine = q->line;
    make_booloperand(&t4.arg1, 1);
    reset_operand(&t4.arg2);
    make_operand(q->result, &t4.result);
    emit_instruction(&t4);
}

void generate_OR(quad* q) {
    // jeq arg1, true, +4
    instruction t1;
    t1.opcode = jeq_v;
    t1.srcLine = q->line;
    make_operand(q->arg1, &t1.arg1);
    make_booloperand(&t1.arg2, 1);
    t1.result.type = label_a;
    t1.result.val = curr_instruction + 4;
    emit_instruction(&t1);
    
    // jeq arg2, true, +3
    instruction t2;
    t2.opcode = jeq_v;
    t2.srcLine = q->line;
    make_operand(q->arg2, &t2.arg1);
    make_booloperand(&t2.arg2, 1);
    t2.result.type = label_a;
    t2.result.val = curr_instruction + 3;
    emit_instruction(&t2);
    
    // assign false, result
    instruction t3;
    t3.opcode = assign_v;
    t3.srcLine = q->line;
    make_booloperand(&t3.arg1, 0);
    reset_operand(&t3.arg2);
    make_operand(q->result, &t3.result);
    emit_instruction(&t3);
    
    // jump +2
    instruction t4;
    t4.opcode = jump_v;
    t4.srcLine = q->line;
    reset_operand(&t4.arg1);
    reset_operand(&t4.arg2);
    t4.result.type = label_a;
    t4.result.val = curr_instruction + 2;
    emit_instruction(&t4);
    
    // assign true, result
    instruction t5;
    t5.opcode = assign_v;
    t5.srcLine = q->line;
    make_booloperand(&t5.arg1, 1);
    reset_operand(&t5.arg2);
    make_operand(q->result, &t5.result);
    emit_instruction(&t5);
}


void generate_AND(quad* q) {
    // jeq arg1, false, +4
    instruction t1;
    t1.opcode = jeq_v;
    t1.srcLine = q->line;
    make_operand(q->arg1, &t1.arg1);
    make_booloperand(&t1.arg2, 0);
    t1.result.type = label_a;
    t1.result.val = curr_instruction + 4;
    emit_instruction(&t1);
    
    // jeq arg2, false, +3
    instruction t2;
    t2.opcode = jeq_v;
    t2.srcLine = q->line;
    make_operand(q->arg2, &t2.arg1);
    make_booloperand(&t2.arg2, 0);
    t2.result.type = label_a;
    t2.result.val = curr_instruction + 3;
    emit_instruction(&t2);
    
    // assign true, result
    instruction t3;
    t3.opcode = assign_v;
    t3.srcLine = q->line;
    make_booloperand(&t3.arg1, 1);
    reset_operand(&t3.arg2);
    make_operand(q->result, &t3.result);
    emit_instruction(&t3);
    
    // jump +2
    instruction t4;
    t4.opcode = jump_v;
    t4.srcLine = q->line;
    reset_operand(&t4.arg1);
    reset_operand(&t4.arg2);
    t4.result.type = label_a;
    t4.result.val = curr_instruction + 2;
    emit_instruction(&t4);
    
    // assign false, result
    instruction t5;
    t5.opcode = assign_v;
    t5.srcLine = q->line;
    make_booloperand(&t5.arg1, 0);
    reset_operand(&t5.arg2);
    make_operand(q->result, &t5.result);
    emit_instruction(&t5);
}

// Function-related operations
void generate_PARAM(quad* q) {
    instruction t;
    t.opcode = pusharg_v;
    t.srcLine = q->line;
    
    make_operand(q->arg1, &t.arg1);
    reset_operand(&t.arg2);
    reset_operand(&t.result);
    
    emit_instruction(&t);
}

void generate_CALL(quad* q) {
    instruction t;
    t.opcode = call_v;
    t.srcLine = q->line;
    
    make_operand(q->arg1, &t.arg1);
    reset_operand(&t.arg2);
    reset_operand(&t.result);
    
    emit_instruction(&t);
}

void generate_GETRETVAL(quad* q) {
    instruction t;
    t.opcode = assign_v;
    t.srcLine = q->line;
    
    make_retvaloperand(&t.arg1);
    reset_operand(&t.arg2);
    make_operand(q->result, &t.result);
    
    emit_instruction(&t);
}

void generate_FUNCSTART(quad* q) {
    SymbolTableEntry* f = q->result->sym;
    
   
    instruction jump_instr;
    jump_instr.opcode = jump_v;
    jump_instr.srcLine = q->line;
    reset_operand(&jump_instr.arg1);
    reset_operand(&jump_instr.arg2);
    jump_instr.result.type = label_a;
    jump_instr.result.val = 0; 
    
    
    unsigned funcend_quad = 0;
    for (unsigned i = currQuad; i < total; i++) {
        if (quads[i].op == funcend && quads[i].result->sym == f) {
            funcend_quad = i + 1; 
            break;
        }
    }
    
    if (funcend_quad < total) {
        add_incomplete_jump(curr_instruction, funcend_quad);
    }
    emit_instruction(&jump_instr);
    
    
    f->taddress = curr_instruction;
    q->taddress = curr_instruction;
    
    push_funcstack(f);
    
    instruction t;
    t.opcode = funcenter_v;
    t.srcLine = q->line;
    
    make_operand(q->result, &t.result);
    reset_operand(&t.arg1);
    reset_operand(&t.arg2);
    
    emit_instruction(&t);
}


void generate_FUNCEND(quad* q) {
    
    
    func_stack* fs = pop_funcstack();
    
    
    backpatch_returns(fs->returnList);
    
    
    q->taddress = curr_instruction;
    
    instruction t;
    t.opcode = funcexit_v;
    t.srcLine = q->line;
    
    make_operand(q->result, &t.result);
    reset_operand(&t.arg1);
    reset_operand(&t.arg2);
    
    emit_instruction(&t);
    
    free(fs);
}

void generate_RETURN(quad* q) {
    q->taddress = curr_instruction;
    
    instruction t;
    t.opcode = assign_v;
    t.srcLine = q->line;
    
    make_retvaloperand(&t.result);
    reset_operand(&t.arg2);
    
    if (q->result) {
        make_operand(q->result, &t.arg1);
    } else {
        reset_operand(&t.arg1);
    }
    
    emit_instruction(&t);
    
    // Add to return list AFTER getting correct instruction number
    if (funcstack) {
        incomplete_jump* ij = (incomplete_jump*)malloc(sizeof(incomplete_jump));
        ij->instrNo = curr_instruction; // This will be the jump instruction
        ij->iaddress = 0;
        ij->next = funcstack->returnList;  
        funcstack->returnList = ij;       
    }
    
    t.opcode = jump_v;
    reset_operand(&t.arg1);
    reset_operand(&t.arg2);
    reset_operand(&t.result);
    t.result.type = label_a;
    t.result.val = 0; // Will be patched by backpatch_returns
    
    emit_instruction(&t);
}

void backpatch_returns(incomplete_jump* returnList) {
    incomplete_jump* ij = returnList;
    while (ij) {
        instructions[ij->instrNo].result.val = curr_instruction;
        incomplete_jump* del = ij;
        ij = ij->next;
        free(del);
    }
}

// Main generation function
void generate(void) {
    for (unsigned i = 1; i < currQuad; i++) {
        quads[i].taddress = curr_instruction;
        
        if (quads[i].op < 0 || quads[i].op >= sizeof(generators)/sizeof(generators[0])) {
            fprintf(stderr, "Error: Invalid opcode %d at quad %d\n", quads[i].op, i);
            exit(1);
        }
        
        generators[quads[i].op](&quads[i]);
    }
    patch_incomplete_jumps();
    print_const_tables();
}

void print_vmarg_to_string(vmarg arg, char* buffer, size_t size) {
    switch (arg.type) {
        case label_a:       snprintf(buffer, size, "%u", arg.val);; break;
        case global_a:     snprintf(buffer, size, "global(%u)", arg.val); break;
        case local_a:      snprintf(buffer, size, "local(%u)", arg.val); break;
        case formal_a:     snprintf(buffer, size, "formal(%u)", arg.val); break;
        case bool_a:       snprintf(buffer, size, "bool(%u)", arg.val); break;
        case number_a:     snprintf(buffer, size, "num(%u)", arg.val); break;
        case string_a:     snprintf(buffer, size, "str(%u)", arg.val); break;
        case nil_a:        snprintf(buffer, size, "nil"); break;
        case userfunc_a:   snprintf(buffer, size, "ufunc(%u)", arg.val); break;
        case libfunc_a:    snprintf(buffer, size, "libfunc(%u)", arg.val); break;
        case retval_a:     snprintf(buffer, size, "retval"); break;
        case -1:           snprintf(buffer, size, "empty"); break;
        default:           snprintf(buffer, size, "unknown"); break;
    }
}

void print_target_code(void) {
    printf("\n\nTarget Code:\n");
    printf("%-8s %-15s %-20s %-20s %-20s\n", "instr#", "opcode", "result", "arg1", "arg2");
    printf("----------------------------------------------------------------------------------------\n");

    for (unsigned i = 0; i < curr_instruction; i++) {
        char resultStr[32], arg1Str[32], arg2Str[32];

        print_vmarg_to_string(instructions[i].result, resultStr, sizeof(resultStr));
        print_vmarg_to_string(instructions[i].arg1, arg1Str, sizeof(arg1Str));
        print_vmarg_to_string(instructions[i].arg2, arg2Str, sizeof(arg2Str));

        printf("%-8u ", i);

        // Print opcode as string
        switch (instructions[i].opcode) {
            case assign_v:        printf("%-15s", "assign"); break;
            case add_v:           printf("%-15s", "add"); break;
            case sub_v:           printf("%-15s", "sub"); break;
            case mul_v:           printf("%-15s", "mul"); break;
            case div_v:           printf("%-15s", "div"); break;
            case mod_v:           printf("%-15s", "mod"); break;
            case uminus_v:        printf("%-15s", "uminus"); break;
            case and_v:           printf("%-15s", "and"); break;
            case or_v:            printf("%-15s", "or"); break;
            case not_v:           printf("%-15s", "not"); break;
            case jeq_v:           printf("%-15s", "jeq"); break;
            case jne_v:           printf("%-15s", "jne"); break;
            case jle_v:           printf("%-15s", "jle"); break;
            case jge_v:           printf("%-15s", "jge"); break;
            case jlt_v:           printf("%-15s", "jlt"); break;
            case jgt_v:           printf("%-15s", "jgt"); break;
            case call_v:          printf("%-15s", "call"); break;
            case pusharg_v:       printf("%-15s", "pusharg"); break;
            case funcenter_v:     printf("%-15s", "funcenter"); break;
            case funcexit_v:      printf("%-15s", "funcexit"); break;
            case newtable_v:      printf("%-15s", "newtable"); break;
            case tablegetelem_v:  printf("%-15s", "tablegetelem"); break;
            case tablesetelem_v:  printf("%-15s", "tablesetelem"); break;
            case jump_v:          printf("%-15s", "jump"); break;
            case nop_v:           printf("%-15s", "nop"); break;
            default:              printf("%-15s", "UNKNOWN"); break;
        }

        // Print the aligned operands
        printf("%-20s %-20s %-20s\n", resultStr, arg1Str, arg2Str);
    }
}


// Binary file output
void print_binary_file(const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open binary file %s\n", filename);
        return;
    }
    
    // Write magic number
    unsigned magic = 0xA1FA;
    fwrite(&magic, sizeof(unsigned), 1, file);
    
    // Write arrays
    fwrite(&currStringConst, sizeof(unsigned), 1, file);
    for (unsigned i = 0; i < currStringConst; i++) {
        unsigned len = strlen(stringConsts[i]) + 1;
        fwrite(&len, sizeof(unsigned), 1, file);
        fwrite(stringConsts[i], sizeof(char), len, file);
    }
    
    fwrite(&currNumConst, sizeof(unsigned), 1, file);
    fwrite(numConsts, sizeof(double), currNumConst, file);
    
    fwrite(&currNamedLibfunc, sizeof(unsigned), 1, file);
    for (unsigned i = 0; i < currNamedLibfunc; i++) {
        unsigned len = strlen(namedLibfuncs[i]) + 1;
        fwrite(&len, sizeof(unsigned), 1, file);
        fwrite(namedLibfuncs[i], sizeof(char), len, file);
    }
    
    fwrite(&currUserFunc, sizeof(unsigned), 1, file);
    for (unsigned i = 0; i < currUserFunc; i++) {
       unsigned address = userFuncs[i]->taddress;
       unsigned localSize = userFuncs[i]->totalLocals;
       unsigned len = strlen(userFuncs[i]->name) + 1;
       
       fwrite(&address, sizeof(unsigned), 1, file);
       fwrite(&localSize, sizeof(unsigned), 1, file);
       fwrite(&len, sizeof(unsigned), 1, file);
       fwrite(userFuncs[i]->name, sizeof(char), len, file);
   }
   
   // Write instructions
   fwrite(&curr_instruction, sizeof(unsigned), 1, file);
   fwrite(instructions, sizeof(instruction), curr_instruction, file);
   
   fclose(file);
}

// Συνάρτηση για εκτύπωση όλων των πινάκων σταθερών
void print_const_tables(void) {
    unsigned i;
    
    printf("\n========== CONSTANT TABLES ==========\n");
    
    // Εκτύπωση πίνακα αριθμών
    printf("\n--- NUMBER CONSTANTS ---\n");
    printf("Total: %u\n", currNumConst);
    for (i = 0; i < currNumConst; i++) {
        printf("[%3u] %.6f\n", i, numConsts[i]);
    }
    
    // Εκτύπωση πίνακα strings
    printf("\n--- STRING CONSTANTS ---\n");
    printf("Total: %u\n", currStringConst);
    for (i = 0; i < currStringConst; i++) {
        printf("[%3u] \"%s\"\n", i, stringConsts[i]);
    }
    
    // Εκτύπωση πίνακα library functions
    printf("\n--- LIBRARY FUNCTIONS USED ---\n");
    printf("Total: %u\n", currNamedLibfunc);
    for (i = 0; i < currNamedLibfunc; i++) {
        printf("[%3u] %s\n", i, namedLibfuncs[i]);
    }
    
    // Εκτύπωση πίνακα user functions
    printf("\n--- USER FUNCTIONS ---\n");
    printf("Total: %u\n", currUserFunc);
    for (i = 0; i < currUserFunc; i++) {
        printf("[%3u] %s (address: %u, locals: %u)\n", 
               i, 
               userFuncs[i]->name, 
               userFuncs[i]->taddress,
               userFuncs[i]->totalLocals);
    }
    
    printf("\n=====================================\n");
}

// Προσθήκη στο target.h
void print_const_tables(void);