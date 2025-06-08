%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include <assert.h>
	#include <stdbool.h>
	#include "../include/symtable.h"
	#include "../include/structs.h"
	#include "../include/stack.h"
	#include "../include/rule_handler.h"
	#include "../include/parser.h"
    #include "../include/target.h"

    int yyerror(char* message);
    int yylex(void);
    
    extern int yylineno;
    extern char* yytext;
    extern FILE* yyin;
%}

// Declarations

%start program

%union {
    unsigned labelval;
    double nval;
    char* sval;
	unsigned int func_addr;
	char* lib_addr;
    struct ForLoopPrefix* forprefixval;
    struct SymbolTableEntry* symval;
    struct Expr* exprval;
    struct Call* callval;
	struct stmt_t* stmtval;
}

%token<nval> NUM
%token<sval> IDENT STRING IF ELSE WHILE FOR FUNCTION RETURN BREAK
             CONTINUE AND NOT OR LOCAL TRUE FALSE NIL ASSIGN PLUS
             MINUS MUL DIV MOD EQUAL NEQUAL INC DEC GT LT GET LET
             BRACKET_OPEN BRACKET_CLOSED SQUARE_OPEN SQUARE_CLOSED
             PAR_OPEN PAR_CLOSED SEMI_COLON COMMA COLON DOUBLE_COLON
             DOT DOUBLE_DOT UMINUS
%type<symval> funcdef funcprefix 
%type<sval> funcname
%type<func_addr> funcbody
%type<exprval> lvalue expr term assignexpr prim member call 
               objectdef const elist indexed indexedelem objectarg
%type<callval> callsuffix normcall methodcall 
%type<labelval> ifprefix elseprefix whileprefix whileargs M N
%type<forprefixval> forprefix
%type<stmtval> break continue stmt block stmts stmts_other ifstmt

%right ASSIGN
%left OR
%left AND
%nonassoc EQUAL NEQUAL
%nonassoc GT GET LT LET
%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE
%left PLUS MINUS
%left MUL DIV MOD
%right NOT INC DEC
%left DOT DOUBLE_DOT
%left SQUARE_OPEN SQUARE_CLOSED 
%left PAR_OPEN PAR_CLOSED
%left UMINUS

%%

program:        stmts ;


stmts:     stmts_other          { $$ = $1; }
                |                       { $$ = (stmt_t*) 0;}
                ;  

stmts_other: stmt                                 { $$ = $1; }
                | stmts_other stmt {  // Remove {resetTemp();}
                                            if($1) $$ = $1;
                                            else if($2) $$ = $2;
                                            else $$ = (stmt_t*) 0;
                                        }
                ;

stmt:      expr SEMI_COLON   { $$ = (stmt_t*) 0; makeBoolStmt($1);}
                | ifstmt                {$$ = $1;}
                | whilestmt             {$$ = (stmt_t*) 0;}
                | forstmt               {$$ = (stmt_t*) 0;}
                | returnstmt            {$$ = (stmt_t*) 0;}
                | continue              {$$ = $1;}
                | break                 {$$ = $1;}
                | block                 {$$ = $1;}
                | funcdef               {$$ = (stmt_t*) 0;}
                | SEMI_COLON            {$$ = (stmt_t*) 0;}
                ;

