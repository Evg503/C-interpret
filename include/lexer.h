#pragma once

// Типы токенов
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_KEYWORD,
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_STAR,      // *
    TOKEN_SLASH,     // /
    TOKEN_ASSIGN,    // =
    TOKEN_SEMICOLON, // ;
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_LT,        // <
    TOKEN_GT,        // >
    TOKEN_EQ,        // ==
    TOKEN_NEQ,       // !=
    TOKEN_LE,        // <=
    TOKEN_GE,        // >=
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_INT,
    TOKEN_PRINT
} TokenType;

// Структура токена
typedef struct {
    TokenType type;
    char* value;     // для IDENTIFIER, NUMBER
    int line;
    int column;
} Token;

// Структура лексера
typedef struct {
    const char* source;
    int pos;
    int line;
    int col;
    Token current_token;
} Lexer;

// Ключевые слова
typedef struct {
    const char* word;
    TokenType type;
} Keyword;


Token get_next_token(Lexer* lexer);
void init_lexer(Lexer* lexer, const char* source);
void print_token(Token* token);

