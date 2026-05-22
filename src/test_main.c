#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"


// Функция для печати AST (отладка)

void print_ast(ASTNode* node, int indent) {
    if (!node) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    
    switch (node->type) {
        case NODE_NUMBER:
            printf("NUMBER(%d)\n", node->number.value);
            break;
        case NODE_IDENTIFIER:
            printf("IDENTIFIER(%s)\n", node->identifier.name);
            break;
        case NODE_BINARY_OP:
            printf("BINARY_OP(%c)\n", node->binary_op.op);
            print_ast(node->binary_op.left, indent + 1);
            print_ast(node->binary_op.right, indent + 1);
            break;
        case NODE_ASSIGNMENT:
            printf("ASSIGNMENT(%s)\n", node->assignment.var_name);
            print_ast(node->assignment.expression, indent + 1);
            break;
        case NODE_VARIABLE_DECL:
            printf("VARIABLE_DECL(%s)\n", node->var_decl.var_name);
            if (node->var_decl.initializer)
                print_ast(node->var_decl.initializer, indent + 1);
            break;
        case NODE_STATEMENT_LIST:
            printf("STATEMENT_LIST (%d statements)\n", node->statement_list.count);
            for (int i = 0; i < node->statement_list.count; i++)
                print_ast(node->statement_list.statements[i], indent + 1);
            break;
        case NODE_IF_STATEMENT:
            printf("IF_STATEMENT\n");
            printf("  Condition:\n");
            print_ast(node->if_stmt.condition, indent + 1);
            printf("  Then:\n");
            print_ast(node->if_stmt.then_branch, indent + 1);
            if (node->if_stmt.else_branch) {
                printf("  Else:\n");
                print_ast(node->if_stmt.else_branch, indent + 1);
            }
            break;
        case NODE_WHILE_STATEMENT:
            printf("WHILE_STATEMENT\n");
            printf("  Condition:\n");
            print_ast(node->while_stmt.condition, indent + 1);
            printf("  Body:\n");
            print_ast(node->while_stmt.body, indent + 1);
            break;
        case NODE_PRINT_STATEMENT:
            printf("PRINT\n");
            print_ast(node->print_stmt.expression, indent + 1);
            break;
    }
}

// Освобождение AST
void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_IDENTIFIER:
            free(node->identifier.name);
            break;
        case NODE_ASSIGNMENT:
            free(node->assignment.var_name);
            free_ast(node->assignment.expression);
            break;
        case NODE_VARIABLE_DECL:
            free(node->var_decl.var_name);
            free_ast(node->var_decl.initializer);
            break;
        case NODE_BINARY_OP:
            free_ast(node->binary_op.left);
            free_ast(node->binary_op.right);
            break;
        case NODE_STATEMENT_LIST:
            for (int i = 0; i < node->statement_list.count; i++)
                free_ast(node->statement_list.statements[i]);
            free(node->statement_list.statements);
            break;
        case NODE_IF_STATEMENT:
            free_ast(node->if_stmt.condition);
            free_ast(node->if_stmt.then_branch);
            free_ast(node->if_stmt.else_branch);
            break;
        case NODE_WHILE_STATEMENT:
            free_ast(node->while_stmt.condition);
            free_ast(node->while_stmt.body);
            break;
        case NODE_PRINT_STATEMENT:
            free_ast(node->print_stmt.expression);
            break;
        default:
            break;
    }
    
    free(node);
}

int main() {
    // Тестовая программа
    const char* program = 
        "int x = 5;\n"
        "int y = 10;\n"
        "x = x + y * 2;\n"
        "if (x > 20) {\n"
        "    print x;\n"
        "} else {\n"
        "    print 0;\n"
        "}\n"
        "while (x > 0) {\n"
        "    x = x - 1;\n"
        "}\n";
    
    printf("Исходная программа:\n%s\n", program);
    printf("\n=== Лексический анализ ===\n");
    
    Lexer lexer;
    init_lexer(&lexer, program);
    
    // Вывод всех токенов
    Token token;
    do {
        token = get_next_token(&lexer);
        print_token(&token);
        if (token.value) {
            printf("('%s')", token.value);
            free(token.value);
        }
        printf(" ");
    } while (token.type != TOKEN_EOF);
    printf("\n\n");
    
    // Сброс лексера для парсера
    init_lexer(&lexer, program);
    
    printf("=== Парсинг ===\n");
    Parser parser;
    init_parser(&parser, &lexer);
    
    ASTNode* ast = parse_program(&parser);
    
    printf("\n=== AST ===\n");
    print_ast(ast, 0);
    
    free_ast(ast);
    
    return 0;
}