#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Вспомогательные функции
static void advance(Parser* parser) {
    if (parser->current.type != TOKEN_EOF) {
        if (parser->current.value) {
            free(parser->current.value);
            parser->current.value = NULL;
        }
        parser->current = get_next_token(parser->lexer);
    }
}

static bool match(Parser* parser, TokenType type) {
    if (parser->current.type == type) {
        advance(parser);
        return true;
    }
    return false;
}

static bool expect(Parser* parser, TokenType type, const char* error_message) {
    if (parser->current.type != type) {
        snprintf(parser->error_message, sizeof(parser->error_message),
                "Ошибка на строке %d, колонке %d: %s", 
                parser->current.line, parser->current.column, error_message);
        parser->has_error = 1;
        return false;
    }
    advance(parser);
    return true;
}

// Функции парсинга выражений
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_assignment(Parser* parser);
static ASTNode* parse_ternary(Parser* parser);
static ASTNode* parse_logical_or(Parser* parser);
static ASTNode* parse_logical_and(Parser* parser);
static ASTNode* parse_bitwise_or(Parser* parser);
static ASTNode* parse_bitwise_xor(Parser* parser);
static ASTNode* parse_bitwise_and(Parser* parser);
static ASTNode* parse_equality(Parser* parser);
static ASTNode* parse_relational(Parser* parser);
static ASTNode* parse_shift(Parser* parser);
static ASTNode* parse_additive(Parser* parser);
static ASTNode* parse_multiplicative(Parser* parser);
static ASTNode* parse_unary(Parser* parser);
static ASTNode* parse_primary(Parser* parser);
static ASTNode* parse_statement(Parser* parser);
static ASTNode* parse_block(Parser* parser);
static ASTNode* parse_switch_body(Parser* parser);

void init_parser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->current = get_next_token(lexer);
    parser->has_error = 0;
}

static ASTNode* create_number_node(int value, int line, int column) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = NODE_NUMBER;
    node->number.value = value;
    node->line = line;
    node->column = column;
    return node;
}

static ASTNode* create_identifier_node(const char* name, int line, int column) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = NODE_IDENTIFIER;
    node->identifier.name = strdup(name);
    node->line = line;
    node->column = column;
    return node;
}

static ASTNode* create_binary_op(int op, ASTNode* left, ASTNode* right, int line, int column) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = NODE_BINARY_OP;
    node->binary.op = op;
    node->binary.left = left;
    node->binary.right = right;
    node->line = line;
    node->column = column;
    return node;
}

static ASTNode* create_unary_op(int op, ASTNode* operand, int line, int column) {
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = NODE_UNARY_OP;
    node->unary.op = op;
    node->unary.operand = operand;
    node->line = line;
    node->column = column;
    return node;
}

// Парсинг программы
ASTNode* parse_program(Parser* parser) {
    ASTNode* program = (ASTNode*)calloc(1, sizeof(ASTNode));
    program->type = NODE_STATEMENT_LIST;
    program->statement_list.statements = NULL;
    program->statement_list.count = 0;
    program->statement_list.capacity = 0;
    
    while (parser->current.type != TOKEN_EOF && !parser->has_error) {
        ASTNode* stmt = parse_statement(parser);
        
        if (program->statement_list.count >= program->statement_list.capacity) {
            program->statement_list.capacity = program->statement_list.capacity == 0 ? 4 : program->statement_list.capacity * 2;
            program->statement_list.statements = (ASTNode**)realloc(
                program->statement_list.statements,
                program->statement_list.capacity * sizeof(ASTNode*)
            );
        }
        program->statement_list.statements[program->statement_list.count++] = stmt;
    }
    
    return program;
}

