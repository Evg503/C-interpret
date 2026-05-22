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
    {"for", TOKEN_FOR},
    {"return", TOKEN_RETURN},
    {"int", TOKEN_INT},
    {"char", TOKEN_CHAR},
    {"void", TOKEN_VOID},
    {"print", TOKEN_PRINT},
    {"break", TOKEN_BREAK},
    {"continue", TOKEN_CONTINUE},
    {NULL, TOKEN_EOF}
};

// Инициализация лексера
void init_lexer(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->col = 1;
}

void skip_whitespace(Lexer* lexer) {
    while (lexer->source[lexer->pos]) {
        char c = lexer->source[lexer->pos];
        if (c == ' ' || c == '\t' || c == '\r') {
            lexer->pos++;
            lexer->col++;
        } else if (c == '\n') {
            lexer->pos++;
            lexer->line++;
            lexer->col = 1;
        } else if (c == '/') {
            if (lexer->source[lexer->pos + 1] == '/') {
                while (lexer->source[lexer->pos] && lexer->source[lexer->pos] != '\n')
                    lexer->pos++;
            } else if (lexer->source[lexer->pos + 1] == '*') {
                // Многострочный комментарий
                lexer->pos += 2;
                while (lexer->source[lexer->pos] && 
                       !(lexer->source[lexer->pos] == '*' && lexer->source[lexer->pos + 1] == '/')) {
                    if (lexer->source[lexer->pos] == '\n') {
                        lexer->line++;
                        lexer->col = 1;
                    } else {
                        lexer->col++;
                    }
                    lexer->pos++;
                }
                if (lexer->source[lexer->pos]) {
                    lexer->pos += 2;
                }
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
    
    // Строки
    if (c == '"') {
        lexer->pos++;
        int start = lexer->pos;
        while (lexer->source[lexer->pos] && lexer->source[lexer->pos] != '"')
            lexer->pos++;
        int len = lexer->pos - start;
        char* str = (char*)malloc(len + 1);
        strncpy(str, lexer->source + start, len);
        str[len] = '\0';
        lexer->pos++;
        token.type = TOKEN_STRING;
        token.value = str;
        return token;
    }
    
    // Идентификаторы и ключевые слова
    if (isalpha(c) || c == '_') {
        char* id = read_identifier(lexer);
        
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
    
    // Операторы (многосимвольные проверяем первыми)
    lexer->pos++;
    lexer->col++;
    
    switch (c) {
        case '+':
            if (lexer->source[lexer->pos] == '+') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_PLUS_PLUS;
            } else {
                token.type = TOKEN_PLUS;
            }
            break;
        case '-':
            if (lexer->source[lexer->pos] == '-') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_MINUS_MINUS;
            } else {
                token.type = TOKEN_MINUS;
            }
            break;
        case '*':
            token.type = TOKEN_STAR;
            break;
        case '/':
            token.type = TOKEN_SLASH;
            break;
        case '%':
            token.type = TOKEN_PERCENT;
            break;
        case '=':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_EQ;
            } else {
                token.type = TOKEN_ASSIGN;
            }
            break;
        case '<':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_LE;
            } else if (lexer->source[lexer->pos] == '<') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_SHIFT_LEFT;
            } else {
                token.type = TOKEN_LT;
            }
            break;
        case '>':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_GE;
            } else if (lexer->source[lexer->pos] == '>') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_SHIFT_RIGHT;
            } else {
                token.type = TOKEN_GT;
            }
            break;
        case '!':
            if (lexer->source[lexer->pos] == '=') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_NEQ;
            } else {
                token.type = TOKEN_NOT;
            }
            break;
        case '&':
            if (lexer->source[lexer->pos] == '&') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_AND;
            } else {
                token.type = TOKEN_BIT_AND;
            }
            break;
        case '|':
            if (lexer->source[lexer->pos] == '|') {
                lexer->pos++;
                lexer->col++;
                token.type = TOKEN_OR;
            } else {
                token.type = TOKEN_BIT_OR;
            }
            break;
        case '^':
            token.type = TOKEN_BIT_XOR;
            break;
        case '~':
            token.type = TOKEN_BIT_NOT;
            break;
        case '?':
            token.type = TOKEN_OR; // Временно, нужно добавить TOKEN_QUESTION
            break;
        case ':':
            token.type = TOKEN_ASSIGN; // Временно
            break;
        case ';': token.type = TOKEN_SEMICOLON; break;
        case ',': token.type = TOKEN_COMMA; break;
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        case '{': token.type = TOKEN_LBRACE; break;
        case '}': token.type = TOKEN_RBRACE; break;
        case '[': token.type = TOKEN_LBRACKET; break;
        case ']': token.type = TOKEN_RBRACKET; break;
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