#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Типы токенов
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
    TOKEN_QUESTION,     // ?
    TOKEN_COLON,        // :
    
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
    char* value;
    int line;
    int column;
} Token;

// Структура лексера
typedef struct {
    const char* source;
    int pos;
    int line;
    int col;
} Lexer;

// Функции лексера
void init_lexer(Lexer* lexer, const char* source);
Token get_next_token(Lexer* lexer);
void print_token(Token* token);
void free_token(Token* token);

#endif // LEXER_H