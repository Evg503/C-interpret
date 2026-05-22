#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"

// Вспомогательные функции
void init_parser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->current = get_next_token(lexer);
}

void advance(Parser* parser) {
    if (parser->current.type != TOKEN_EOF) {
        if (parser->current.value) {
            free(parser->current.value);
        }
        parser->current = get_next_token(parser->lexer);
    }
}

bool match(Parser* parser, TokenType type) {
    if (parser->current.type == type) {
        advance(parser);
        return true;
    }
    return false;
}

void expect(Parser* parser, TokenType type, const char* error_message) {
    if (parser->current.type != type) {
        printf("Ошибка на строке %d, колонке %d: %s\n", 
               parser->current.line, parser->current.column, error_message);
        exit(1);
    }
    advance(parser);
}

// Парсинг программы (список операторов)
ASTNode* parse_program(Parser* parser) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_STATEMENT_LIST;
    node->statement_list.statements = NULL;
    node->statement_list.count = 0;
    node->statement_list.capacity = 0;
    
    while (parser->current.type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement(parser);
        
        if (node->statement_list.count >= node->statement_list.capacity) {
            node->statement_list.capacity = node->statement_list.capacity == 0 ? 4 : node->statement_list.capacity * 2;
            node->statement_list.statements = (ASTNode**)realloc(
                node->statement_list.statements,
                node->statement_list.capacity * sizeof(ASTNode*)
            );
        }
        node->statement_list.statements[node->statement_list.count++] = stmt;
    }
    
    return node;
}

// Парсинг оператора
ASTNode* parse_statement(Parser* parser) {
    ASTNode* node = NULL;
    
    // Объявление переменной: int x = 5;
    if (parser->current.type == TOKEN_INT) {
        advance(parser);
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_VARIABLE_DECL;
        
        expect(parser, TOKEN_IDENTIFIER, "Ожидается имя переменной");
        node->var_decl.var_name = strdup(parser->current.value); // Временно сохраняем, но current уже изменён expect
        // Нужно сохранить имя до advance
        char* var_name = strdup(parser->current.value);
        node->var_decl.var_name = var_name;
        
        if (match(parser, TOKEN_ASSIGN)) {
            node->var_decl.initializer = parse_expression(parser);
        } else {
            node->var_decl.initializer = NULL;
        }
        
        expect(parser, TOKEN_SEMICOLON, "Ожидается ';' после объявления переменной");
        return node;
    }
    
    // Присваивание: x = 10;
    if (parser->current.type == TOKEN_IDENTIFIER) {
        return parse_assignment(parser);
    }
    
    // print выражение;
    if (parser->current.type == TOKEN_PRINT) {
        advance(parser);
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_PRINT_STATEMENT;
        node->print_stmt.expression = parse_expression(parser);
        expect(parser, TOKEN_SEMICOLON, "Ожидается ';' после print");
        return node;
    }
    
    // if (условие) { ... } [else { ... }]
    if (parser->current.type == TOKEN_IF) {
        advance(parser);
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_IF_STATEMENT;
        
        expect(parser, TOKEN_LPAREN, "Ожидается '(' после if");
        node->if_stmt.condition = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Ожидается ')' после условия");
        
        node->if_stmt.then_branch = parse_statement(parser);
        
        if (match(parser, TOKEN_ELSE)) {
            node->if_stmt.else_branch = parse_statement(parser);
        } else {
            node->if_stmt.else_branch = NULL;
        }
        
        return node;
    }
    
    // while (условие) { ... }
    if (parser->current.type == TOKEN_WHILE) {
        advance(parser);
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_WHILE_STATEMENT;
        
        expect(parser, TOKEN_LPAREN, "Ожидается '(' после while");
        node->while_stmt.condition = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Ожидается ')' после условия");
        
        node->while_stmt.body = parse_statement(parser);
        return node;
    }
    
    // Блок операторов { ... }
    if (parser->current.type == TOKEN_LBRACE) {
        advance(parser);
        
        ASTNode* block = (ASTNode*)malloc(sizeof(ASTNode));
        block->type = NODE_STATEMENT_LIST;
        block->statement_list.statements = NULL;
        block->statement_list.count = 0;
        block->statement_list.capacity = 0;
        
        while (parser->current.type != TOKEN_RBRACE && parser->current.type != TOKEN_EOF) {
            ASTNode* stmt = parse_statement(parser);
            
            if (block->statement_list.count >= block->statement_list.capacity) {
                block->statement_list.capacity = block->statement_list.capacity == 0 ? 4 : block->statement_list.capacity * 2;
                block->statement_list.statements = (ASTNode**)realloc(
                    block->statement_list.statements,
                    block->statement_list.capacity * sizeof(ASTNode*)
                );
            }
            block->statement_list.statements[block->statement_list.count++] = stmt;
        }
        
        expect(parser, TOKEN_RBRACE, "Ожидается '}'");
        return block;
    }
    
    printf("Ошибка: неожиданный токен ");
    print_token(&parser->current);
    printf("\n");
    exit(1);
}

