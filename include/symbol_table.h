#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>

// Struct for variable
typedef struct Variable {
    char* name;
    unsigned int scope;
    unsigned int line;
} Variable;

// Struct for function
typedef struct Function {
    const char* name;
    unsigned int scope;
    unsigned int line;
    // unsigned int num_args;
    // char** args;
} Function;

// Enum for symbol types
enum SymbolType {
    GLOBAL, LOCAL, FORMAL, USERFUNC, LIBFUNC
};

// Struct for symbol table entry
typedef struct SymbolTableEntry {
    bool isActive;
    union {
        Variable* varVal;
        Function* funcVal;
    } value;
    enum SymbolType type;
} SymbolTableEntry;

// Function declarations
void insert(SymbolTableEntry entry);
SymbolTableEntry* lookup(const char* name);
SymbolTableEntry* lookupScope(const char* name, unsigned int scope);
void hide(unsigned int scope);
void printSymbolTable();

#endif /* SYMBOL_TABLE_H */
