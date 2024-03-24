#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define TABLE_SIZE 10000

SymbolTableEntry symbolTable[TABLE_SIZE]; // Declare the symbol table

typedef struct Variable{
    char* name;
    unsigned int scope;
    unsigned int line;
} Variable;

typedef struct Function{
    const char* name;
    unsigned int scope;
    unsigned int line;
    // unsigned int num_args;
    // char** args;
} Function;

enum SymbolType{
    GLOBAL, LOCAL, FORMAL, USERFUNC, LIBFUNC
};

typedef struct SymbolTableEntry{
    bool isActive;
    union{
        Variable* varVal;
        Function* funcVal;
    } value;
    enum SymbolType type;
} SymbolTableEntry;


SymbolTableEntry symbolTable[TABLE_SIZE];

// Hash function to calculate index
unsigned int hash(const char *name) {
    unsigned int hash = 0;
    while (*name) {
        hash = (hash << 5) + *name++;
    }
    return hash % TABLE_SIZE;
}

// Function to insert an entry into the symbol table
void insert(SymbolTableEntry entry) {
    unsigned int index = hash(entry.value.varVal->name);
    while (symbolTable[index].isActive) {
        index = (index + 1) % TABLE_SIZE;
    }
    symbolTable[index] = entry;
}

// General lookup function to search for a variable in all scopes
SymbolTableEntry* lookup(const char* name) {
    unsigned int index = hash(name);
    unsigned int originalIndex = index;
    do {
        if (symbolTable[index].isActive &&
            strcmp(symbolTable[index].value.varVal->name, name) == 0) {
            return &symbolTable[index];
        }
        index = (index + 1) % TABLE_SIZE;
    } while (index != originalIndex);
    return NULL; // Not found
}

// Scope-specific lookup function to search for a variable in a specific scope
SymbolTableEntry* lookupScope(const char* name, unsigned int scope) {
    unsigned int index = hash(name);
    unsigned int originalIndex = index;
    do {
        if (symbolTable[index].isActive &&
            strcmp(symbolTable[index].value.varVal->name, name) == 0 &&
            symbolTable[index].value.varVal->scope == scope) {
            return &symbolTable[index];
        }
        index = (index + 1) % TABLE_SIZE;
    } while (index != originalIndex);
    return NULL; // Not found
}

// Function to hide all variables for the specified scope
void hide(unsigned int scope) {
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (symbolTable[i].isActive &&
            symbolTable[i].value.varVal->scope == scope) {
            symbolTable[i].isActive = false;
        }
    }
}

// Function to print all contents of the symbol table
void printSymbolTable() {
    printf("Symbol Table:\n");
    printf("Index\t| Name\t| Scope\t| Line\t| Type\n");
    printf("--------------------------------------------\n");
    for (int i = 0; i < TABLE_SIZE; ++i) {
        if (symbolTable[i].isActive) {
            if (symbolTable[i].type == GLOBAL || symbolTable[i].type == LOCAL || symbolTable[i].type == FORMAL) {
                printf("%d\t| %s\t| %d\t| %d\t| Variable\n", i, symbolTable[i].value.varVal->name, symbolTable[i].value.varVal->scope, symbolTable[i].value.varVal->line);
            }
            else if (symbolTable[i].type == USERFUNC || symbolTable[i].type == LIBFUNC) {
                printf("%d\t| %s\t| %d\t| %d\t| Function\n", i, symbolTable[i].value.funcVal->name, symbolTable[i].value.funcVal->scope, symbolTable[i].value.funcVal->line);
            }
        }
    }
    printf("--------------------------------------------\n");
}