// Парсинг оператора
ASTNode* parse_statement(Parser* parser) {
    // Объявление переменной
    if (parser->current.type == TOKEN_INT) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        char* var_name = strdup(parser->current.value);
        expect(parser, TOKEN_IDENTIFIER, "Ожидается имя переменной");
        
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_VARIABLE_DECL;
        node->var_decl.var_name = var_name;
        node->var_decl.var_type = TOKEN_INT;
        node->line = line;
        node->column = column;
        
        if (match(parser, TOKEN_ASSIGN)) {
            node->var_decl.initializer = parse_expression(parser);
        } else {
            node->var_decl.initializer = NULL;
        }
        
        expect(parser, TOKEN_SEMICOLON, "Ожидается ';' после объявления");
        return node;
    }
    
    // Блок
    if (parser->current.type == TOKEN_LBRACE) {
        return parse_block(parser);
    }
    
    // Оператор if
    if (parser->current.type == TOKEN_IF) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_IF_STATEMENT;
        node->line = line;
        node->column = column;
        
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
    
    // Оператор while
    if (parser->current.type == TOKEN_WHILE) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_WHILE_STATEMENT;
        node->line = line;
        node->column = column;
        
        expect(parser, TOKEN_LPAREN, "Ожидается '(' после while");
        node->while_stmt.condition = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Ожидается ')' после условия");
        
        node->while_stmt.body = parse_statement(parser);
        
        return node;
    }
    
    // Оператор for
    if (parser->current.type == TOKEN_FOR) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_FOR_STATEMENT;
        node->line = line;
        node->column = column;
        
        expect(parser, TOKEN_LPAREN, "Ожидается '(' после for");
        
        node->for_stmt.init = parse_expression(parser);
        expect(parser, TOKEN_SEMICOLON, "Ожидается ';' в заголовке for");
        
        node->for_stmt.condition = parse_expression(parser);
        expect(parser, TOKEN_SEMICOLON, "Ожидается ';' в заголовке for");
        
        node->for_stmt.increment = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Ожидается ')' после заголовка for");
        
        node->for_stmt.body = parse_statement(parser);
        
        return node;
    }
    
    // Оператор do-while
    if (parser->current.type == TOKEN_DO) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_DO_WHILE_STATEMENT;
        node->line = line;
        node->column = column;
        
        node->do_while_stmt.body = parse_statement(parser);
        
        expect(parser, TOKEN_WHILE, "Ожидается 'while'");
        expect(parser, TOKEN_LPAREN, "Ожидается '(' после do-while");
        node->do_while_stmt.condition = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Ожидается ')' после условия");
        expect(parser, TOKEN_SEMICOLON, "Ожидается ';' после do-while");
        
        return node;
    }
    
    // Оператор switch
    if (parser->current.type == TOKEN_SWITCH) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_SWITCH_STATEMENT;
        node->line = line;
        node->column = column;
        
        expect(parser, TOKEN_LPAREN, "Ожидается '(' после switch");
        node->switch_stmt.expression = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Ожидается ')' после выражения switch");
        expect(parser, TOKEN_LBRACE, "Ожидается '{' после switch");
        
        node->switch_stmt.case_blocks = parse_switch_body(parser);
        
        return node;
    }
    
    // Оператор return
    if (parser->current.type == TOKEN_RETURN) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_RETURN_STATEMENT;
        node->line = line;
        node->column = column;
        
        if (parser->current.type != TOKEN_SEMICOLON) {
            node->return_stmt.expression = parse_expression(parser);
        } else {
            node->return_stmt.expression = NULL;
        }
        expect(parser, TOKEN_SEMICOLON, "Ожидается ';' после return");
        
        return node;
    }
    
    // Оператор print
    if (parser->current.type == TOKEN_PRINT) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        expect(parser, TOKEN_LPAREN, "Ожидается '(' после print");
        
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_PRINT_STATEMENT;
        node->print_stmt.expression = parse_expression(parser);
        node->line = line;
        node->column = column;
        
        expect(parser, TOKEN_RPAREN, "Ожидается ')'");
        expect(parser, TOKEN_SEMICOLON, "Ожидается ';' после print");
        
        return node;
    }
    
    // Оператор break
    if (parser->current.type == TOKEN_BREAK) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_BREAK_STATEMENT;
        node->line = line;
        node->column = column;
        
        expect(parser, TOKEN_SEMICOLON, "Ожидается ';' после break");
        return node;
    }
    
    // Оператор continue
    if (parser->current.type == TOKEN_CONTINUE) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_CONTINUE_STATEMENT;
        node->line = line;
        node->column = column;
        
        expect(parser, TOKEN_SEMICOLON, "Ожидается ';' после continue");
        return node;
    }
    
    // Вычисление выражения
    ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
    node->type = NODE_EXPRESSION_STATEMENT;
    node->expr_stmt.expression = parse_expression(parser);
    expect(parser, TOKEN_SEMICOLON, "Ожидается ';' после выражения");
    return node;
}

