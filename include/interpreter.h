#pragma once
#include "parser.h"


typedef struct {
    char* name;
    int value;
} Symbol;

int interpret_ast(ASTNode* node);
int get_variable(const char* name);
void set_variable(const char* name, int value);