#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lex.yy.c"
int main(int argc, char *argv[]){
    FILE *fp;
    FILE *fp2;
    if ((fp = freopen(argv[1],"r",stdin)) == NULL){
       printf("Error! opening file1");
       // Program exits if the file pointer returns NULL.
       exit(1);
    }
    if (argc== 3){ //if there is a second argument
        if ((fp2 = fopen(argv[2],"w")) == NULL){ //open the file
            printf("Error! opening file2");
            // Program exits if the file pointer returns NULL.
            exit(1);
        }
    }
    //This calls the lexicographical analysis
    alpha_yylex(head);
    if(argc == 3)
        Print(fp2);
    else
        Print(stdout);
    FreeFunct();
    return 0;
}