// Парсинг присваивания
ASTNode* parse_assignment(Parser* parser) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODE_ASSIGNMENT;
    
    expect(parser, TOKEN_IDENTIFIER, "Ожидается идентификатор");
    node->assignment.var_name = strdup(parser->current.value); // Сохраняем имя
    char* var_name = strdup(parser->current.value);
    node->assignment.var_name = var_name;
    
    expect(parser, TOKEN_ASSIGN, "Ожидается '='");
    node->assignment.expression = parse_expression(parser);
    expect(parser, TOKEN_SEMICOLON, "Ожидается ';' после присваивания");
    
    return node;
}

// Парсинг выражения (сравнение)
ASTNode* parse_expression(Parser* parser) {
    return parse_comparison(parser);
}

// Парсинг сравнения
ASTNode* parse_comparison(Parser* parser) {
    ASTNode* node = parse_additive(parser);
    
    while (1) {
        TokenType op = parser->current.type;
        if (op == TOKEN_LT || op == TOKEN_GT || op == TOKEN_EQ || 
            op == TOKEN_NEQ || op == TOKEN_LE || op == TOKEN_GE) {
            advance(parser);
            ASTNode* new_node = (ASTNode*)malloc(sizeof(ASTNode));
            new_node->type = NODE_BINARY_OP;
            new_node->binary_op.left = node;
            new_node->binary_op.right = parse_additive(parser);
            
            // Преобразуем тип оператора в символ для упрощения
            switch (op) {
                case TOKEN_LT: new_node->binary_op.op = '<'; break;
                case TOKEN_GT: new_node->binary_op.op = '>'; break;
                case TOKEN_EQ: new_node->binary_op.op = '='; break;
                case TOKEN_NEQ: new_node->binary_op.op = '!'; break;
                default: new_node->binary_op.op = '?';
            }
            node = new_node;
        } else {
            break;
        }
    }
    
    return node;
}

// Парсинг сложения/вычитания
ASTNode* parse_additive(Parser* parser) {
    ASTNode* node = parse_multiplicative(parser);
    
    while (parser->current.type == TOKEN_PLUS || parser->current.type == TOKEN_MINUS) {
        char op = (parser->current.type == TOKEN_PLUS) ? '+' : '-';
        advance(parser);
        
        ASTNode* new_node = (ASTNode*)malloc(sizeof(ASTNode));
        new_node->type = NODE_BINARY_OP;
        new_node->binary_op.left = node;
        new_node->binary_op.right = parse_multiplicative(parser);
        new_node->binary_op.op = op;
        node = new_node;
    }
    
    return node;
}

// Парсинг умножения/деления
ASTNode* parse_multiplicative(Parser* parser) {
    ASTNode* node = parse_primary(parser);
    
    while (parser->current.type == TOKEN_STAR || parser->current.type == TOKEN_SLASH) {
        char op = (parser->current.type == TOKEN_STAR) ? '*' : '/';
        advance(parser);
        
        ASTNode* new_node = (ASTNode*)malloc(sizeof(ASTNode));
        new_node->type = NODE_BINARY_OP;
        new_node->binary_op.left = node;
        new_node->binary_op.right = parse_primary(parser);
        new_node->binary_op.op = op;
        node = new_node;
    }
    
    return node;
}

// Парсинг первичных выражений (числа, идентификаторы, скобки)
ASTNode* parse_primary(Parser* parser) {
    ASTNode* node = NULL;
    
    if (parser->current.type == TOKEN_NUMBER) {
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_NUMBER;
        node->number.value = atoi(parser->current.value);
        advance(parser);
        return node;
    }
    
    if (parser->current.type == TOKEN_IDENTIFIER) {
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_IDENTIFIER;
        node->identifier.name = strdup(parser->current.value);
        advance(parser);
        return node;
    }
    
    if (parser->current.type == TOKEN_LPAREN) {
        advance(parser);
        node = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Ожидается ')'");
        return node;
    }
    
    printf("Ошибка: неожиданный токен ");
    print_token(&parser->current);
    printf("\n");
    exit(1);
}