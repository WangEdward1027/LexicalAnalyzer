// scanner.c
#include "scanner.h"
#include <stdbool.h>

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

// ȫ�ֱ���
Scanner scanner;

void initScanner(const char* source) {
    // ��ʼ��scanner
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

/***************************************************************************************
 *                                   ��������											*
 ***************************************************************************************/

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
        (c >= 'A' && c <= 'Z') ||
        c == '_';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

static char advance() {
    return *scanner.current++;
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return *(scanner.current + 1);
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (peek() != expected) return false;
    scanner.current++;
    return true;
}

// ����TokenType, ������Ӧ���͵�Token�������ء�
static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

// �������ܽ��������ʱ�����Ǵ���һ��ERROR Token. ���磺����@��$�ȷ���ʱ�������ַ������ַ�û�ж�Ӧ��������ʱ��
static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

static void skipWhitespace() {
    while (1) {
        char c = peek();
        switch (c) {
        case '/':
            if (peekNext() == '/')
            {
                while (advance() != '\n');
                scanner.line++;
            }
            break;
        case ' ': advance(); break;
        case '\r':advance(); break;
        case '\t':advance(); break;
        case '\n':scanner.line++; advance(); break;
        default: scanner.start = scanner.current; return;  //while���˳�����
        }
    }

    ////1.����TOKENǰ�õĿհ��ַ�: ' ', '\r', '\t', '\n'��ע��
    //while (peek() == ' ' || peek() == '\r' || peek() == '\t' || peek() == '\n') {
    //    advance();
    //    //�����У����ӡ�µ��к�
    //    if (peek() == '\n') {
    //        scanner.line++;    
    //    }
    //}

    ////2.����ע��
    //// ע����'//'��ͷ, һֱ����β
    //while(peek() == '/' && peekNext() == '/') {
    //    while (peek()!= '\n') {   //ע���ƶ�currentָ�뵽��һ�е�����
    //        advance();
    //    }
    //    advance();
    //    scanner.line++;
    //}

    ////3.����ע�ͺ�TOKENǰ�Ŀհ��ַ�: ' ', '\r', '\t', '\n'��ע��
    //while (peek() == ' ' || peek() == '\r' || peek() == '\t' || peek() == '\n') {
    //    advance();
    //    //�����У����ӡ�µ��к�
    //    if (peek() == '\n') {
    //        scanner.line++;
    //    }
    //}
}

// ����˵����
// start: ���ĸ�����λ�ÿ�ʼ�Ƚ�
// length: Ҫ�Ƚ��ַ��ĳ���
// rest: Ҫ�Ƚϵ�����
// type: �����ȫƥ�䣬��˵����type���͵Ĺؼ���
static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
    int len = (int)(scanner.current - scanner.start); // TOKEN�ĳ���
    // start + length: �ؼ��ֵĳ���
    if (start + length == len && memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

// �жϵ�ǰToken�����Ǳ�ʶ�����ǹؼ���
static TokenType identifierType() {
    // ȷ��identifier������Ҫ�����ַ�ʽ��
    // 1. �����еĹؼ��ַ����ϣ���У�Ȼ����ȷ��
    // 2. �����еĹؼ��ַ���Trie���У�Ȼ����ȷ��
    // Trie���ķ�ʽ�����ǿռ��ϻ���ʱ���϶����ڹ�ϣ��ķ�ʽ
    int token_len = scanner.current - scanner.start;
    // ��switch���ʵ��Trie��
    
    //��ʱscanner.current�Ѿ��ƶ�������β��
    
    //��һ����ĸ
    switch (scanner.start[0]) {
    case 'b': return checkKeyword(1, 4, "reak", TOKEN_BREAK);
    case 'c':
        if (token_len == 4) {
            //�ڶ�����ĸ
            switch (scanner.start[1]) {
            case 'a': return checkKeyword(2, 2, "se", TOKEN_CASE);
            case 'h': return checkKeyword(2, 2, "ar", TOKEN_CHAR);
            }
        }
        else if (token_len == 8) {
            return checkKeyword(1, 7, "ontinue", TOKEN_CONTINUE);
        }
        else if (token_len== 5) {
            return checkKeyword(1, 4, "onst", TOKEN_CONST);
        }
        else {
            break;
        }
    case 'd': 
        if (token_len == 6)        return checkKeyword(1, 5, "ouble", TOKEN_DOUBLE);
        else if (token_len == 7)   return checkKeyword(1, 6, "efault", TOKEN_DEFAULT);
        else break;
    case 'e':   //enum  else
        if (token_len == 4) {      
            switch (scanner.start[1]) {
            case 'n': return checkKeyword(2, 2, "um", TOKEN_ENUM);
            case 'l': return checkKeyword(2, 2, "se", TOKEN_ELSE);
            }
        }
    case 'f':
        if(token_len == 5)        return checkKeyword(1, 4, "loat", TOKEN_FLOAT);
        else if(token_len == 3)   return checkKeyword(1, 2, "or", TOKEN_FOR);
        else break;
    case 'g': return checkKeyword(1, 3, "oto", TOKEN_GOTO);
    case 'i':
        if (token_len == 2)       return checkKeyword(1, 1, "f", TOKEN_IF);
        else if (token_len == 3)  return checkKeyword(1, 2, "nt", TOKEN_INT);
        else break;
    case 'l': return checkKeyword(1, 3, "ong", TOKEN_LONG);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':
        if (token_len == 5) {
            return checkKeyword(1, 4, "ort", TOKEN_SHORT);
        }
        else if (token_len == 6) {
            //�ڶ�����ĸ
            switch (scanner.start[1]) {
            case 'i':
                //��������ĸ
                switch (scanner.start[2]) {
                case 'g': return checkKeyword(3, 3, "ned", TOKEN_SIGNED);
                case 'z': return checkKeyword(3, 3, "eof", TOKEN_SIZEOF);
                }
            case 't': 
                //��������ĸ
                switch (scanner.start[2]) {
                case 'r':  return checkKeyword(3, 3, "uct", TOKEN_STRUCT);
                case 'a':  return checkKeyword(3, 3, "tic", TOKEN_STATIC);
                } 
            case 'w': return checkKeyword(2, 4, "itch", TOKEN_SWITCH);
            }
        }
    case 't': return checkKeyword(1, 6, "ypedef", TOKEN_TYPEDEF);
    case 'u':
        if (token_len == 8)            return checkKeyword(1, 7, "nsigned", TOKEN_UNSIGNED);
        else if(token_len == 5)        return checkKeyword(1, 4,"nion",TOKEN_UNION);
        else  break;
    case 'v': return checkKeyword(1, 3, "oid", TOKEN_VOID);
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }

    // ��ʶ�� identifier
    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    // IDENTIFIER����: ��ĸ�����ֺ��»���
    while (isAlpha(peek()) || isDigit(peek())) {
        advance();
    }
    // ������Token�����Ǳ�ʶ��, Ҳ�����ǹؼ���, identifierType()������ȷ��Token���͵�
    return makeToken(identifierType());
}

static Token number() {
    // ����������ǽ�NUMBER�Ĺ���������:
    // 1. NUMBER���԰������ֺ����һ��'.'��
    // 2. '.'��ǰ��Ҫ������
    // 3. '.'�ź���ҲҪ������
    // ��Щ���ǺϷ���NUMBER: 123, 3.14
    // ��Щ���ǲ��Ϸ���NUMBER: 123., .14

    //������������
    while (isDigit(peek())) {
        advance();
    }

    //����С����
    if (peek() == '.') {
        if (isDigit(peekNext())) {
            //����С����
            advance();
            //����С�������������
            while (isDigit(peek())) {
                advance();
            }
        }
        else {
            return errorToken("Number must have digits after the decimal point.\n");
        }
    }

    if(peek() == ' ' || peek() == '\n' || peek() == '\0' || peek()==';')   return makeToken(TOKEN_NUMBER);
    else                                                    return errorToken("Not a number");
}

static Token string() {
    // �ַ�����"��ͷ����"��β�����Ҳ��ܿ���
     
    //������ͷ����˫����
    advance();

    while (1) {
        if (peek() != '"' && peek() != '\n' && !isAtEnd()) {
            advance();
        }
        if (peek() == '"') {
            advance();  //����������˫����
            return makeToken(TOKEN_STRING);
        }   
        if (peek() == '\n')   return errorToken("string must be in one line.");
    }
}

static Token character() {
    // �ַ�'��ͷ����'��β�����Ҳ��ܿ���   
    
    //������ͷ��������
    advance();
    
    //�ж��м��Ǳ�ʶ��
    if (isAlpha(peek()) || isDigit(peek())) {
        advance();
    }

    if (peek() == '\'') {
        advance();
        return makeToken(TOKEN_CHARACTER);
    }else         return errorToken("unterminated character.");
}

/***************************************************************************************
 *                                   	�ִ�											  *
 ***************************************************************************************/

//ɨ��token
Token scanToken() {
    // ����ǰ�ÿհ��ַ���ע��
    skipWhitespace();
    // ��¼��һ��Token����ʼλ��
    scanner.start = scanner.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();
    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        //1.���ַ�TOKEN��single-character tokens 
    case '(': return makeToken(TOKEN_LEFT_PAREN);
    case ')': return makeToken(TOKEN_RIGHT_PAREN);
    case '[': return makeToken(TOKEN_LEFT_BRACKET);
    case ']': return makeToken(TOKEN_RIGHT_BRACKET);
    case '{': return makeToken(TOKEN_LEFT_BRACE);
    case '}': return makeToken(TOKEN_RIGHT_BRACE);
    case ',': return makeToken(TOKEN_COMMA);
    case '.': return makeToken(TOKEN_DOT);
    case ';': return makeToken(TOKEN_SEMICOLON);
    case '~': return makeToken(TOKEN_TILDE);
    case ':': return makeToken(TOKEN_COLON);
    case '#': return makeToken(TOKEN_ALARM);
        //2.���ַ���˫�ַ�TOKEN��one or two characters tokens
    case '+':
        if (match('+')) return makeToken(TOKEN_PLUS_PLUS);
        else if (match('=')) return makeToken(TOKEN_PLUS_EQUAL);
        else return makeToken(TOKEN_PLUS);
    case '*':
        if (match('=')) return makeToken(TOKEN_STAR_EQUAL);
        else return makeToken(TOKEN_STAR);
    case '/':
        if (match('=')) return makeToken(TOKEN_SLASH_EQUAL);
        else return makeToken(TOKEN_SLASH);
    case '%':
        if (match('=')) return makeToken(TOKEN_PERCENT_EQUAL);
        else return makeToken(TOKEN_PERCENT);
    case '&':
        if (match('=')) return makeToken(TOKEN_AMPER_EQUAL);
        else if (match('&')) return makeToken(TOKEN_AMPER_AMPER);
        else return makeToken(TOKEN_AMPER);
    case '|':
        if (match('=')) return makeToken(TOKEN_PIPE_EQUAL);
        else if (match('|')) return makeToken(TOKEN_PIPE_PIPE);
        else return makeToken(TOKEN_PIPE);
    case '^':
        if (match('=')) return makeToken(TOKEN_HAT_EQUAL);
        else return makeToken(TOKEN_HAT);
    case '=':
        if (match('=')) return makeToken(TOKEN_EQUAL_EQUAL);
        else return makeToken(TOKEN_EQUAL);
    case '!':
        if (match('=')) return makeToken(TOKEN_BANG_EQUAL);
        else return makeToken(TOKEN_BANG);
    case '<':
        if (match('=')) return makeToken(TOKEN_LESS_EQUAL);
        else if (match('<')) return makeToken(TOKEN_LESS_LESS);
        else return makeToken(TOKEN_LESS);
    case '>':
        if (match('=')) return makeToken(TOKEN_GREATER_EQUAL);
        else if (match('>')) return makeToken(TOKEN_GREATER_GREATER);
        else return makeToken(TOKEN_GREATER);
        //3.���ַ�TOKEN��various-character tokens
    case '"': return string();
    case '\'': return character();
    }
        //4.�����ַ����Ƿ��ַ�
    return errorToken("Unexpected character.");
}