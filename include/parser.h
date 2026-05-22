#pragma once

// AST узлы
typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_BINARY_OP,
    NODE_ASSIGNMENT,
    NODE_VARIABLE_DECL,
    NODE_STATEMENT_LIST,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_PRINT_STATEMENT
} NodeType;

typedef struct ASTNode {
    NodeType type;
    union {
        struct {
            int value;
        } number;
        struct {
            char* name;
        } identifier;
        struct {
            struct ASTNode* left;
            struct ASTNode* right;
            char op;  // '+', '-', '*', '/'
        } binary_op;
        struct {
            char* var_name;
            struct ASTNode* expression;
        } assignment;
        struct {
            char* var_name;
            struct ASTNode* initializer;
        } var_decl;
        struct {
            struct ASTNode** statements;
            int count;
            int capacity;
        } statement_list;
        struct {
            struct ASTNode* condition;
            struct ASTNode* then_branch;
            struct ASTNode* else_branch;
        } if_stmt;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
        } while_stmt;
        struct {
            struct ASTNode* expression;
        } print_stmt;
    };
} ASTNode;

// Структура парсера
typedef struct {
    Lexer* lexer;
    Token current;
} Parser;

// Прототипы функций парсера
ASTNode* parse_program(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_assignment(Parser* parser);
ASTNode* parse_comparison(Parser* parser);
ASTNode* parse_additive(Parser* parser);
ASTNode* parse_multiplicative(Parser* parser);
ASTNode* parse_primary(Parser* parser);

void init_parser(Parser* parser, Lexer* lexer);
void advance(Parser* parser);
bool match(Parser* parser, TokenType type);
void expect(Parser* parser, TokenType type, const char* error_message);

