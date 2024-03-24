%{
    #include <stdlib.h>
    #include <stdio.h>
	#include <string.h>
	#include <assert.h>
	#include <stdbool.h>
    #include "symbol_table.h"
    #include "rule_handler.h"



    int yyerror(char* message);
    int yylex(void);
    
    extern int yylineno;
    extern char* yytext;
    extern FILE* yyin;
%}

%start program

%union {
    double nval;
    char* sval;
    struct SymbolTableEntry* expressionval;
}

%token<nval> NUM
%token<sval> IDENT STRING IF ELSE WHILE FOR FUNCTION RETURN BREAK
             CONTINUE AND NOT OR LOCAL TRUE FALSE NIL ASSIGN PLUS
             MINUS MUL DIV MOD EQUAL NEQUAL INC DEC GT LT GET LET
             CURLY_OPEN CURLY_CLOSED SQUARE_OPEN SQUARE_CLOSED
             PAR_OPEN PAR_CLOSED SEMI_COLON COMMA COLON DOUBLE_COLON
             DOT DOUBLE_DOT UMINUS
%type<sval> funcname
%type<expressionval> lvalue


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

program:        statements ;

statements:     statements statement 
                |
                ;

statement:    expression SEMI_COLON
                |ifstatement
                |whilestatement
                |forstatement
                |returnstatement
                |BREAK SEMI_COLON
                |CONTINUE SEMI_COLON
                |block
                |funcdefinition
                |SEMI_COLON
                ;

expression:           assignexpression
                |expression op expression
                |term

op :            PLUS
                |MINUS
                |MUL
                |DIV
                |MOD
                |EQUAL
                |NEQUAL
                |GT
                |GET
                |LT
                |LET
                |AND
                |OR
                ;

term:           PAR_OPEN expression PAR_CLOSED
                | UMINUS expression
                | NOT expression
                | INC lvalue            
                | lvalue INC            
                | DEC lvalue            
                | lvalue DEC            
                | primary
                ;

assignexpression:     lvalue ASSIGN expression

primary:       lvalue
                |call
                |objectdefinition
                |PAR_OPEN funcdefinition PAR_CLOSED 
                |constant
                ;

lvalue:         IDENT                   
                | LOCAL IDENT           
                | DOUBLE_COLON IDENT    
                | member                
                ;

member:         lvalue DOT IDENT
                | lvalue SQUARE_OPEN expression SQUARE_CLOSED
                | call DOT IDENT
                | call SQUARE_OPEN expression SQUARE_CLOSED
                ;   

call:           call PAR_OPEN elist PAR_CLOSED
                | lvalue callsuffix     
                | PAR_OPEN funcdefinition PAR_CLOSED PAR_OPEN elist PAR_CLOSED
                ;

callsuffix:     normcall
                | methodcall
                ;

normcall:       PAR_OPEN elist PAR_CLOSED;

methodcall:     DOUBLE_DOT IDENT PAR_OPEN elist PAR_CLOSED ;

elist:          expression elist_other
                |
                ;

elist_other:      COMMA expression elist_other
                | 
                ;

objectdefinition:      SQUARE_OPEN expression SQUARE_CLOSED
                | SQUARE_OPEN expression COMMA elist SQUARE_CLOSED
                | SQUARE_OPEN indexedelem SQUARE_CLOSED
                | SQUARE_OPEN indexedelem COMMA indexed SQUARE_CLOSED
                | SQUARE_OPEN SQUARE_CLOSED
                ;

indexed:        indexedelem indexed_other
                |
                ;

indexed_other:    COMMA indexedelem indexed_other
                |
                ;

indexedelem:    CURLY_OPEN expression COLON expression CURLY_CLOSED ;

block:          CURLY_OPEN statements CURLY_CLOSED ;

funcdefinition:        funcname PAR_OPEN  block PAR_CLOSED ;

constant :         NUM
                | STRING
                | TRUE
                | FALSE
                | NIL
                ;

idlist:         IDENT idlist_other        
                |
                ;

idlist_other:     COMMA IDENT idlist_other  
                |
                ;

ifstatement:         IF PAR_OPEN expression PAR_CLOSED statement %prec LOWER_THAN_ELSE
                | IF PAR_OPEN expression PAR_CLOSED statement ELSE statement
                ;

whilestatement:      WHILE PAR_OPEN expression PAR_CLOSED statement       

forstatement:        FOR PAR_OPEN elist SEMI_COLON expression SEMI_COLON elist PAR_CLOSED statement ;

returnstatement:     RETURN SEMI_COLON
                | RETURN expression SEMI_COLON
                ;


%%


int yyerror(char* message) {
    fprintf(stderr, "Error: %s at line %d\n", message, yylineno);
    return 1;
}


int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input file>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");
    if (yyin == NULL) {
        fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
        return 1;
    }

    insertLibFunction("print");
    insertLibFunction("input");
    insertLibFunction("objectmemberkeys");
    insertLibFunction("objecttotalmembers");
    insertLibFunction("objectcopy");
    insertLibFunction("totalarguments");
    insertLibFunction("argument");
    insertLibFunction("typeof");
    insertLibFunction("strtonum");
    insertLibFunction("sqrt");
    insertLibFunction("cos");
    insertLibFunction("sin");
    
    yyparse();
    symbolTable.printSymbolTable();
    fclose(yyin);
    return 0;
}


                