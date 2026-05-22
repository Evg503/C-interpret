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

Precedence get_precedence(TokenType op) {
    switch (op) {
        // Тернарный оператор (низший приоритет)
        case TOKEN_ASSIGN: return (Precedence){1, 1}; // right-associative
        
        // Логические операторы
        case TOKEN_OR: return (Precedence){2, 0};
        case TOKEN_AND: return (Precedence){3, 0};
        
        // Битовые OR
        case TOKEN_BIT_OR: return (Precedence){4, 0};
        case TOKEN_BIT_XOR: return (Precedence){5, 0};
        case TOKEN_BIT_AND: return (Precedence){6, 0};
        
        // Сравнение
        case TOKEN_EQ: case TOKEN_NEQ: return (Precedence){7, 0};
        case TOKEN_LT: case TOKEN_GT: case TOKEN_LE: case TOKEN_GE: 
            return (Precedence){8, 0};
        
        // Сдвиги
        case TOKEN_SHIFT_LEFT: case TOKEN_SHIFT_RIGHT: 
            return (Precedence){9, 0};
        
        // Сложение/вычитание
        case TOKEN_PLUS: case TOKEN_MINUS: 
            return (Precedence){10, 0};
        
        // Умножение/деление/остаток
        case TOKEN_STAR: case TOKEN_SLASH: case TOKEN_PERCENT: 
            return (Precedence){11, 0};
        
        // Унарные операторы (высший приоритет)
        default: return (Precedence){0, 0};
    }
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

// Основная функция парсинга выражений с приоритетами
ASTNode* parse_expression(Parser* parser) {
    return parse_assignment(parser);
}

// Присваивание (самый низкий приоритет)
ASTNode* parse_assignment(Parser* parser) {
    ASTNode* node = parse_ternary(parser);
    
    if (parser->current.type == TOKEN_ASSIGN) {
        advance(parser);
        ASTNode* right = parse_assignment(parser);
        
        // Проверяем, что слева - lvalue (идентификатор)
        if (node->type != NODE_IDENTIFIER) {
            printf("Ошибка: левая часть присваивания должна быть lvalue\n");
            exit(1);
        }
        
        ASTNode* assign = (ASTNode*)malloc(sizeof(ASTNode));
        assign->type = NODE_ASSIGNMENT;
        assign->assignment.var_name = strdup(node->identifier.name);
        assign->assignment.expression = right;
        free_ast(node); // Освобождаем старый узел
        return assign;
    }
    
    return node;
}

// Тернарный оператор ? :
ASTNode* parse_ternary(Parser* parser) {
    ASTNode* node = parse_logical_or(parser);
    
    if (parser->current.type == TOKEN_OR) { // Временно используем TOKEN_OR для '?'
        advance(parser);
        ASTNode* true_expr = parse_expression(parser);
        expect(parser, TOKEN_ASSIGN, "Ожидается ':'"); // Временно
        ASTNode* false_expr = parse_ternary(parser);
        
        ASTNode* ternary = (ASTNode*)malloc(sizeof(ASTNode));
        ternary->type = NODE_TERNARY_OP;
        ternary->ternary.condition = node;
        ternary->ternary.true_expr = true_expr;
        ternary->ternary.false_expr = false_expr;
        return ternary;
    }
    
    return node;
}

// Логическое ИЛИ
ASTNode* parse_logical_or(Parser* parser) {
    ASTNode* node = parse_logical_and(parser);
    
    while (parser->current.type == TOKEN_OR) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_logical_and(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Логическое И
ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* node = parse_bitwise_or(parser);
    
    while (parser->current.type == TOKEN_AND) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_bitwise_or(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Битовое ИЛИ
ASTNode* parse_bitwise_or(Parser* parser) {
    ASTNode* node = parse_bitwise_xor(parser);
    
    while (parser->current.type == TOKEN_BIT_OR) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_bitwise_xor(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Битовое XOR
ASTNode* parse_bitwise_xor(Parser* parser) {
    ASTNode* node = parse_bitwise_and(parser);
    
    while (parser->current.type == TOKEN_BIT_XOR) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_bitwise_and(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Битовое И
ASTNode* parse_bitwise_and(Parser* parser) {
    ASTNode* node = parse_equality(parser);
    
    while (parser->current.type == TOKEN_BIT_AND) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_equality(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Операторы равенства
ASTNode* parse_equality(Parser* parser) {
    ASTNode* node = parse_relational(parser);
    
    while (parser->current.type == TOKEN_EQ || parser->current.type == TOKEN_NEQ) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_relational(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Операторы сравнения
ASTNode* parse_relational(Parser* parser) {
    ASTNode* node = parse_shift(parser);
    
    while (parser->current.type == TOKEN_LT || parser->current.type == TOKEN_GT ||
           parser->current.type == TOKEN_LE || parser->current.type == TOKEN_GE) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_shift(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Операторы сдвига
ASTNode* parse_shift(Parser* parser) {
    ASTNode* node = parse_additive(parser);
    
    while (parser->current.type == TOKEN_SHIFT_LEFT || parser->current.type == TOKEN_SHIFT_RIGHT) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_additive(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Сложение и вычитание
ASTNode* parse_additive(Parser* parser) {
    ASTNode* node = parse_multiplicative(parser);
    
    while (parser->current.type == TOKEN_PLUS || parser->current.type == TOKEN_MINUS) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_multiplicative(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Умножение, деление, остаток
ASTNode* parse_multiplicative(Parser* parser) {
    ASTNode* node = parse_unary(parser);
    
    while (parser->current.type == TOKEN_STAR || parser->current.type == TOKEN_SLASH ||
           parser->current.type == TOKEN_PERCENT) {
        int op = parser->current.type;
        advance(parser);
        ASTNode* right = parse_unary(parser);
        
        ASTNode* binary = (ASTNode*)malloc(sizeof(ASTNode));
        binary->type = NODE_BINARY_OP;
        binary->binary.op = op;
        binary->binary.left = node;
        binary->binary.right = right;
        node = binary;
    }
    
    return node;
}

// Унарные операторы
ASTNode* parse_unary(Parser* parser) {
    TokenType op = parser->current.type;
    
    if (op == TOKEN_PLUS || op == TOKEN_MINUS || op == TOKEN_NOT || 
        op == TOKEN_BIT_NOT || op == TOKEN_PLUS_PLUS || op == TOKEN_MINUS_MINUS) {
        advance(parser);
        ASTNode* operand = parse_unary(parser);
        
        ASTNode* unary = (ASTNode*)malloc(sizeof(ASTNode));
        unary->type = NODE_UNARY_OP;
        unary->unary.op = op;
        unary->unary.operand = operand;
        return unary;
    }
    
    return parse_primary(parser);
}

// Первичные выражения (литералы, идентификаторы, скобки)
ASTNode* parse_primary(Parser* parser) {
    ASTNode* node = NULL;
    
    if (parser->current.type == TOKEN_NUMBER) {
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_NUMBER;
        node->number.value = atoi(parser->current.value);
        advance(parser);
        return node;
    }
    
    if (parser->current.type == TOKEN_STRING) {
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_STRING;
        node->string.value = strdup(parser->current.value);
        advance(parser);
        return node;
    }
    
    if (parser->current.type == TOKEN_IDENTIFIER) {
        node = (ASTNode*)malloc(sizeof(ASTNode));
        node->type = NODE_IDENTIFIER;
        node->identifier.name = strdup(parser->current.value);
        advance(parser);
        
        // Проверка на вызов функции
        if (parser->current.type == TOKEN_LPAREN) {
            advance(parser);
            ASTNode* call = (ASTNode*)malloc(sizeof(ASTNode));
            call->type = NODE_CALL;
            call->call.func_name = strdup(node->identifier.name);
            call->call.args = NULL;
            call->call.arg_count = 0;
            
            // Парсим аргументы
            if (parser->current.type != TOKEN_RPAREN) {
                int capacity = 4;
                call->call.args = (ASTNode**)malloc(capacity * sizeof(ASTNode*));
                do {
                    if (call->call.arg_count >= capacity) {
                        capacity *= 2;
                        call->call.args = (ASTNode**)realloc(call->call.args, capacity * sizeof(ASTNode*));
                    }
                    call->call.args[call->call.arg_count++] = parse_expression(parser);
                } while (match(parser, TOKEN_COMMA));
            }
            
            expect(parser, TOKEN_RPAREN, "Ожидается ')' после аргументов функции");
            free_ast(node);
            return call;
        }
        
        return node;
    }
    
    if (parser->current.type == TOKEN_LPAREN) {
        advance(parser);
        node = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Ожидается ')'");
        return node;
    }
    
    printf("Ошибка: неожиданный токен ");
    // print_token(&parser->current);
    printf("\n");
    exit(1);
}

// Функция освобождения AST (рекурсивная)
void free_ast(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_IDENTIFIER:
            free(node->identifier.name);
            break;
        case NODE_STRING:
            free(node->string.value);
            break;
        case NODE_UNARY_OP:
            free_ast(node->unary.operand);
            break;
        case NODE_BINARY_OP:
            free_ast(node->binary.left);
            free_ast(node->binary.right);
            break;
        case NODE_TERNARY_OP:
            free_ast(node->ternary.condition);
            free_ast(node->ternary.true_expr);
            free_ast(node->ternary.false_expr);
            break;
        case NODE_ASSIGNMENT:
            free(node->assignment.var_name);
            free_ast(node->assignment.expression);
            break;
        case NODE_CALL:
            free(node->call.func_name);
            for (int i = 0; i < node->call.arg_count; i++)
                free_ast(node->call.args[i]);
            free(node->call.args);
            break;
        default:
            break;
    }
    
    free(node);
}