// Парсинг тела switch
static ASTNode* parse_switch_body(Parser* parser) {
    ASTNode* case_blocks = NULL;
    ASTNode* last_case = NULL;
    
    expect(parser, TOKEN_LBRACE, "Ожидается '{' после switch");
    
    while (!parser->has_error && parser->current.type != TOKEN_RBRACE && parser->current.type != TOKEN_EOF) {
        if (parser->current.type == TOKEN_CASE) {
            advance(parser);
            
            ASTNode* case_node = (ASTNode*)calloc(1, sizeof(ASTNode));
            case_node->type = NODE_CASE_BLOCK;
            case_node->line = parser->current.line;
            case_node->column = parser->current.column;
            
            case_node->case_block.condition = parse_expression(parser);
            expect(parser, TOKEN_COLON, "Ожидается ':' после case");
            
            ASTNode* body = (ASTNode*)calloc(1, sizeof(ASTNode));
            body->type = NODE_STATEMENT_LIST;
            body->statement_list.statements = NULL;
            body->statement_list.count = 0;
            body->statement_list.capacity = 0;
            
            while (!parser->has_error && parser->current.type != TOKEN_CASE 
                   && parser->current.type != TOKEN_DEFAULT 
                   && parser->current.type != TOKEN_RBRACE
                   && parser->current.type != TOKEN_EOF) {
                ASTNode* stmt = parse_statement(parser);
                if (body->statement_list.count >= body->statement_list.capacity) {
                    body->statement_list.capacity = body->statement_list.capacity == 0 ? 4 : body->statement_list.capacity * 2;
                    body->statement_list.statements = (ASTNode**)realloc(
                        body->statement_list.statements,
                        body->statement_list.capacity * sizeof(ASTNode*)
                    );
                }
                body->statement_list.statements[body->statement_list.count++] = stmt;
            }
            
            case_node->case_block.body = body;
            
            if (case_blocks == NULL) {
                case_blocks = case_node;
            } else {
                last_case->case_block.next = case_node;
            }
            last_case = case_node;
        } else if (parser->current.type == TOKEN_DEFAULT) {
            advance(parser);
            
            ASTNode* default_node = (ASTNode*)calloc(1, sizeof(ASTNode));
            default_node->type = NODE_CASE_BLOCK;
            default_node->line = parser->current.line;
            default_node->column = parser->current.column;
            default_node->case_block.condition = NULL;
            
            expect(parser, TOKEN_COLON, "Ожидается ':' после default");
            
            ASTNode* body = (ASTNode*)calloc(1, sizeof(ASTNode));
            body->type = NODE_STATEMENT_LIST;
            body->statement_list.statements = NULL;
            body->statement_list.count = 0;
            body->statement_list.capacity = 0;
            
            while (!parser->has_error && parser->current.type != TOKEN_DEFAULT 
                   && parser->current.type != TOKEN_RBRACE
                   && parser->current.type != TOKEN_EOF) {
                ASTNode* stmt = parse_statement(parser);
                if (body->statement_list.count >= body->statement_list.capacity) {
                    body->statement_list.capacity = body->statement_list.capacity == 0 ? 4 : body->statement_list.capacity * 2;
                    body->statement_list.statements = (ASTNode**)realloc(
                        body->statement_list.statements,
                        body->statement_list.capacity * sizeof(ASTNode*)
                    );
                }
                body->statement_list.statements[body->statement_list.count++] = stmt;
            }
            
            default_node->case_block.body = body;
            
            if (case_blocks == NULL) {
                case_blocks = default_node;
            } else {
                last_case->case_block.next = default_node;
            }
            last_case = default_node;
        } else {
            advance(parser);
        }
    }
    
    expect(parser, TOKEN_RBRACE, "Ожидается '}' после switch");
    
    return case_blocks;
}

