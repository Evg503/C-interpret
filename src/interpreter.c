#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>


#include "lexer.h"
#include "interpreter.h"

Symbol sym_table[100];
int sym_count = 0;

int get_variable(const char* name) {
    for (int i = 0; i < sym_count; i++) {
        if (strcmp(sym_table[i].name, name) == 0)
            return sym_table[i].value;
    }
    return 0;
}

void set_variable(const char* name, int value) {
    for (int i = 0; i < sym_count; i++) {
        if (strcmp(sym_table[i].name, name) == 0) {
            sym_table[i].value = value;
            return;
        }
    }
    sym_table[sym_count].name = strdup(name);
    sym_table[sym_count].value = value;
    sym_count++;
}

// Интерпретация AST
int interpret_ast(ASTNode* node) {
    if (!node) return 0;
    
    switch (node->type) {
        case NODE_NUMBER:
            return node->number.value;
            
        case NODE_IDENTIFIER:
            return get_variable(node->identifier.name);
            
        case NODE_STRING:
            printf("%s", node->string.value);
            return 0;
            
        case NODE_UNARY_OP: {
            int val = interpret_ast(node->unary.operand);
            switch (node->unary.op) {
                case TOKEN_PLUS: return +val;
                case TOKEN_MINUS: return -val;
                case TOKEN_NOT: return !val;
                case TOKEN_BIT_NOT: return ~val;
                case TOKEN_PLUS_PLUS: return val + 1;
                case TOKEN_MINUS_MINUS: return val - 1;
                default: return 0;
            }
        }
        
        case NODE_BINARY_OP: {
            int left = interpret_ast(node->binary.left);
            int right = interpret_ast(node->binary.right);
            
            switch (node->binary.op) {
                case TOKEN_PLUS: return left + right;
                case TOKEN_MINUS: return left - right;
                case TOKEN_STAR: return left * right;
                case TOKEN_SLASH: return right != 0 ? left / right : 0;
                case TOKEN_PERCENT: return right != 0 ? left % right : 0;
                case TOKEN_LT: return left < right;
                case TOKEN_GT: return left > right;
                case TOKEN_LE: return left <= right;
                case TOKEN_GE: return left >= right;
                case TOKEN_EQ: return left == right;
                case TOKEN_NEQ: return left != right;
                case TOKEN_AND: return left && right;
                case TOKEN_OR: return left || right;
                case TOKEN_BIT_AND: return left & right;
                case TOKEN_BIT_OR: return left | right;
                case TOKEN_BIT_XOR: return left ^ right;
                case TOKEN_SHIFT_LEFT: return left << right;
                case TOKEN_SHIFT_RIGHT: return left >> right;
                default: return 0;
            }
        }
        
        case NODE_TERNARY_OP:
            return interpret_ast(node->ternary.condition) ? 
                   interpret_ast(node->ternary.true_expr) : 
                   interpret_ast(node->ternary.false_expr);
        
        case NODE_ASSIGNMENT:
            set_variable(node->assignment.var_name, 
                        interpret_ast(node->assignment.expression));
            return get_variable(node->assignment.var_name);
        
        case NODE_CALL: {
            if (strcmp(node->call.func_name, "print") == 0) {
                for (int i = 0; i < node->call.arg_count; i++) {
                    int val = interpret_ast(node->call.args[i]);
                    printf("%d", val);
                    if (i < node->call.arg_count - 1) printf(" ");
                }
                printf("\n");
                return 0;
            }
            printf("Неизвестная функция: %s\n", node->call.func_name);
            return 0;
        }
        
        default:
            printf("Неизвестный тип узла\n");
            return 0;
    }
}
