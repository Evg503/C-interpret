#pragma once

// Типы токенов (расширенные)
typedef enum {
    TOKEN_EOF = 0,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    
    // Операторы
    TOKEN_ASSIGN,       // =
    TOKEN_PLUS,         // +
    TOKEN_MINUS,        // -
    TOKEN_STAR,         // *
    TOKEN_SLASH,        // /
    TOKEN_PERCENT,      // %
    TOKEN_PLUS_PLUS,    // ++
    TOKEN_MINUS_MINUS,  // --
    
    // Сравнение
    TOKEN_EQ,           // ==
    TOKEN_NEQ,          // !=
    TOKEN_LT,           // <
    TOKEN_GT,           // >
    TOKEN_LE,           // <=
    TOKEN_GE,           // >=
    
    // Логические
    TOKEN_AND,          // &&
    TOKEN_OR,           // ||
    TOKEN_NOT,          // !
    
    // Битовые
    TOKEN_BIT_AND,      // &
    TOKEN_BIT_OR,       // |
    TOKEN_BIT_XOR,      // ^
    TOKEN_BIT_NOT,      // ~
    TOKEN_SHIFT_LEFT,   // <<
    TOKEN_SHIFT_RIGHT,  // >>
    
    // Разделители
    TOKEN_SEMICOLON,    // ;
    TOKEN_COMMA,        // ,
    TOKEN_LPAREN,       // (
    TOKEN_RPAREN,       // )
    TOKEN_LBRACE,       // {
    TOKEN_RBRACE,       // }
    TOKEN_LBRACKET,     // [
    TOKEN_RBRACKET,     // ]
    
    // Ключевые слова
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_FOR,
    TOKEN_RETURN,
    TOKEN_INT,
    TOKEN_CHAR,
    TOKEN_VOID,
    TOKEN_PRINT,
    TOKEN_BREAK,
    TOKEN_CONTINUE
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