// Блок операторов
static ASTNode* parse_block(Parser* parser) {
    ASTNode* block = (ASTNode*)calloc(1, sizeof(ASTNode));
    block->type = NODE_STATEMENT_LIST;
    block->statement_list.statements = NULL;
    block->statement_list.count = 0;
    block->statement_list.capacity = 0;
    
    if (parser->has_error) {
        free(block);
        return NULL;
    }
    expect(parser, TOKEN_LBRACE, "Ожидается '{'");
    
    while (!parser->has_error && parser->current.type != TOKEN_RBRACE && parser->current.type != TOKEN_EOF) {
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

// Парсинг выражений с приоритетами
ASTNode* parse_expression(Parser* parser) {
    return parse_assignment(parser);
}

static ASTNode* parse_assignment(Parser* parser) {
    ASTNode* node = parse_ternary(parser);
    
    if (parser->current.type == TOKEN_ASSIGN) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* right = parse_assignment(parser);
        
        if (node->type != NODE_IDENTIFIER) {
            snprintf(parser->error_message, sizeof(parser->error_message),
                    "Ошибка: левая часть присваивания должна быть lvalue");
            parser->has_error = 1;
            return NULL;
        }
        
        ASTNode* assign = (ASTNode*)calloc(1, sizeof(ASTNode));
        assign->type = NODE_ASSIGNMENT;
        assign->assignment.var_name = strdup(node->identifier.name);
        assign->assignment.expression = right;
        assign->line = line;
        assign->column = column;
        
        free_ast(node);
        return assign;
    }
    
    return node;
}

static ASTNode* parse_ternary(Parser* parser) {
    ASTNode* node = parse_logical_or(parser);
    
    if (parser->current.type == TOKEN_QUESTION) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* true_expr = parse_expression(parser);
        expect(parser, TOKEN_COLON, "Ожидается ':'");
        ASTNode* false_expr = parse_ternary(parser);
        
        ASTNode* ternary = (ASTNode*)calloc(1, sizeof(ASTNode));
        ternary->type = NODE_TERNARY_OP;
        ternary->ternary.condition = node;
        ternary->ternary.true_expr = true_expr;
        ternary->ternary.false_expr = false_expr;
        ternary->line = line;
        ternary->column = column;
        return ternary;
    }
    
    return node;
}

static ASTNode* parse_logical_or(Parser* parser) {
    ASTNode* node = parse_logical_and(parser);
    
    while (parser->current.type == TOKEN_OR) {
        int line = parser->current.line;
        int column = parser->current.column;
        int op = parser->current.type;
        advance(parser);
        
        ASTNode* right = parse_logical_and(parser);
        node = create_binary_op(op, node, right, line, column);
    }
    
    return node;
}

static ASTNode* parse_logical_and(Parser* parser) {
    ASTNode* node = parse_bitwise_or(parser);
    
    while (parser->current.type == TOKEN_AND) {
        int line = parser->current.line;
        int column = parser->current.column;
        int op = parser->current.type;
        advance(parser);
        
        ASTNode* right = parse_bitwise_or(parser);
        node = create_binary_op(op, node, right, line, column);
    }
    
    return node;
}

static ASTNode* parse_bitwise_or(Parser* parser) {
    ASTNode* node = parse_bitwise_xor(parser);
    
    while (parser->current.type == TOKEN_BIT_OR) {
        int line = parser->current.line;
        int column = parser->current.column;
        int op = parser->current.type;
        advance(parser);
        
        ASTNode* right = parse_bitwise_xor(parser);
        node = create_binary_op(op, node, right, line, column);
    }
    
    return node;
}

