%{
/***Authors:Stelios Athanasiou 4581
            Yiannis Gonianakis 4863
***/
#define YY_DECL int alpha_yylex(void* ylval)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// The alpha token struct 
typedef struct alpha_token_t {
  unsigned int     numline;
  unsigned int     numToken;
  char          *raw_content;
  char          *content;
  char          *type;
  struct alpha_token_t *next;
  struct alpha_token_t *prev;
}alpha_token_t;

// Function to create a token node
alpha_token_t* createToken(int , int ,char*, char *, char *);

//Function to append a token node to the list 
void appendToken(alpha_token_t *);

//Function to empty the list after running the programm
void FreeFunct();

//Global pointer, the head of the list
alpha_token_t *head=NULL;

//Temporary node to save the token before its appended to the list
static alpha_token_t *temp=NULL;

//Global int variable, which counts the tokens
static int token_num=0;

//Function to parse the multiline comments
char *parseMulticomment();

// Enums and corresponding arrays for Keywords,Operators and Punctuation Marks 
char* Keywords_arr[16]=
{
  "IF",
  "ELSE",
  "WHILE",
  "FOR",
  "FUNCTION",
  "RETURN",
  "BREAK",
  "CONTINUE",
  "AND",
  "NOT",
  "OR",
  "LOCAL",
  "TRUE",
  "FALSE",
  "NIL"
};
char* Operators_arr[16] =
  {
    "ASSIGN",
    "PLUS",
    "MINUS",
    "MULTI",
    "DIV",
    "MOD",
    "EQUAL",
    "NEQUAL",
    "INC",
    "DEC",
    "GT",
    "LT",
    "GTE",
    "LTE"
  };
char* Punc_arr[16] =
  {
    "LAB",
    "RAB",
    "LB",
    "RB",
    "LP",
    "RP",
    "SEMI",
    "COMMA",
    "COLON",
    "NS",
    "DOT",
    "DD"
  };
enum punc_mark{
  LAB,
  RAB,
  LB,
  RB,
  LP,
  RP,
  SEMI,
  COMMA,
  COLON,
  NS,
  DOT,
  DD
};
enum keywords{
  IF,
  ELSE,
  WHILE,
  FOR,
  FUNCTION,
  RETURN,
  BREAK,
  CONTINUE,
  AND,
  NOT,
  OR,
  LOCAL,
  TRUE,
  FALSE,
  NIL
};
enum operators{
  ASSIGN,
  PLUS,
  MINUS,
  MULTI,
  DIV,
  MOD,
  EQUAL,
  NEQUAL,
  INC,
  DEC,
  GT,
  LT,
  GTE,
  LTE,
};
%}
%option noyywrap
%option yylineno


/*Regular expressions*/
keyword if|else|while|for|function|return|break|continue|and|not|or|local|true|false|nil
operator <|>|=|==|!=|>=|<=|\+|\+\+|--|-=|\+=|\/|\*|-|%
punc_mark \{|\}|\[|\]|\(|\)|;|,|:|::|\.|\.\.
id {letter}({letter}|{digit}|{underscore})*
integer {digit}+
delim [\r\n \t\v]
ws {delim}+
letter [a-zA-Z]
digit [0-9]
underscore [_]
float {digit}*\.{digit}*
string \"(\\.|[^\"])*\"
comment \/\/.*
multicomment \/\*(.|\n)*?\*\/
/* multicomment "/*"


/*** Rules Section ***/
%%
{keyword} {
  ++token_num;
  temp= createToken(yylineno,token_num,yytext,Keywords_arr[(!strcmp(yytext,"if"))?IF:(!strcmp(yytext,"else"))?ELSE:(!strcmp(yytext,"while"))?WHILE:(!strcmp(yytext,"for"))?FOR:(!strcmp(yytext,"function"))?FUNCTION:(!strcmp(yytext,"return"))?RETURN:(!strcmp(yytext,"break"))?BREAK:(!strcmp(yytext,"continue"))?CONTINUE:(!strcmp(yytext,"and"))?AND:(!strcmp(yytext,"not"))?NOT:(!strcmp(yytext,"or"))?OR:(!strcmp(yytext,"local"))?LOCAL:(!strcmp(yytext,"true"))?TRUE:(!strcmp(yytext,"false"))?FALSE:(!strcmp(yytext,"nil"))?NIL:-1],"KEYWORD");
 appendToken(temp);
}
{operator} { 
  ++token_num;
  temp=createToken(yylineno,token_num,yytext,Operators_arr[(!strcmp(yytext,"="))?ASSIGN:(!strcmp(yytext,"+"))?PLUS:(!strcmp(yytext,"-"))?MINUS:(!strcmp(yytext,"*"))?MULTI:(!strcmp(yytext,"/"))?DIV:(!strcmp(yytext,"%"))?MOD:(!strcmp(yytext,"=="))?EQUAL:(!strcmp(yytext,"!="))?NEQUAL:(!strcmp(yytext,"++"))?INC:(!strcmp(yytext,"--"))?DEC:(!strcmp(yytext,">"))?GT:(!strcmp(yytext,"<"))?LT:(!strcmp(yytext,">="))?GTE:(!strcmp(yytext,"<="))?LTE:-1],"OPERATOR");
 appendToken(temp);
}
{integer} {
  ++token_num;
  temp=createToken(yylineno,token_num,yytext,yytext,"INTEGER");
 appendToken(temp);
}
{float} { 
  ++token_num;
  temp=createToken(yylineno,token_num,yytext,yytext,"FLOAT");
 appendToken(temp);
}
{string} { 
  ++token_num;
  temp=createToken(yylineno,token_num,yytext,yytext,"STRING");
 appendToken(temp);
}
{punc_mark} { 
  ++token_num;
  temp=createToken(yylineno,token_num,yytext,Punc_arr[(!strcmp(yytext,"{"))?LAB:(!strcmp(yytext,"}"))?RAB:(!strcmp(yytext,"["))?LB:(!strcmp(yytext,"]"))?RB:(!strcmp(yytext,"("))?LP:(!strcmp(yytext,")"))?RP:(!strcmp(yytext,";"))?SEMI:(!strcmp(yytext,","))?COMMA:(!strcmp(yytext,":"))?COLON:(!strcmp(yytext,"::"))?NS:(!strcmp(yytext,"."))?DOT:(!strcmp(yytext,".."))?DD:-1],"PUNCT_MARK");
 appendToken(temp);
}
{id} {
  ++token_num;
  temp=createToken(yylineno,token_num,yytext,yytext,"IDENTIFIER");
 appendToken(temp);
}
{comment} {
  ++token_num;
  temp=createToken(yylineno,token_num,yytext,yytext,"COMMENT");
 appendToken(temp);
}
{multicomment} {
  ++token_num;
  printf("I AM HERE\n");
  //char *text = parseMulticomment();
  temp=createToken(yylineno,token_num,yytext,yytext,"MULTICOMMENT");
  appendToken(temp);
}
{ws} {
  ;
}
. {
  printf("Undefined input %s in line %d",yytext,yylineno);
  exit(1);
}
%%
/*** Code Section ***/
alpha_token_t* createToken(int numline, int numToken,char* raw_content, char *content, char *type){
  alpha_token_t *new = malloc(sizeof(alpha_token_t)); //memory allocation for new token
  new->numline = numline;
  new->numToken = numToken;
  new->raw_content= strdup(raw_content);
  new->content = strdup(content);
  new->type = type;
  new->next = NULL;
  new->prev = NULL;
  return new; //return the token after having stored all the information needed
}

