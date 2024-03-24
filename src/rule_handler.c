#include "../include/rule_handler.h"
#include "../include/symbol_table.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int isVar(SymbolTableEntry* temp){
    return temp->type == FORMAL || temp->type == GLOBAL || temp->type == LOCAL;
}

Variable* makeVar(char* key, int lineno, int scope){
    Variable* temp = malloc(sizeof(struct Variable));
    temp->name = malloc(sizeof(key) + 1);
    strcpy(temp->name, key);
    temp->line = lineno;
    temp->scope = scope;

    return temp;
}

Function* makeFunc(char* key, int lineno, int scope){
    Function* temp = malloc(sizeof(struct Function));
    temp->name = malloc(sizeof(key) + 1);
    strcpy(temp->name, key);
    temp->line = lineno;
    temp->scope = scope;

	return temp;
}

// Function to insert a library function token into the symbol table
void insertLibFunction(const char* name) {
    // Create a Function token for the library function
    Function* func = makeFunc(name, 0, 0); // Line number and scope are set to 0 for library functions

    // Create a SymbolTableEntry for the function
    SymbolTableEntry entry;
    entry.isActive = true; // Set the entry as active
    entry.type = LIBFUNC; // Set the type of symbol as LIBFUNC
    entry.value.funcVal = func; // Assign the function to the value field of the entry

    // Insert the token into the symbol table
    insert(entry);
}
