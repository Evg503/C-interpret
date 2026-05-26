#include "interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SYMBOLS 256

static Symbol symbol_table[MAX_SYMBOLS];
static int symbol_count = 0;

void init_interpreter(void) {
    symbol_count = 0;
    memset(symbol_table, 0, sizeof(symbol_table));
}

int get_variable(const char* name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            return symbol_table[i].value;
        }
    }
    return 0;
}

void set_variable(const char* name, int value) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0) {
            symbol_table[i].value = value;
            return;
        }
    }
    
    if (symbol_count < MAX_SYMBOLS) {
        symbol_table[symbol_count].name = strdup(name);
        symbol_table[symbol_count].value = value;
        symbol_count++;
    } else {
        printf("Ошибка: слишком много переменных\n");
    }
}

void print_symbol_table(void) {
    for (int i = 0; i < symbol_count; i++) {
        printf("  %s = %d\n", symbol_table[i].name, symbol_table[i].value);
    }
}

static int interpret_expression(ASTNode* node);

typedef enum {
    EXECUTE_NORMAL,
    EXECUTE_BREAK,
    EXECUTE_CONTINUE
} ExecutionResult;

static ExecutionResult interpret_statement(ASTNode* node);

static ExecutionResult interpret_statement_list(ASTNode* node_list);

// Интерпретация оператора
static ExecutionResult interpret_statement(ASTNode* node) {
    if (!node) return EXECUTE_NORMAL;
    
    switch (node->type) {
        case NODE_VARIABLE_DECL:
            if (node->var_decl.initializer) {
                int value = interpret_expression(node->var_decl.initializer);
                set_variable(node->var_decl.var_name, value);
            } else {
                set_variable(node->var_decl.var_name, 0);
            }
            break;
            
        case NODE_ASSIGNMENT:
            {
                int value = interpret_expression(node->assignment.expression);
                set_variable(node->assignment.var_name, value);
            }
            break;
            
        case NODE_IF_STATEMENT:
            {
                int condition = interpret_expression(node->if_stmt.condition);
                if (condition) {
                    ExecutionResult result = interpret_statement(node->if_stmt.then_branch);
                    if (result == EXECUTE_BREAK || result == EXECUTE_CONTINUE) {
                        return result;
                    }
                } else if (node->if_stmt.else_branch) {
                    ExecutionResult result = interpret_statement(node->if_stmt.else_branch);
                    if (result == EXECUTE_BREAK || result == EXECUTE_CONTINUE) {
                        return result;
                    }
                }
            }
            break;
            
        case NODE_WHILE_STATEMENT:
            {
                while (interpret_expression(node->while_stmt.condition)) {
                    ExecutionResult result = interpret_statement(node->while_stmt.body);
                    if (result == EXECUTE_BREAK) {
                        return EXECUTE_BREAK;
                    } else if (result == EXECUTE_CONTINUE) {
                        continue;
                    }
                }
            }
            break;
            
        case NODE_PRINT_STATEMENT:
            {
                int value = interpret_expression(node->print_stmt.expression);
                printf("%d\n", value);
            }
            break;
            
        case NODE_RETURN_STATEMENT:
            if (node->return_stmt.expression) {
                interpret_expression(node->return_stmt.expression);
            }
            break;
            
        case NODE_BREAK_STATEMENT:
            return EXECUTE_BREAK;
            
        case NODE_CONTINUE_STATEMENT:
            return EXECUTE_CONTINUE;
            
        case NODE_SWITCH_STATEMENT: {
            int value = interpret_expression(node->switch_stmt.expression);
            ASTNode* current_case = node->switch_stmt.case_blocks;
            int matched = 0;
            int executed = 0;
            
            while (current_case) {
                if (current_case->case_block.condition == NULL) {
                    matched = 1;
                }
                
                if (interpret_expression(current_case->case_block.condition) == value) {
                    matched = 1;
                }
                
                if (matched && !executed) {
                    executed = 1;
                }
                
                if (executed) {
                    ExecutionResult result = interpret_statement(current_case->case_block.body);
                    if (result == EXECUTE_BREAK) {
                        return EXECUTE_BREAK;
                    } else if (result == EXECUTE_CONTINUE) {
                        return EXECUTE_CONTINUE;
                    }
                    if (current_case->case_block.next) {
                        executed = 0;
                        matched = 0;
                    }
                }
                
                current_case = current_case->case_block.next;
            }
            break;
        }
            
        case NODE_STATEMENT_LIST:
            return interpret_statement_list(node);
            
        case NODE_EXPRESSION_STATEMENT:
            interpret_expression(node->expr_stmt.expression);
            break;
            
        default:
            interpret_expression(node);
            break;
    }
    return EXECUTE_NORMAL;
}