static ASTNode* parse_bitwise_xor(Parser* parser) {
    ASTNode* node = parse_bitwise_and(parser);
    
    while (parser->current.type == TOKEN_BIT_XOR) {
        int line = parser->current.line;
        int column = parser->current.column;
        int op = parser->current.type;
        advance(parser);
        
        ASTNode* right = parse_bitwise_and(parser);
        node = create_binary_op(op, node, right, line, column);
    }
    
    return node;
}

static ASTNode* parse_bitwise_and(Parser* parser) {
    ASTNode* node = parse_equality(parser);
    
    while (parser->current.type == TOKEN_BIT_AND) {
        int line = parser->current.line;
        int column = parser->current.column;
        int op = parser->current.type;
        advance(parser);
        
        ASTNode* right = parse_equality(parser);
        node = create_binary_op(op, node, right, line, column);
    }
    
    return node;
}

static ASTNode* parse_equality(Parser* parser) {
    ASTNode* node = parse_relational(parser);
    
    while (parser->current.type == TOKEN_EQ || parser->current.type == TOKEN_NEQ) {
        int line = parser->current.line;
        int column = parser->current.column;
        int op = parser->current.type;
        advance(parser);
        
        ASTNode* right = parse_relational(parser);
        node = create_binary_op(op, node, right, line, column);
    }
    
    return node;
}

static ASTNode* parse_relational(Parser* parser) {
    ASTNode* node = parse_shift(parser);
    
    while (parser->current.type == TOKEN_LT || parser->current.type == TOKEN_GT ||
           parser->current.type == TOKEN_LE || parser->current.type == TOKEN_GE) {
        int line = parser->current.line;
        int column = parser->current.column;
        int op = parser->current.type;
        advance(parser);
        
        ASTNode* right = parse_shift(parser);
        node = create_binary_op(op, node, right, line, column);
    }
    
    return node;
}

static ASTNode* parse_shift(Parser* parser) {
    ASTNode* node = parse_additive(parser);
    
    while (parser->current.type == TOKEN_SHIFT_LEFT || parser->current.type == TOKEN_SHIFT_RIGHT) {
        int line = parser->current.line;
        int column = parser->current.column;
        int op = parser->current.type;
        advance(parser);
        
        ASTNode* right = parse_additive(parser);
        node = create_binary_op(op, node, right, line, column);
    }
    
    return node;
}

static ASTNode* parse_additive(Parser* parser) {
    ASTNode* node = parse_multiplicative(parser);
    
    while (parser->current.type == TOKEN_PLUS || parser->current.type == TOKEN_MINUS) {
        int line = parser->current.line;
        int column = parser->current.column;
        int op = parser->current.type;
        advance(parser);
        
        ASTNode* right = parse_multiplicative(parser);
        node = create_binary_op(op, node, right, line, column);
    }
    
    return node;
}

static ASTNode* parse_multiplicative(Parser* parser) {
    ASTNode* node = parse_unary(parser);
    
    while (parser->current.type == TOKEN_STAR || parser->current.type == TOKEN_SLASH ||
           parser->current.type == TOKEN_PERCENT) {
        int line = parser->current.line;
        int column = parser->current.column;
        int op = parser->current.type;
        advance(parser);
        
        ASTNode* right = parse_unary(parser);
        node = create_binary_op(op, node, right, line, column);
    }
    
    return node;
}

static ASTNode* parse_unary(Parser* parser) {
    TokenType op = parser->current.type;
    
    if (op == TOKEN_PLUS || op == TOKEN_MINUS || op == TOKEN_NOT || 
        op == TOKEN_BIT_NOT || op == TOKEN_PLUS_PLUS || op == TOKEN_MINUS_MINUS) {
        int line = parser->current.line;
        int column = parser->current.column;
        advance(parser);
        
        ASTNode* operand = parse_unary(parser);
        return create_unary_op(op, operand, line, column);
    }
    
    return parse_primary(parser);
}

