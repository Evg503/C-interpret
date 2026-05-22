#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "parser.h"

// Символьная таблица
typedef struct {
    char* name;
    int value;
} Symbol;

// Функции интерпретатора
void init_interpreter(void);
int interpret_ast(ASTNode* node);
void set_variable(const char* name, int value);
int get_variable(const char* name);
void print_symbol_table(void);

#endif // INTERPRETER_H