#include "../include/structs.h"
#include "../include/symtable.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

unsigned	scope = 0;
unsigned	anon_count = 0;
quad*		quads = (void*)0; // quad vector
unsigned	total = 1;
unsigned	currQuad = 1;
static unsigned tempCounterPerScope[100] = {0}; // Support up to 100 nested scopes
static unsigned maxScope = 0;
unsigned tempVarOffset = 0;  // Separate counter for temporaries


// Expand the quad vector
void expand (void) {
    assert(total == currQuad);
    quad *p = malloc(NEW_SIZE);
    if (quads) {
        memcpy(p, quads, CURR_SIZE);
        free(quads);
    }
    quads = p;
    total += EXPAND_SIZE;
}

// Emit a quad
void emit(iopcode op, Expr* arg1, Expr* arg2, Expr* result, unsigned label, unsigned line) {
    
    
    
    if (currQuad == total) {
        expand();
    }
    
    quad* p = quads + currQuad++;
    p->op = op;
    p->arg1 = arg1;  // These should store the full Expr, including sym pointer
    p->arg2 = arg2;
    p->result = result;
    p->label = label;
    p->line = line;
    p->taddress = 0;
    
    // Check for heap corruption
    if (result && result->sym) {
        if (strlen(result->sym->name) > 100) { // Sanity check
            printf("ERROR: Corrupted symbol name detected!\n");
            exit(1);
        }
    }
}

unsigned programVarOffset = 0;
unsigned functionLocalOffset = 0;
unsigned formalArgOffset = 0;
unsigned scopeSpaceCounter = 1;
unsigned tempCounter = 0;
unsigned loopCounter = 0;
unsigned funcCounter = 0;

// makes a bool statement
void makeBoolStmt(Expr* e){
    
    if(e->type == boolexpr_e){
        patchList(e->trueList, nextQuadLabel());
        emit(assign, newExprConstBool(1), NULL, e, 0, 0);
        emit(jump, NULL, NULL, NULL, nextQuadLabel() + 2, 0);
        patchList(e->falseList, nextQuadLabel());
        emit(assign, newExprConstBool(0), NULL, e, 0, 0);
    }
    
}
// Return the current scope space
ScopeSpace currScopeSpace(void){
    if(scopeSpaceCounter == 1){
        return programvar;
    }

    if(scopeSpaceCounter % 2 == 0){
        return formalarg;
    }

    return functionlocal;
}

// Return the current scope offset
unsigned currScopeOffset (void) { 
    switch (currScopeSpace ()) {
        case programvar: 
            return programVarOffset; 
        case functionlocal:
            return functionLocalOffset;
        case formalarg: 
            return formalArgOffset;
        default:
            assert (0);
    }
}

// Increase the current scope offset
void incCurrScopeOffset(void){ 
    switch (currScopeSpace()) {
        case programvar: 
            ++programVarOffset; 
            break; 
        case functionlocal: 
            ++functionLocalOffset;
            break; 
        case formalarg: 
            ++formalArgOffset; 
            break; 
        default: 
            assert (0);
    }
}

// Exit the current scope space
void exitScopeSpace (void){ 
    assert(scopeSpaceCounter>1); 
    --scopeSpaceCounter; 
}

// Enter a new scope space
void enterScopeSpace (void){
    ++scopeSpaceCounter;
}

// Reset the formal arguments offset
void resetFormalArgsOffset(void){
    formalArgOffset = 0;
}

// Reset the program variables offset
void resetFunctionLocalOffset(void){
    functionLocalOffset = 0;
}

// Reset the temp counter
void resetTemp() {
    // Only reset if we're in a deeper scope than we've seen
    if (scope <= maxScope) {
        tempCounter = tempCounterPerScope[scope];
    }
}

// Restore the current scope offset
void restoreCurrScopeOffset(unsigned n){
    switch(currScopeSpace()){
        case programvar:    programVarOffset = n; break;
        case functionlocal: functionLocalOffset = n; break;
        case formalarg:     formalArgOffset = n; break;
        default:            assert(0);
    }
}

// set the label of a quad
void patchLabel(unsigned quadNo, unsigned label){
    assert(quadNo < currQuad);
    quads[quadNo].label = label;
}

// Return the next quad label
unsigned nextQuadLabel(void){
    return currQuad;
}

// Return a name for a new temporary symbol
char* newTempName(){
    int count = 0, num = tempCounter;
    
    if (num == 0) {
        count = 1;
    } else {
        while (num) {
            num /= 10;
            ++count;
        }
    }

    char* tempName = malloc(count + 3);
    sprintf(tempName, "_t%d", tempCounter++);
    
    // Update scope tracking
    if (scope > maxScope) maxScope = scope;
    tempCounterPerScope[scope] = tempCounter;
    
    return tempName;
}

// Return a new temporary symbol
SymbolTableEntry* newTemp(){
    char* name = newTempName();
    
    SymbolTableEntry* temp = makeSymbol(name, 0, scope);
    temp->type = (scope ? VAR_LOCAL : VAR_GLOBAL);
    temp->space = currScopeSpace();
    
    // Use separate offset for temporaries at global scope
    if (scope == 0) {
        temp->offset = tempVarOffset++;
    } else {
        temp->offset = currScopeOffset();
        incCurrScopeOffset();
    }
    
    temp->taddress = 0;
    temp->totalLocals = 0;
    temp->iadress = 0;
    
    // Insert into symbol table
    SymbolTableEntry* result = SymTable_insert(current_table, name, temp);
    if (!result) {
        fprintf(stderr, "Error: Failed to insert temporary into symbol table\n");
        exit(1);
    }
    
    return result;
}

