#include "lexer.h"

// Ключевые слова
typedef struct {
    const char* word;
    TokenType type;
} Keyword;

static Keyword keywords[] = {
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
    {"switch", TOKEN_SWITCH},
    {"case", TOKEN_CASE},
    {"default", TOKEN_DEFAULT},
    {NULL, TOKEN_EOF}
};

void init_lexer(Lexer* lexer, const char* source) {
    lexer->source = source;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->col = 1;
    lexer->error_message[0] = '\0';
    lexer->has_error = 0;
}

static void skip_whitespace(Lexer* lexer) {
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
                // Однострочный комментарий
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

static char* read_number(Lexer* lexer) {
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

static char* read_string(Lexer* lexer) {
    lexer->pos++; // пропускаем открывающую кавычку
    int start = lexer->pos;
    while (lexer->source[lexer->pos] && lexer->source[lexer->pos] != '"')
        lexer->pos++;
    int len = lexer->pos - start;
    char* str = (char*)malloc(len + 1);
    strncpy(str, lexer->source + start, len);
    str[len] = '\0';
    lexer->pos++; // пропускаем закрывающую кавычку
    lexer->col += len + 2;
    return str;
}

static char* read_identifier(Lexer* lexer) {
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
        token.type = TOKEN_STRING;
        token.value = read_string(lexer);
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
        case '*': token.type = TOKEN_STAR; break;
        case '/': token.type = TOKEN_SLASH; break;
        case '%': token.type = TOKEN_PERCENT; break;
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
        case '^': token.type = TOKEN_BIT_XOR; break;
        case '~': token.type = TOKEN_BIT_NOT; break;
        case '?': token.type = TOKEN_QUESTION; break;
        case ':': token.type = TOKEN_COLON; break;
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

void print_token(Token* token) {
    switch (token->type) {
        case TOKEN_EOF: printf("EOF"); break;
        case TOKEN_IDENTIFIER: printf("IDENTIFIER(%s)", token->value); break;
        case TOKEN_NUMBER: printf("NUMBER(%s)", token->value); break;
        case TOKEN_STRING: printf("STRING(\"%s\")", token->value); break;
        case TOKEN_PLUS: printf("PLUS"); break;
        case TOKEN_MINUS: printf("MINUS"); break;
        case TOKEN_STAR: printf("STAR"); break;
        case TOKEN_SLASH: printf("SLASH"); break;
        case TOKEN_PERCENT: printf("PERCENT"); break;
        case TOKEN_ASSIGN: printf("ASSIGN"); break;
        case TOKEN_PLUS_PLUS: printf("PLUS_PLUS"); break;
        case TOKEN_MINUS_MINUS: printf("MINUS_MINUS"); break;
        case TOKEN_EQ: printf("EQ"); break;
        case TOKEN_NEQ: printf("NEQ"); break;
        case TOKEN_LT: printf("LT"); break;
        case TOKEN_GT: printf("GT"); break;
        case TOKEN_LE: printf("LE"); break;
        case TOKEN_GE: printf("GE"); break;
        case TOKEN_AND: printf("AND"); break;
        case TOKEN_OR: printf("OR"); break;
        case TOKEN_NOT: printf("NOT"); break;
        case TOKEN_BIT_AND: printf("BIT_AND"); break;
        case TOKEN_BIT_OR: printf("BIT_OR"); break;
        case TOKEN_BIT_XOR: printf("BIT_XOR"); break;
        case TOKEN_BIT_NOT: printf("BIT_NOT"); break;
        case TOKEN_SHIFT_LEFT: printf("SHIFT_LEFT"); break;
        case TOKEN_SHIFT_RIGHT: printf("SHIFT_RIGHT"); break;
        case TOKEN_SEMICOLON: printf("SEMICOLON"); break;
        case TOKEN_COMMA: printf("COMMA"); break;
        case TOKEN_LPAREN: printf("LPAREN"); break;
        case TOKEN_RPAREN: printf("RPAREN"); break;
        case TOKEN_LBRACE: printf("LBRACE"); break;
        case TOKEN_RBRACE: printf("RBRACE"); break;
        case TOKEN_LBRACKET: printf("LBRACKET"); break;
        case TOKEN_RBRACKET: printf("RBRACKET"); break;
        case TOKEN_QUESTION: printf("QUESTION"); break;
        case TOKEN_COLON: printf("COLON"); break;
        case TOKEN_IF: printf("IF"); break;
        case TOKEN_ELSE: printf("ELSE"); break;
        case TOKEN_WHILE: printf("WHILE"); break;
        case TOKEN_FOR: printf("FOR"); break;
        case TOKEN_RETURN: printf("RETURN"); break;
        case TOKEN_INT: printf("INT"); break;
        case TOKEN_PRINT: printf("PRINT"); break;
        case TOKEN_BREAK: printf("BREAK"); break;
        case TOKEN_CONTINUE: printf("CONTINUE"); break;
        default: printf("UNKNOWN");
    }
}

void free_token(Token* token) {
    if (token->value) {
        free(token->value);
        token->value = NULL;
    }
}