expr:     assignexpr                      {$$ = $1;}
                | expr PLUS expr    {$$ = HANDLE_ARITH_OP(add, $1, $3);}
                | expr MINUS expr   {$$ = HANDLE_ARITH_OP(sub, $1, $3);}
                | expr MUL expr     {$$ = HANDLE_ARITH_OP(mul, $1, $3);}
                | expr DIV expr     {$$ = HANDLE_ARITH_OP(mydiv, $1, $3);}
                | expr MOD expr     {$$ = HANDLE_ARITH_OP(mod, $1, $3);}
                | expr GT expr      {$$ = HANDLE_REL_OP(if_greater, $1, $3);}
                | expr GET expr     {$$ = HANDLE_REL_OP(if_geatereq, $1, $3);}
                | expr LT expr      {$$ = HANDLE_REL_OP(if_less, $1, $3);}
                | expr LET expr     {$$ = HANDLE_REL_OP(if_lesseq, $1, $3);}
                | expr EQUAL expr   {makeBoolStmt($1); makeBoolStmt($3); $$ = HANDLE_REL_OP(if_eq, $1, $3);}
                | expr NEQUAL expr  {makeBoolStmt($1); makeBoolStmt($3); $$ = HANDLE_REL_OP(if_noteq, $1, $3);}
                | expr AND M expr   {$$ = HANDLE_BOOL_OP(and, $1, $4, $3);}
                | expr OR M expr    {$$ = HANDLE_BOOL_OP(or, $1, $4, $3);}
                | term                          {$$ = $1;}
                ;

term:           PAR_OPEN expr PAR_CLOSED  {$$ = $2;}
                | MINUS expr              {$$ = HANDLE_TERM_TO_UMINUS_EXPR($2);} %prec UMINUS
                | NOT expr                {$$ = HANDLE_TERM_TO_NOT_EXPR($2);}
                | INC lvalue                    {$$ = HANDLE_TERM_TO_INC_LVALUE($2, yylineno);}
                | lvalue INC                    {$$ = HANDLE_TERM_TO_LVALUE_INC($1, yylineno);}
                | DEC lvalue                    {$$ = HANDLE_TERM_TO_DEC_LVALUE($2, yylineno);}
                | lvalue DEC                    {$$ = HANDLE_TERM_TO_LVALUE_DEC($1, yylineno);}
                | prim                          {$$ = $1;}
                ;

assignexpr:     lvalue ASSIGN expr        {makeBoolStmt($3); $$ = HANDLE_ASSIGNEXPR_TO_LVALUE_ASSIGN_EXPRESSION($1, $3, yylineno);};

prim:           lvalue                          {$$ = HANDLE_PRIM_TO_LVALUE($1, yylineno);}
                | call                          {$$ = $1;}
                | objectdef                     {$$ = $1;}
                | PAR_OPEN funcdef PAR_CLOSED   {$$ = HANDLE_PRIM_TO_FUNCDEF($2);}
                | const                         {$$ = $1;}
                ;   

lvalue:         IDENT                   {$$ = HANDLE_LVALUE_TO_IDENT($1, yylineno);}
                | LOCAL IDENT           {$$ = HANDLE_LVALUE_TO_LOCAL_IDENT($2, yylineno);}
                | DOUBLE_COLON IDENT    {$$ = HANDLE_LVALUE_TO_GLOBAL_IDENT($2, yylineno);}
                | member                {$$ = $1;}
                ;

member:         lvalue DOT IDENT                                {$$ = HANDLE_MEMBER_TO_LVALUE_DOT_IDENT($1, $3);}
                | lvalue SQUARE_OPEN expr SQUARE_CLOSED   {$$ = HANDLE_MEMBER_TO_LVALUE_SQUARE_EXPR($1, $3);}
                | call DOT IDENT                                {$$ = HANDLE_MEMBER_TO_LVALUE_DOT_IDENT($1, $3);}
                | call SQUARE_OPEN expr SQUARE_CLOSED     {$$ = HANDLE_MEMBER_TO_LVALUE_SQUARE_EXPR($1, $3);}        
                ;

call:           call PAR_OPEN elist PAR_CLOSED                              {$$ = HANDLE_CALL_ELIST($1, $3);}
                | lvalue callsuffix                                         {$$ = HANDLE_CALL_LVALUE_SUFFIX($1, $2);}
                | PAR_OPEN funcdef PAR_CLOSED PAR_OPEN elist PAR_CLOSED     {$$ = HANDLE_CALL_FUNCDEF_ELIST($2, $5);}
                ;

