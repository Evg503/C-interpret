#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// Типы узлов AST
typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_STRING,
    NODE_UNARY_OP,
    NODE_BINARY_OP,
    NODE_TERNARY_OP,
    NODE_ASSIGNMENT,
    NODE_CALL,
    NODE_VARIABLE_DECL,
    NODE_STATEMENT_LIST,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_FOR_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_BREAK_STATEMENT,
    NODE_CONTINUE_STATEMENT,
    NODE_PRINT_STATEMENT,
    NODE_EXPRESSION_STATEMENT,
    NODE_BLOCK,
    NODE_DO_WHILE_STATEMENT,
    NODE_SWITCH_STATEMENT,
    NODE_CASE_BLOCK
} NodeType;

// Структура AST узла
typedef struct ASTNode {
    NodeType type;
    int line;
    int column;
    union {
        struct {
            int value;
        } number;
        struct {
            char* name;
        } identifier;
        struct {
            char* value;
        } string;
        struct {
            int op;
            struct ASTNode* operand;
        } unary;
        struct {
            int op;
            struct ASTNode* left;
            struct ASTNode* right;
        } binary;
        struct {
            struct ASTNode* condition;
            struct ASTNode* true_expr;
            struct ASTNode* false_expr;
        } ternary;
        struct {
            char* var_name;
            struct ASTNode* expression;
        } assignment;
        struct {
            char* func_name;
            struct ASTNode** args;
            int arg_count;
        } call;
        struct {
            char* var_name;
            struct ASTNode* initializer;
            int var_type;
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
            struct ASTNode* init;
            struct ASTNode* condition;
            struct ASTNode* increment;
            struct ASTNode* body;
        } for_stmt;
        struct {
            struct ASTNode* body;
            struct ASTNode* condition;
        } do_while_stmt;
        struct {
            struct ASTNode* expression;
        } return_stmt;
        struct {
            struct ASTNode* expression;
        } expr_stmt;
        struct {
            struct ASTNode* expression;
        } print_stmt;
        struct {
            struct ASTNode* expression;
            struct ASTNode* case_blocks;
        } switch_stmt;
        struct {
            struct ASTNode* condition;
            struct ASTNode* body;
            struct ASTNode* next;
        } case_block;
    };
} ASTNode;

// Структура парсера
typedef struct {
    Lexer* lexer;
    Token current;
    char error_message[256];
    int has_error;
} Parser;

// Функции парсера
void init_parser(Parser* parser, Lexer* lexer);
ASTNode* parse_program(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_expression(Parser* parser);
void free_ast(ASTNode* node);
void print_ast(ASTNode* node, int indent);
const char* parser_get_error(Parser* parser);
int parser_has_error(Parser* parser);

#endif // PARSER_H