static ExecutionResult interpret_statement_list(ASTNode* node_list) {
    if (!node_list || node_list->type != NODE_STATEMENT_LIST) {
        return EXECUTE_NORMAL;
    }
    
    for (int i = 0; i < node_list->statement_list.count; i++) {
        ExecutionResult result = interpret_statement(node_list->statement_list.statements[i]);
        if (result == EXECUTE_BREAK || result == EXECUTE_CONTINUE) {
            return result;
        }
    }
    return EXECUTE_NORMAL;
}

// Интерпретация выражения
static int interpret_expression(ASTNode* node) {
    if (!node) return 0;
    
    switch (node->type) {
        case NODE_NUMBER:
            return node->number.value;
            
        case NODE_IDENTIFIER:
            return get_variable(node->identifier.name);
            
        case NODE_UNARY_OP: {
            int val = interpret_expression(node->unary.operand);
            switch (node->unary.op) {
                case TOKEN_PLUS: return +val;
                case TOKEN_MINUS: return -val;
                case TOKEN_NOT: return !val;
                case TOKEN_BIT_NOT: return ~val;
                default: return val;
            }
        }
        
        case NODE_BINARY_OP: {
            int left = interpret_expression(node->binary.left);
            int right = interpret_expression(node->binary.right);
            
            switch (node->binary.op) {
                case TOKEN_PLUS: return left + right;
                case TOKEN_MINUS: return left - right;
                case TOKEN_STAR: return left * right;
                case TOKEN_SLASH: 
                    if (right == 0) {
                        printf("Ошибка: деление на ноль\n");
                        return 0;
                    }
                    return left / right;
                case TOKEN_PERCENT:
                    if (right == 0) {
                        printf("Ошибка: деление на ноль\n");
                        return 0;
                    }
                    return left % right;
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
            return interpret_expression(node->ternary.condition) ?
                   interpret_expression(node->ternary.true_expr) :
                   interpret_expression(node->ternary.false_expr);
        
        case NODE_ASSIGNMENT:
            {
                int value = interpret_expression(node->assignment.expression);
                set_variable(node->assignment.var_name, value);
                return value;
            }
            
        case NODE_CALL:
            if (strcmp(node->call.func_name, "print") == 0) {
                for (int i = 0; i < node->call.arg_count; i++) {
                    int val = interpret_expression(node->call.args[i]);
                    printf("%d", val);
                    if (i < node->call.arg_count - 1) printf(" ");
                }
                printf("\n");
                return 0;
            }
            printf("Предупреждение: вызов неизвестной функции '%s'\n", node->call.func_name);
            return 0;
            
        default:
            return 0;
    }
}

// Основная функция интерпретации AST
int interpret_ast(ASTNode* node) {
    if (!node) return 0;
    
    if (node->type == NODE_STATEMENT_LIST) {
        if (node->statement_list.count == 1 && 
            node->statement_list.statements[0]->type == NODE_EXPRESSION_STATEMENT) {
            return interpret_expression(node->statement_list.statements[0]->expr_stmt.expression);
        }
        interpret_statement(node);
        return 0;
    } else if (node->type == NODE_VARIABLE_DECL ||
               node->type == NODE_IF_STATEMENT ||
               node->type == NODE_WHILE_STATEMENT ||
               node->type == NODE_FOR_STATEMENT ||
               node->type == NODE_DO_WHILE_STATEMENT ||
               node->type == NODE_RETURN_STATEMENT ||
               node->type == NODE_BREAK_STATEMENT ||
               node->type == NODE_CONTINUE_STATEMENT ||
               node->type == NODE_PRINT_STATEMENT) {
        interpret_statement(node);
        return 0;
    } else if (node->type == NODE_EXPRESSION_STATEMENT) {
        return interpret_expression(node->expr_stmt.expression);
    } else if (node->type == NODE_NUMBER ||
               node->type == NODE_IDENTIFIER ||
               node->type == NODE_UNARY_OP ||
               node->type == NODE_BINARY_OP ||
               node->type == NODE_TERNARY_OP ||
               node->type == NODE_ASSIGNMENT ||
               node->type == NODE_CALL) {
        return interpret_expression(node);
    } else {
        return 0;
    }
}