static ASTNode* parse_primary(Parser* parser) {
    int line = parser->current.line;
    int column = parser->current.column;
    
    if (parser->current.type == TOKEN_NUMBER) {
        int value = atoi(parser->current.value);
        advance(parser);
        return create_number_node(value, line, column);
    }
    
    if (parser->current.type == TOKEN_STRING) {
        ASTNode* node = (ASTNode*)calloc(1, sizeof(ASTNode));
        node->type = NODE_STRING;
        node->string.value = strdup(parser->current.value);
        node->line = line;
        node->column = column;
        advance(parser);
        return node;
    }
    
    if (parser->current.type == TOKEN_IDENTIFIER) {
        char* name = strdup(parser->current.value);
        advance(parser);
        
        // Вызов функции
        if (parser->current.type == TOKEN_LPAREN) {
            advance(parser);
            
            ASTNode* call = (ASTNode*)calloc(1, sizeof(ASTNode));
            call->type = NODE_CALL;
            call->call.func_name = name;
            call->call.args = NULL;
            call->call.arg_count = 0;
            call->line = line;
            call->column = column;
            
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
            
            expect(parser, TOKEN_RPAREN, "Ожидается ')'");
            return call;
        }
        
        return create_identifier_node(name, line, column);
    }
    
    if (parser->current.type == TOKEN_LPAREN) {
        advance(parser);
        ASTNode* node = parse_expression(parser);
        expect(parser, TOKEN_RPAREN, "Ожидается ')'");
        return node;
    }
    
    snprintf(parser->error_message, sizeof(parser->error_message),
            "Ошибка: неожиданный токен на строке %d", parser->current.line);
    parser->has_error = 1;
    return NULL;
}

// Освобождение AST
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
        case NODE_VARIABLE_DECL:
            free(node->var_decl.var_name);
            free_ast(node->var_decl.initializer);
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
        case NODE_RETURN_STATEMENT:
            free_ast(node->return_stmt.expression);
            break;
        case NODE_PRINT_STATEMENT:
            free_ast(node->print_stmt.expression);
            break;
        case NODE_BREAK_STATEMENT:
            break;
        case NODE_CONTINUE_STATEMENT:
            break;
        case NODE_SWITCH_STATEMENT:
            free_ast(node->switch_stmt.expression);
            free_ast(node->switch_stmt.case_blocks);
            break;
        case NODE_CASE_BLOCK:
            free_ast(node->case_block.condition);
            free_ast(node->case_block.body);
            free_ast(node->case_block.next);
            break;
        default:
            break;
    }
    
    free(node);
}

// Печать AST для отладки
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
        case NODE_RETURN_STATEMENT:
            printf("RETURN\n");
            if (node->return_stmt.expression)
                print_ast(node->return_stmt.expression, indent + 1);
            break;
        case NODE_PRINT_STATEMENT:
            printf("PRINT\n");
            print_ast(node->print_stmt.expression, indent + 1);
            break;
        case NODE_BREAK_STATEMENT:
            printf("BREAK\n");
            break;
        case NODE_CONTINUE_STATEMENT:
            printf("CONTINUE\n");
            break;
        case NODE_SWITCH_STATEMENT:
            printf("SWITCH\n");
            printf("  Expression:\n");
            print_ast(node->switch_stmt.expression, indent + 1);
            if (node->switch_stmt.case_blocks) {
                printf("  Cases:\n");
                ASTNode* current_case = node->switch_stmt.case_blocks;
                while (current_case) {
                    if (current_case->case_block.condition) {
                        printf("    CASE:\n");
                        print_ast(current_case->case_block.condition, indent + 3);
                    } else {
                        printf("    DEFAULT:\n");
                    }
                    printf("    Body:\n");
                    print_ast(current_case->case_block.body, indent + 3);
                    current_case = current_case->case_block.next;
                }
            }
            break;
        default:
            printf("UNKNOWN\n");
    }
}

const char* parser_get_error(Parser* parser) {
    return parser->error_message;
}

int parser_has_error(Parser* parser) {
    return parser->has_error;
}