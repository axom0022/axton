#include "axton.h"

static char *inputstr;
static int inputpos;
static int inputline;
static int inputcol;
static int *indentstack;
static int indentdepth;

static char peekchar(void) {
    if (inputpos >= (int)strlen(inputstr)) return 0;
    return inputstr[inputpos];
}

static char nextchar(void) {
    if (inputpos >= (int)strlen(inputstr)) return 0;
    char c = inputstr[inputpos++];
    if (c == '\n') { inputline++; inputcol = 1; }
    else inputcol++;
    return c;
}

static void skipspace(void) {
    while (1) {
        char c = peekchar();
        if (c == ' ' || c == '\r') {
            nextchar();
        } else if (c == '\t') {
            nextchar();
        } else break;
    }
}

static char *readident(void) {
    int start = inputpos;
    while (1) {
        char c = peekchar();
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_') {
            nextchar();
        } else break;
    }
    int len = inputpos - start;
    char *s = malloc(len + 1);
    memcpy(s, inputstr + start, len);
    s[len] = 0;
    return s;
}

static char *readnumber(void) {
    int start = inputpos;
    int dot = 0;
    while (1) {
        char c = peekchar();
        if (c >= '0' && c <= '9') nextchar();
        else if (c == '.' && !dot) { dot = 1; nextchar(); }
        else break;
    }
    int len = inputpos - start;
    char *s = malloc(len + 1);
    memcpy(s, inputstr + start, len);
    s[len] = 0;
    return s;
}

static char *readstring(char quote) {
    nextchar();
    int start = inputpos;
    char *buf = malloc(1024);
    int idx = 0;
    while (1) {
        char c = peekchar();
        if (c == 0) { free(buf); return NULL; }
        if (c == '\\') {
            nextchar();
            c = nextchar();
            if (c == 'n') buf[idx++] = '\n';
            else if (c == 't') buf[idx++] = '\t';
            else if (c == '\\') buf[idx++] = '\\';
            else if (c == '"') buf[idx++] = '"';
            else buf[idx++] = c;
        } else if (c == quote) {
            nextchar();
            buf[idx] = 0;
            return strdup(buf);
        } else {
            buf[idx++] = nextchar();
        }
    }
}

static toktype keywordtype(char *s) {
    if (strcmp(s, "let") == 0) return toklet;
    if (strcmp(s, "const") == 0) return tokconst;
    if (strcmp(s, "fn") == 0) return tokfn;
    if (strcmp(s, "if") == 0) return tokif;
    if (strcmp(s, "else") == 0) return tokelse;
    if (strcmp(s, "elif") == 0) return tokelif;
    if (strcmp(s, "for") == 0) return tokfor;
    if (strcmp(s, "in") == 0) return tokin;
    if (strcmp(s, "while") == 0) return tokwhile;
    if (strcmp(s, "break") == 0) return tokbreak;
    if (strcmp(s, "next") == 0) return toknext;
    if (strcmp(s, "return") == 0) return tokreturn;
    if (strcmp(s, "none") == 0) return toknone;
    if (strcmp(s, "true") == 0) return toktrue;
    if (strcmp(s, "false") == 0) return tokfalse;
    if (strcmp(s, "and") == 0) return tokand;
    if (strcmp(s, "or") == 0) return tokor;
    if (strcmp(s, "not") == 0) return toknot;
    if (strcmp(s, "class") == 0) return tokclass;
    if (strcmp(s, "try") == 0) return tokcry;
    if (strcmp(s, "catch") == 0) return tokcatch;
    if (strcmp(s, "finally") == 0) return tokfinally;
    if (strcmp(s, "throw") == 0) return tokthrow;
    if (strcmp(s, "async") == 0) return tokasync;
    if (strcmp(s, "await") == 0) return tokawait;
    if (strcmp(s, "yield") == 0) return tokyield;
    if (strcmp(s, "import") == 0) return tokimport;
    return tokident;
}