// Return a new expression
Expr* newExpr(ExprType t){
    Expr* e = (Expr*) malloc(sizeof(Expr));
    memset(e, 0, sizeof(Expr));
    e->type = t;
    return e;
}

// Change the strConst of an expression
Expr* newExprConstString(char* s){
    Expr* e = newExpr(conststring_e);
    e->strConst = strdup(s);
    return e;
}

// Change the numConst of an expression
Expr* newExprConstNum(double i){
    Expr* e = newExpr(constnum_e);
    e->numConst = i;
    return e;
}

// Change the boolConst of an expression
Expr* newExprConstBool(unsigned char boolean){
    Expr* e = newExpr(constbool_e);
    e->boolConst = !!boolean;
    return e;
}

// create a new table item expression

SymbolTableEntry* makeSymbol(char* key, int lineno, int scope){
    
    SymbolTableEntry* temp = calloc(1, sizeof(SymbolTableEntry));
    temp->isActive = 1;
    temp->name = malloc(strlen(key) + 1);
    strcpy(temp->name, key);
    
    
    temp->line = lineno;
    temp->scope = scope;
    temp->returnList = 0;
    temp->taddress = 0;

    return temp;
}

// make a new statement
void make_stmt(stmt_t *s) {
	s->breakList = s->contList = 0;
}


int newList(int i ) {
    if (total == currQuad) {
        expand();
    }
	quads[i].label = 0;
	return i;
}

// merge two lists
int mergeList(int l1, int l2) {
	if(!l1) {
		return l2;
	}
	else if(!l2) {
		return l1;
	}
	else {
		int i = l1;
		while (quads[i].label) {
			i = quads[i].label;
		}
		quads[i].label = l2;
		return l1;
	}
	return 0; // dummy return to avoid warning
}
// patch the labels of a list
void patchList(int list, int label) {
	while (list) {
		int next = quads[list].label;
		quads[list].label = label;
		list = next;
	}
}

// check if an expression is legal arithmetical
void checkArith(Expr* e, const char* context){
    if (e->type == constbool_e ||
        e->type == conststring_e ||
        e->type == nil_e ||
        e->type == newtable_e ||
        e->type == programfunc_e ||
        e->type == libraryfunc_e ||
        e->type == boolexpr_e )
    fprintf(stderr,"Illegal expr used in %s!", context);
}

// check if an expression is legal boolean
int boolVal(Expr *e) {
    switch (e->type) {
        case constbool_e:
            return e->boolConst;
        case constnum_e:
            return e->numConst != 0;
        case conststring_e:
            return strlen(e->strConst) != 0;
        case nil_e:
            return 0;
        case tableitem_e:
            return 1;
        case programfunc_e:
            return 1;
        case libraryfunc_e:
            return 1;
        case newtable_e:
            return 1;
        default:
            assert(0);
    }
}

// get the value of an expression as a string
char* getStringValueQuad(Expr* e){
    switch(e->type){
        case conststring_e:
            return e->strConst;
        case constnum_e:{
            char* str = malloc(sizeof(char) * 32);
            sprintf(str, "%.6f", e->numConst);  // Changed from %.1f to %.6f
            return str;
        }
        case nil_e:
            return "nil";
        case tableitem_e:
            return e->sym->name;
        case programfunc_e:
            return e->sym->name;
        case libraryfunc_e:
            return e->sym->name;
        case boolexpr_e:
            return e->sym->name;
        case arithmexpr_e:{
            return e->sym->name;
        }
        case newtable_e:
            return e->sym->name;
        case constbool_e:
            return e->boolConst ? "true" : "false";
        case var_e:
            return e->sym->name;
        case assignexpr_e:
            return e->sym->name;
        default: assert(0);
    }
}


// return the opcode name of a quad
const char* str_iopcodeName[] = {
    "assign",
	"jump",
    "mul",
    "uminus",
    "not",
    "if_lesseq",
    "if_greater",
    "ret",
    "funcend",
    "tablegetelem",
    "add",
    "mydiv",
    "and",
    "if_eq",
    "if_geatereq",
    "call",
    "getretval",
    "tablecreate",
    "tablesetelem",
    "sub",
    "mod",
    "or",
    "if_noteq",
    "if_less",
    "param",
    "funcstart"
};

// check if a quad is a branch

int isBranch(iopcode op) {
	switch(op) {
		case jump:
		case if_lesseq:
		case if_greater:
		case if_eq:
		case if_geatereq:
		case if_noteq:
		case if_less:
			return 1;
		default:
			return 0;
	}
	return 0;
}
// return the opcode name of a quad
const char* iopcodeName(quad* q){
    return str_iopcodeName[q->op];
}

// print the quads
void printQuads(void) {
    printf("Quads:\n");
	char str_label[16] = {0};
	printf("%8s | %14s | %10s | %10s | %10s | %10s |\n", "quad#", "opcode", "result", "arg1", "arg2", "label");
    for (int i = 1; i < currQuad; i++) {
        quad* q = &quads[i];

		if (isBranch(q->op)) {
			sprintf(str_label, "%d", q->label);
		}
		else {
			strcpy(str_label, " ");
		}

        printf("%8d | %14s | %10s | %10s | %10s | %10s |\n",
        i,
        iopcodeName(q),
        (q->result != NULL ? getStringValueQuad(q->result) : " "),
        (q->arg1 != NULL ? getStringValueQuad(q->arg1) : " "),
        (q->arg2 != NULL ? getStringValueQuad(q->arg2) : " "),
        str_label);
    }
}