#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

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
        case NODE_STRING:
            printf("STRING(\"%s\")\n", node->string.value);
            break;
        case NODE_UNARY_OP:
            printf("UNARY_OP(%d)\n", node->unary.op);
            print_ast(node->unary.operand, indent + 1);
            break;
        case NODE_BINARY_OP:
            printf("BINARY_OP(%d)\n", node->binary.op);
            print_ast(node->binary.left, indent + 1);
            print_ast(node->binary.right, indent + 1);
            break;
        case NODE_TERNARY_OP:
            printf("TERNARY_OP\n");
            printf("  Condition:\n");
            print_ast(node->ternary.condition, indent + 1);
            printf("  True:\n");
            print_ast(node->ternary.true_expr, indent + 1);
            printf("  False:\n");
            print_ast(node->ternary.false_expr, indent + 1);
            break;
        case NODE_ASSIGNMENT:
            printf("ASSIGNMENT(%s)\n", node->assignment.var_name);
            print_ast(node->assignment.expression, indent + 1);
            break;
        case NODE_CALL:
            printf("CALL(%s, %d args)\n", node->call.func_name, node->call.arg_count);
            for (int i = 0; i < node->call.arg_count; i++)
                print_ast(node->call.args[i], indent + 1);
            break;
        default:
            printf("UNKNOWN\n");
    }
}

// Тестовая функция
int test_expressions() {
    // Тестовые выражения
    const char* expressions[] = {
        "1 + 2 * 3",
        "(1 + 2) * 3",
        "a = b = 5",
        "x = 10 + (y = 20) * 3",
        "a + b * c - d / e % f",
        "x > 0 && y < 10 || z == 5",
        "!a && b || c",
        "a << 2 + b >> 1",
        "max(10, 20, 30)",
        "a ? b : c",
        "++i",
        "-x * 3",
        NULL
    };
    
    for (int i = 0; expressions[i] != NULL; i++) {
        printf("\n=== Тест %d: %s ===\n", i+1, expressions[i]);
        
        Lexer lexer;
        init_lexer(&lexer, expressions[i]);
        
        Parser parser;
        init_parser(&parser, &lexer);
        
        ASTNode* ast = parse_expression(&parser);
        
        printf("AST:\n");
        print_ast(ast, 0);
        
        free_ast(ast);
    }
    
    return 0;
}

// Тест интерпретатора
int test_interpreter() {
    const char* test_code = 
        "a = 10\n"
        "b = 20\n"
        "c = a + b * 2\n"
        "print(c)\n"
        "d = (c - 10) / 2\n"
        "print(d)\n"
        "e = a > b ? a : b\n"
        "print(e)\n"
        "f = -a + b\n"
        "print(f)\n";
    
    printf("=== Интерпретация программы ===\n");
    
    // Разбиваем на строки и интерпретируем
    char* code_copy = strdup(test_code);
    char* line = strtok(code_copy, "\n");
    
    while (line) {
        printf("> %s\n", line);
        
        Lexer lexer;
        init_lexer(&lexer, line);
        
        Parser parser;
        init_parser(&parser, &lexer);
        
        ASTNode* ast = parse_expression(&parser);
        int result = interpret_ast(ast);
        
        // Если выражение не было присваиванием или вызовом функции
        if (ast->type != NODE_ASSIGNMENT && ast->type != NODE_CALL) {
            printf("= %d\n", result);
        }
        
        free_ast(ast);
        line = strtok(NULL, "\n");
    }
    
    free(code_copy);
    
    return 0;
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
    

    test_expressions();
    test_interpreter();
    
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