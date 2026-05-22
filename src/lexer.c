#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#include "lexer.h"

Keyword keywords[] = {
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"return", TOKEN_RETURN},
    {"int", TOKEN_INT},
    {"print", TOKEN_PRINT},
    {NULL, TOKEN_EOF}
};

// Инициализация лексера
void init_lexer(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->col = 1;
}

// Пропуск пробелов и комментариев
void skip_whitespace(Lexer* lexer) {
    while (lexer->source[lexer->pos]) {
        char c = lexer->source[lexer->pos];
        if (c == ' ' || c == '\t') {
            lexer->pos++;
            lexer->col++;
        } else if (c == '\n') {
            lexer->pos++;
            lexer->line++;
            lexer->col = 1;
        } else if (c == '/') {
            // Проверка на комментарий //
            if (lexer->source[lexer->pos + 1] == '/') {
                while (lexer->source[lexer->pos] && lexer->source[lexer->pos] != '\n')
                    lexer->pos++;
            } else {
                break;
            }
        } else {
            break;
        }
    }
}

// Чтение числа
char* read_number(Lexer* lexer) {
    int start = lexer->pos;
    while (isdigit(lexer->source[lexer->pos]))
        lexer->pos++;
    
    int len = lexer->pos - start;
    char* num = (char*)malloc(len + 1);
    strncpy(num, lexer->source + start, len);
    num[len] = '\0';
    
    lexer->col += len;
    return num;
}

// Чтение идентификатора или ключевого слова
char* read_identifier(Lexer* lexer) {
    int start = lexer->pos;
    while (isalnum(lexer->source[lexer->pos]) || lexer->source[lexer->pos] == '_')
        lexer->pos++;
    
    int len = lexer->pos - start;
    char* id = (char*)malloc(len + 1);
    strncpy(id, lexer->source + start, len);
    id[len] = '\0';
    
    lexer->col += len;
    return id;
}

// Получение следующего токена
Token get_next_token(Lexer* lexer) {
    Token token;
    token.value = NULL;
    token.line = lexer->line;
    token.column = lexer->col;
    
    skip_whitespace(lexer);
    
    char c = lexer->source[lexer->pos];
    
    if (c == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }
    
    // Числа
    if (isdigit(c)) {
        token.type = TOKEN_NUMBER;
        token.value = read_number(lexer);
        return token;
    }
    
    // Идентификаторы и ключевые слова
    if (isalpha(c) || c == '_') {
        char* id = read_identifier(lexer);
        
        // Проверка на ключевое слово
        int i = 0;
        while (keywords[i].word != NULL) {
            if (strcmp(id, keywords[i].word) == 0) {
                token.type = keywords[i].type;
                free(id);
                return token;
            }
            i++;
        }
        
        token.type = TOKEN_IDENTIFIER;
        token.value = id;
        return token;
    }
    
    // Операторы
    lexer->pos++;
    lexer->col++;
    
    switch (c) {
        case '+': token.type = TOKEN_PLUS; break;
        case '-': token.type = TOKEN_MINUS; break;
        case '*': token.type = TOKEN_STAR; break;
        case '/': token.type = TOKEN_SLASH; break;
        case '=':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                token.type = TOKEN_EQ;
            } else {
                token.type = TOKEN_ASSIGN;
            }
            break;
        case '<':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                token.type = TOKEN_LE;
            } else {
                token.type = TOKEN_LT;
            }
            break;
        case '>':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                token.type = TOKEN_GE;
            } else {
                token.type = TOKEN_GT;
            }
            break;
        case '!':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                token.type = TOKEN_NEQ;
            } else {
                printf("Ошибка: неожиданный символ '!'\n");
                token.type = TOKEN_EOF;
            }
            break;
        case ';': token.type = TOKEN_SEMICOLON; break;
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        case '{': token.type = TOKEN_LBRACE; break;
        case '}': token.type = TOKEN_RBRACE; break;
        default:
            printf("Ошибка: неизвестный символ '%c' на строке %d\n", c, lexer->line);
            token.type = TOKEN_EOF;
    }
    
    return token;
}

// Печать токена (для отладки)
void print_token(Token* token) {
    switch (token->type) {
        case TOKEN_EOF: printf("EOF"); break;
        case TOKEN_IDENTIFIER: printf("IDENTIFIER(%s)", token->value); break;
        case TOKEN_NUMBER: printf("NUMBER(%s)", token->value); break;
        case TOKEN_PLUS: printf("PLUS"); break;
        case TOKEN_MINUS: printf("MINUS"); break;
        case TOKEN_STAR: printf("STAR"); break;
        case TOKEN_SLASH: printf("SLASH"); break;
        case TOKEN_ASSIGN: printf("ASSIGN"); break;
        case TOKEN_SEMICOLON: printf("SEMICOLON"); break;
        case TOKEN_LPAREN: printf("LPAREN"); break;
        case TOKEN_RPAREN: printf("RPAREN"); break;
        case TOKEN_LBRACE: printf("LBRACE"); break;
        case TOKEN_RBRACE: printf("RBRACE"); break;
        case TOKEN_IF: printf("IF"); break;
        case TOKEN_ELSE: printf("ELSE"); break;
        case TOKEN_WHILE: printf("WHILE"); break;
        case TOKEN_RETURN: printf("RETURN"); break;
        case TOKEN_INT: printf("INT"); break;
        case TOKEN_LT: printf("LT"); break;
        case TOKEN_GT: printf("GT"); break;
        case TOKEN_EQ: printf("EQ"); break;
        case TOKEN_NEQ: printf("NEQ"); break;
        case TOKEN_LE: printf("LE"); break;
        case TOKEN_GE: printf("GE"); break;
        case TOKEN_PRINT: printf("PRINT"); break;
        default: printf("UNKNOWN");
    }
}