static token nexttokenraw(void) {
    token tok;
    memset(&tok, 0, sizeof(token));
    skipspace();
    
    if (inputpos >= (int)strlen(inputstr)) {
        tok.type = tokeof;
        tok.line = inputline;
        return tok;
    }
    
    char c = peekchar();
    
    if (c == '#') {
        while (inputpos < (int)strlen(inputstr) && peekchar() != '\n') nextchar();
        return nexttokenraw();
    }
    
    if (c == '\n') {
        nextchar();
        tok.type = toknewline;
        tok.line = inputline - 1;
        return tok;
    }
    
    if (c == '"' || c == '\'') {
        tok.text = readstring(c);
        tok.type = tokstring;
        tok.line = inputline;
        return tok;
    }
    
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') {
        tok.text = readident();
        tok.type = keywordtype(tok.text);
        tok.line = inputline;
        return tok;
    }
    
    if (c >= '0' && c <= '9') {
        tok.text = readnumber();
        tok.type = toknumber;
        tok.line = inputline;
        return tok;
    }
    
    nextchar();
    tok.line = inputline;
    
    switch (c) {
        case '+': tok.type = tokplus; break;
        case '-': tok.type = tokminus; break;
        case '*': tok.type = tokstar; break;
        case '/': tok.type = tokslash; break;
        case '%': tok.type = tokpercent; break;
        case '=':
            if (peekchar() == '=') { nextchar(); tok.type = tokeqeq; }
            else tok.type = tokeq;
            break;
        case '!':
            if (peekchar() == '=') { nextchar(); tok.type = tokne; }
            break;
        case '<':
            if (peekchar() == '=') { nextchar(); tok.type = tokle; }
            else tok.type = toklt;
            break;
        case '>':
            if (peekchar() == '=') { nextchar(); tok.type = tokge; }
            else tok.type = tokgt;
            break;
        case '(': tok.type = toklparen; break;
        case ')': tok.type = tokrparen; break;
        case '[': tok.type = toklbracket; break;
        case ']': tok.type = tokrbracket; break;
        case '{': tok.type = toklbrace; break;
        case '}': tok.type = tokrbrace; break;
        case ',': tok.type = tokcomma; break;
        case '.': tok.type = tokdot; break;
        case ':': tok.type = tokcolon; break;
    }
    return tok;
}

token *tokenize(char *src) {
    inputstr = src;
    inputpos = 0;
    inputline = 1;
    inputcol = 1;
    indentstack = malloc(sizeof(int) * 100);
    indentstack[0] = 0;
    indentdepth = 1;
    
    token *arr = malloc(sizeof(token) * 4096);
    int size = 0;
    int lastline = 1;
    
    while (1) {
        token tok = nexttokenraw();
        
        if (tok.type == toknewline) {
            if (lastline == tok.line) continue;
            lastline = tok.line;
            
            int spaces = 0;
            while (inputpos < (int)strlen(inputstr) && inputstr[inputpos] == ' ') {
                spaces++;
                inputpos++;
            }
            if (inputpos < (int)strlen(inputstr) && inputstr[inputpos] == '#') continue;
            if (inputpos < (int)strlen(inputstr) && inputstr[inputpos] != '\n' && inputstr[inputpos] != 0) {
                if (spaces > indentstack[indentdepth - 1]) {
                    indentstack[indentdepth++] = spaces;
                    tok.type = tokindent;
                    arr[size++] = tok;
                } else {
                    while (spaces < indentstack[indentdepth - 1]) {
                        indentdepth--;
                        tok.type = tokdedent;
                        arr[size++] = tok;
                    }
                }
            }
            continue;
        }
        
        if (tok.type == tokeof) {
            while (indentdepth > 1) {
                indentdepth--;
                tok.type = tokdedent;
                arr[size++] = tok;
            }
            arr[size++] = tok;
            break;
        }
        
        arr[size++] = tok;
    }
    
    tcount = size;
    free(indentstack);
    return arr;
}
