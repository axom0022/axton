#include "axton.h"

static char *input_str;
static int input_pos;
static int input_line;
static int input_col;
static int *indent_stack;
static int indent_depth;

static char peek_char(void) {
    if (input_pos >= (int)strlen(input_str)) return 0;
    return input_str[input_pos];
}

static char next_char(void) {
    if (input_pos >= (int)strlen(input_str)) return 0;
    char c = input_str[input_pos++];
    if (c == '\n') { input_line++; input_col = 1; }
    else input_col++;
    return c;
}

static void skip_space(void) {
    while (1) {
        char c = peek_char();
        if (c == ' ' || c == '\r') {
            next_char();
        } else if (c == '\t') {
            next_char();
        } else break;
    }
}

static char *read_ident(void) {
    int start = input_pos;
    while (1) {
        char c = peek_char();
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_') {
            next_char();
        } else break;
    }
    int len = input_pos - start;
    char *s = malloc(len + 1);
    memcpy(s, input_str + start, len);
    s[len] = 0;
    return s;
}

static char *read_number(void) {
    int start = input_pos;
    int dot = 0;
    while (1) {
        char c = peek_char();
        if (c >= '0' && c <= '9') next_char();
        else if (c == '.' && !dot) { dot = 1; next_char(); }
        else break;
    }
    int len = input_pos - start;
    char *s = malloc(len + 1);
    memcpy(s, input_str + start, len);
    s[len] = 0;
    return s;
}

static char *read_string(char quote) {
    next_char();
    int start = input_pos;
    char *buf = malloc(1024);
    int idx = 0;
    while (1) {
        char c = peek_char();
        if (c == 0) { free(buf); return NULL; }
        if (c == '\\') {
            next_char();
            c = next_char();
            if (c == 'n') buf[idx++] = '\n';
            else if (c == 't') buf[idx++] = '\t';
            else if (c == '\\') buf[idx++] = '\\';
            else if (c == '"') buf[idx++] = '"';
            else buf[idx++] = c;
        } else if (c == quote) {
            next_char();
            buf[idx] = 0;
            return strdup(buf);
        } else {
            buf[idx++] = next_char();
        }
    }
}

static toktype keyword_type(char *s) {
    if (strcmp(s, "let") == 0) return TOK_LET;
    if (strcmp(s, "const") == 0) return TOK_CONST;
    if (strcmp(s, "fn") == 0) return TOK_FN;
    if (strcmp(s, "if") == 0) return TOK_IF;
    if (strcmp(s, "else") == 0) return TOK_ELSE;
    if (strcmp(s, "elif") == 0) return TOK_ELIF;
    if (strcmp(s, "for") == 0) return TOK_FOR;
    if (strcmp(s, "in") == 0) return TOK_IN;
    if (strcmp(s, "while") == 0) return TOK_WHILE;
    if (strcmp(s, "break") == 0) return TOK_BREAK;
    if (strcmp(s, "next") == 0) return TOK_NEXT;
    if (strcmp(s, "return") == 0) return TOK_RETURN;
    if (strcmp(s, "none") == 0) return TOK_NONE;
    if (strcmp(s, "true") == 0) return TOK_TRUE;
    if (strcmp(s, "false") == 0) return TOK_FALSE;
    if (strcmp(s, "and") == 0) return TOK_AND;
    if (strcmp(s, "or") == 0) return TOK_OR;
    if (strcmp(s, "not") == 0) return TOK_NOT;
    if (strcmp(s, "class") == 0) return TOK_CLASS;
    if (strcmp(s, "try") == 0) return TOK_TRY;
    if (strcmp(s, "catch") == 0) return TOK_CATCH;
    if (strcmp(s, "finally") == 0) return TOK_FINALLY;
    if (strcmp(s, "throw") == 0) return TOK_THROW;
    if (strcmp(s, "async") == 0) return TOK_ASYNC;
    if (strcmp(s, "await") == 0) return TOK_AWAIT;
    if (strcmp(s, "yield") == 0) return TOK_YIELD;
    if (strcmp(s, "import") == 0) return TOK_IMPORT;
    return TOK_IDENT;
}