callsuffix:     normcall        {$$ = $1;}
                | methodcall    {$$ = $1;}
                ;

normcall:       PAR_OPEN elist PAR_CLOSED                      {$$ = HANDLE_NORMCALL($2);};

methodcall:     DOUBLE_DOT IDENT PAR_OPEN elist PAR_CLOSED     {$$ = HANDLE_METHODCALL($2, $4);};

elist:          expr                  {makeBoolStmt($1); $$ = $1; $$->next = NULL;}
                | expr COMMA elist    {$$ = HANDLE_ELIST_ADD($1, $3);}
				|                           {$$ = (Expr*) 0;}
                ;

objectdef:      SQUARE_OPEN objectarg SQUARE_CLOSED  {$$ = $2;};

objectarg:      elist       {$$ = HANDLE_OBJECTDEF_TO_ELIST($1);}
                | indexed   {$$ = HANDLE_OBJECTDEF_TO_INDEXED($1);}
                ;

indexed:        indexedelem                     {$$ = $1;}
                | indexedelem COMMA indexed     {$$ = HANDLE_INDEXED_ADD($1, $3);}
                ;

indexedelem:    BRACKET_OPEN expr COLON expr BRACKET_CLOSED {makeBoolStmt($4); $$ = HANDLE_INDEXELEM($2, $4);};

block:          BRACKET_OPEN {scope++; current_table = SymTable_next(current_table);} stmts BRACKET_CLOSED {scope--; SymTable_hide(current_table); current_table = SymTable_prev(current_table); $$ = $3;};

funcdef:        funcprefix
				{
					funcCounter++;
					loopCounterStack = stack_push(loopCounterStack, loopCounter);
					loopCounter = 0;
				}
				funcargs
				funcbody
				{
					$$ = HANDLE_FUNCDEF($1, $4, yylineno);
					funcCounter--;
					loopCounter = stack_top(loopCounterStack);
					loopCounterStack = stack_pop(loopCounterStack);
				};

funcbody:       block {
                    $$ = currScopeOffset();
                    exitScopeSpace();
                };

funcprefix:     FUNCTION funcname {$$ = HANDLE_FUNCPREFIX($2, yylineno);};

funcargs:       PAR_OPEN 
                {
                    scope++; 
                    current_table = SymTable_next(current_table);
                } 
                idlist 
                {
                    scope--;
                    current_table = SymTable_prev(current_table); 
                    functionScopeStack = stack_push(functionScopeStack, scope);
                } 
                PAR_CLOSED
                {
                    enterScopeSpace();
                    resetFunctionLocalOffset();
                };

funcname:       IDENT          {$$ = HANDLE_FUNCTION_WITH_NAME($1, yylineno);}
                |              {$$ = HANDLE_FUNCTION_WITHOUT_NAME(yylineno);}
                ;

const:          NUM         {$$ = newExprConstNum($1);}
                | STRING    {$$ = newExprConstString($1);}
                | NIL       {$$ = newExpr(nil_e);}
                | TRUE      {$$ = newExprConstBool(1);}
                | FALSE     {$$ = newExprConstBool(0);}
                ;

idlist:         IDENT                   {HANDLE_IDLIST_IDENT($1, yylineno);}
                | IDENT COMMA idlist    {HANDLE_IDLIST_IDENT($1, yylineno);} 
                |
                ;

ifprefix:       IF PAR_OPEN expr PAR_CLOSED   {makeBoolStmt($3); $$ = HANDLE_IFPREFIX($3);}

elseprefix:     ELSE                                {$$ = HANDLE_ELSEPREFIX(yylineno);}

ifstmt:         ifprefix stmt {patchLabel($1, nextQuadLabel()); $$ = $2;} %prec LOWER_THAN_ELSE
                | ifprefix stmt elseprefix stmt {
                    patchLabel($1, $3 + 1);
                    patchLabel($3, nextQuadLabel());
                    $$ = $2;
                    if(!$$) $$ = $4;
                }
                ;

