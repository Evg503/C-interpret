#pragma once

// Типы узлов AST для выражений
typedef enum {
    NODE_NUMBER,
    NODE_IDENTIFIER,
    NODE_STRING,
    NODE_UNARY_OP,
    NODE_BINARY_OP,
    NODE_TERNARY_OP,    // ? :
    NODE_ASSIGNMENT,
    NODE_CALL,
    NODE_VARIABLE_DECL,
    NODE_STATEMENT_LIST,
    NODE_IF_STATEMENT,
    NODE_WHILE_STATEMENT,
    NODE_FOR_STATEMENT,
    NODE_RETURN_STATEMENT,
    NODE_PRINT_STATEMENT,
    NODE_BLOCK
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
            int op;  // тип операции
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
            int type;  // INT, CHAR и т.д.
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
            struct ASTNode* expression;
        } return_stmt;
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

// Приоритеты операций (чем выше число, тем выше приоритет)
typedef struct {
    int precedence;
    int associativity; // 0 = left, 1 = right
} Precedence;

Precedence get_precedence(TokenType op);

// Прототипы функций парсера
ASTNode* parse_program(Parser* parser);
ASTNode* parse_statement(Parser* parser);
ASTNode* parse_expression(Parser* parser);
ASTNode* parse_assignment(Parser* parser);
ASTNode* parse_ternary(Parser* parser);
ASTNode* parse_logical_or(Parser* parser);
ASTNode* parse_logical_and(Parser* parser);
ASTNode* parse_bitwise_or(Parser* parser);
ASTNode* parse_bitwise_xor(Parser* parser);
ASTNode* parse_bitwise_and(Parser* parser);
ASTNode* parse_equality(Parser* parser);
ASTNode* parse_relational(Parser* parser);
ASTNode* parse_shift(Parser* parser);
ASTNode* parse_additive(Parser* parser);
ASTNode* parse_multiplicative(Parser* parser);
ASTNode* parse_unary(Parser* parser);
ASTNode* parse_primary(Parser* parser);
void free_ast(ASTNode* node);

void init_parser(Parser* parser, Lexer* lexer);
void advance(Parser* parser);
bool match(Parser* parser, TokenType type);
void expect(Parser* parser, TokenType type, const char* error_message);