static token next_token_raw(void) {
    token tok;
    memset(&tok, 0, sizeof(token));
    skip_space();
    
    if (input_pos >= (int)strlen(input_str)) {
        tok.type = TOK_EOF;
        tok.line = input_line;
        return tok;
    }
    
    char c = peek_char();
    
    if (c == '#') {
        while (input_pos < (int)strlen(input_str) && peek_char() != '\n') next_char();
        return next_token_raw();
    }
    
    if (c == '\n') {
        next_char();
        tok.type = TOK_NEWLINE;
        tok.line = input_line - 1;
        return tok;
    }
    
    if (c == '"' || c == '\'') {
        tok.text = read_string(c);
        tok.type = TOK_STRING;
        tok.line = input_line;
        return tok;
    }
    
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        tok.text = read_ident();
        tok.type = keyword_type(tok.text);
        tok.line = input_line;
        return tok;
    }
    
    if (c >= '0' && c <= '9') {
        tok.text = read_number();
        tok.type = TOK_NUMBER;
        tok.line = input_line;
        return tok;
    }
    
    next_char();
    tok.line = input_line;
    
    switch (c) {
        case '+': tok.type = TOK_PLUS; break;
        case '-': tok.type = TOK_MINUS; break;
        case '*': tok.type = TOK_STAR; break;
        case '/': tok.type = TOK_SLASH; break;
        case '%': tok.type = TOK_PERCENT; break;
        case '=':
            if (peek_char() == '=') { next_char(); tok.type = TOK_EQEQ; }
            else tok.type = TOK_EQ;
            break;
        case '!':
            if (peek_char() == '=') { next_char(); tok.type = TOK_NE; }
            break;
        case '<':
            if (peek_char() == '=') { next_char(); tok.type = TOK_LE; }
            else tok.type = TOK_LT;
            break;
        case '>':
            if (peek_char() == '=') { next_char(); tok.type = TOK_GE; }
            else tok.type = TOK_GT;
            break;
        case '(': tok.type = TOK_LPAREN; break;
        case ')': tok.type = TOK_RPAREN; break;
        case '[': tok.type = TOK_LBRACKET; break;
        case ']': tok.type = TOK_RBRACKET; break;
        case '{': tok.type = TOK_LBRACE; break;
        case '}': tok.type = TOK_RBRACE; break;
        case ',': tok.type = TOK_COMMA; break;
        case '.': tok.type = TOK_DOT; break;
        case ':': tok.type = TOK_COLON; break;
    }
    return tok;
}

token *tokenize(char *src) {
    input_str = src;
    input_pos = 0;
    input_line = 1;
    input_col = 1;
    indent_stack = malloc(sizeof(int) * 100);
    indent_stack[0] = 0;
    indent_depth = 1;
    
    token *arr = malloc(sizeof(token) * 4096);
    int size = 0;
    int last_line = 1;
    
    while (1) {
        token tok = next_token_raw();
        
        if (tok.type == TOK_NEWLINE) {
            if (last_line == tok.line) continue;
            last_line = tok.line;
            
            int spaces = 0;
            while (input_pos < (int)strlen(input_str) && input_str[input_pos] == ' ') {
                spaces++;
                input_pos++;
            }
            if (input_pos < (int)strlen(input_str) && input_str[input_pos] == '#') continue;
            if (input_pos < (int)strlen(input_str) && input_str[input_pos] != '\n' && input_str[input_pos] != 0) {
                if (spaces > indent_stack[indent_depth - 1]) {
                    indent_stack[indent_depth++] = spaces;
                    tok.type = TOK_INDENT;
                    arr[size++] = tok;
                } else {
                    while (spaces < indent_stack[indent_depth - 1]) {
                        indent_depth--;
                        tok.type = TOK_DEDENT;
                        arr[size++] = tok;
                    }
                }
            }
            continue;
        }
        
        if (tok.type == TOK_EOF) {
            while (indent_depth > 1) {
                indent_depth--;
                tok.type = TOK_DEDENT;
                arr[size++] = tok;
            }
            arr[size++] = tok;
            break;
        }
        
        arr[size++] = tok;
    }
    
    tcount = size;
    free(indent_stack);
    return arr;
}