whileprefix:    WHILE                               {$$ = nextQuadLabel(); loopCounter++;};

whileargs:      PAR_OPEN expr PAR_CLOSED      {makeBoolStmt($2); $$ = HANDLE_WHILEARGS($2);}

whilestmt:      whileprefix whileargs stmt		{HANDLE_WHILE($1, $2, $3); loopCounter--;}

N:              {$$ = nextQuadLabel(); emit(jump, NULL, NULL, NULL, 0, yylineno);};
M:              {$$ = nextQuadLabel();};

forprefix:      FOR {loopCounter++;} PAR_OPEN elist SEMI_COLON M expr SEMI_COLON {makeBoolStmt($7); $$ = HANDLE_FORPREFIX($6, $7);};

forstmt:        forprefix N elist PAR_CLOSED N stmt N {HANDLE_FORSTMT($1, $2, $5, $7, $6); loopCounter--;};

returnstmt:		RETURN SEMI_COLON               {
                                                    if(funcCounter == 0) {
                                                        fprintf(stderr, "Line %d: Return used outside function", yylineno);
                                                        exit(1);
                                                    }
                                                    emit(ret, NULL, NULL, NULL, 0, yylineno);
                                                }
                | RETURN expr SEMI_COLON  {
                                                    makeBoolStmt($2);
                                                    if(funcCounter == 0) {
                                                        fprintf(stderr, "Line %d: Return used outside function", yylineno);
                                                        exit(1);
                                                    }
                                                    emit(ret, NULL, NULL, $2, 0, yylineno);
                                                }
                ;

break:			BREAK SEMI_COLON { 
                        if(loopCounter > 0) $$ = HANDLE_BREAK(); 
                        else {
                            fprintf(stderr, "Line %d: Break outside loop\n", yylineno);
                            exit(1);
                        }
                    }

continue:		CONTINUE SEMI_COLON {
                        if(loopCounter > 0) $$ = HANDLE_CONTINUE(); 
                        else {
                            fprintf(stderr, "Line %d: Continue outside loop\n", yylineno);
                            exit(1);
                        }
                }

%%

int yyerror(char *message){
    printf("Error on line %d: %s\n", yylineno, message);
    return -1;
}

int main(int argc, char **argv) {
    head = SymTable_new();
    current_table = head;

    // Reserve space for temporary variables (0-99)
    programVarOffset = 100;  // Start regular globals at offset 100

    // Add library functions
    libFunc(head, "print");
    libFunc(head, "input");
    libFunc(head, "objectmemberkeys");
    libFunc(head, "objecttotalmembers");
    libFunc(head, "objectcopy");
    libFunc(head, "totalarguments");
    libFunc(head, "argument");
    libFunc(head, "typeof");
    libFunc(head, "strtonum");
    libFunc(head, "sqrt");
    libFunc(head, "cos");
    libFunc(head, "sin");


    // Check for correct number of arguments
	if(argc > 3) {
		fprintf(stderr, "Invalid argument format\n");
		exit(0);
	}

    // Open input file
    if(argc == 1) {
		yyin = stdin;
    }
	else {
		if(!(yyin = fopen(argv[1], "r"))){
            fprintf(stderr, "Error opening input file %s\n", argv[1]);
            exit(0);
        }
	}

    // Open output file
	if(argc == 3 && !freopen(argv[2], "w", stdout)) {
		fprintf(stderr, "Error opening output file \n");
		exit(0);
	}
	printf("DEBUG: Starting yyparse\n");
    yyparse();
    printf("DEBUG: yyparse completed, calling printQuads\n");
    printQuads();
    printf("DEBUG: printQuads completed, calling generate\n");
    generate();
    printf("DEBUG: generate completed\n");
    print_target_code();
    print_binary_file("binary.abc");
    return 0;
}