void appendToken(alpha_token_t *new){
  if (token_num == 1) { //if the first token is passed as parameter, assign it to the head of the list
    head = new;
  }
  else {
    alpha_token_t *current = head; // 
    while(current->next){ // go to the last node of the list
      current = current -> next;
    }
    current->next = new; // update the last node's next pointer
    new->prev = current; // update the new node's prev pointer
  }
}
//a Print function, that takes as a parameter the stream at which we want to print the output
void Print(FILE *stream) { 
    printf("Lexicographical analysis of the given input is being processed...\n");
    fprintf(stream,"=======================Lexical Analysis==========================\n");
    alpha_token_t *current = head;
    while (current) {
        // Determine the width for each field
        int numline_width = 1; // Adjust as needed
        int numToken_width = 2; // Adjust as needed
        int raw_content_width = 20; // Adjust as needed
        int type_width = 10; // Adjust as needed
        int content_width = 10; // Adjust as needed
        
        // Print with aligned fields
        if(!strcmp(current->type,"STRING")) {
            fprintf(stream,"%-*d: #%-*d %-*s %-*s %-*s <-char*\n", numline_width, current->numline, numToken_width, current->numToken, raw_content_width, current->raw_content, type_width, current->type, content_width, current->content);
        } else if(!strcmp(current->type,"IDENTIFIER")) {
            fprintf(stream,"%-*d: #%-*d \"%-*s\" %-*s %-*s <-char*\n", numline_width, current->numline, numToken_width, current->numToken, raw_content_width, current->raw_content, type_width, current->type, content_width, current->content);
        
        } else if(!strcmp(current->type,"KEYWORD") || !strcmp(current->type,"OPERATOR") || !strcmp(current->type,"PUNCT_MARK") || !strcmp(current->type,"COMMENT")) {
            fprintf(stream,"%-*d: #%-*d \"%-*s\" %-*s %-*s <-enumerated\n", numline_width, current->numline, numToken_width, current->numToken, raw_content_width, current->raw_content, type_width, current->type, content_width, current->content);
  

        }else if(!strcmp(current->type,"MULTICOMMENT")) {
            fprintf(stream,"%-*d: #%-*d \"%-*s\" %-*s %-*s <-enumerated\n", numline_width, current->numline, numToken_width, current->numToken, raw_content_width, current->raw_content, type_width, current->type, content_width, current->content);
        
        
        } else if(!strcmp(current->type,"INTEGER")) {
            fprintf(stream,"%-*d: #%-*d \"%-*s\" %-*s %-*s <-integer\n", numline_width, current->numline, numToken_width, current->numToken, raw_content_width, current->raw_content, type_width, current->type, content_width, current->content);
        } else {
            fprintf(stream,"%-*d: #%-*d \"%-*s\" %-*s %-*s <-float\n", numline_width, current->numline, numToken_width, current->numToken, raw_content_width, current->raw_content, type_width, current->type, content_width, current->content);
        }
        current = current->next;
    }
}


//function to free the memory allocated for the tokens
void FreeFunct(){
  alpha_token_t *temp;
  alpha_token_t *current = head;
  while (current) {
    temp = current;
    current = current->next;
    free(temp);
  }
}

char *parseMulticomment() {
  int c;
  char *text;
  printf("I AM IN parseMULTICOMMENT\n");
  while((c = input()) != EOF) {
    if(c == '*') {
      if((c = input()) == '/') {
        text = strdup(yytext);
        return text;
      }
    }else{
      unput(c);
    }
    if(c =='/') {
      if((c = input()) == '*') {
        text = parseMulticomment();
      }else{
        unput(c);
      }
    }
    return text;
  }

}