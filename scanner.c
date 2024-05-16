// scanner.c
#include "scanner.h"
#include <stdbool.h>

typedef struct {
    const char* start;
    const char* current;
    int line;
} Scanner;

// 全局变量
Scanner scanner;

void initScanner(const char* source) {
    // 初始化scanner
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

/***************************************************************************************
 *                                   辅助方法											*
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

// 传入TokenType, 创建对应类型的Token，并返回。
static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

// 遇到不能解析的情况时，我们创建一个ERROR Token. 比如：遇到@，$等符号时，比如字符串，字符没有对应的右引号时。
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
        default: scanner.start = scanner.current; return;  //while的退出条件
        }
    }

    ////1.跳过TOKEN前置的空白字符: ' ', '\r', '\t', '\n'和注释
    //while (peek() == ' ' || peek() == '\r' || peek() == '\t' || peek() == '\n') {
    //    advance();
    //    //若换行，则打印新的行号
    //    if (peek() == '\n') {
    //        scanner.line++;    
    //    }
    //}

    ////2.跳过注释
    //// 注释以'//'开头, 一直到行尾
    //while(peek() == '/' && peekNext() == '/') {
    //    while (peek()!= '\n') {   //注意移动current指针到下一行的行首
    //        advance();
    //    }
    //    advance();
    //    scanner.line++;
    //}

    ////3.跳过注释后TOKEN前的空白字符: ' ', '\r', '\t', '\n'和注释
    //while (peek() == ' ' || peek() == '\r' || peek() == '\t' || peek() == '\n') {
    //    advance();
    //    //若换行，则打印新的行号
    //    if (peek() == '\n') {
    //        scanner.line++;
    //    }
    //}
}

// 参数说明：
// start: 从哪个索引位置开始比较
// length: 要比较字符的长度
// rest: 要比较的内容
// type: 如果完全匹配，则说明是type类型的关键字
static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
    int len = (int)(scanner.current - scanner.start); // TOKEN的长度
    // start + length: 关键字的长度
    if (start + length == len && memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }
    return TOKEN_IDENTIFIER;
}

// 判断当前Token到底是标识符还是关键字
static TokenType identifierType() {
    // 确定identifier类型主要有两种方式：
    // 1. 将所有的关键字放入哈希表中，然后查表确认
    // 2. 将所有的关键字放入Trie树中，然后查表确认
    // Trie树的方式不管是空间上还是时间上都优于哈希表的方式
    int token_len = scanner.current - scanner.start;
    // 用switch语句实现Trie树
    
    //此时scanner.current已经移动到单词尾部
    
    //第一个字母
    switch (scanner.start[0]) {
    case 'b': return checkKeyword(1, 4, "reak", TOKEN_BREAK);
    case 'c':
        if (token_len == 4) {
            //第二个字母
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
            //第二个字母
            switch (scanner.start[1]) {
            case 'i':
                //第三个字母
                switch (scanner.start[2]) {
                case 'g': return checkKeyword(3, 3, "ned", TOKEN_SIGNED);
                case 'z': return checkKeyword(3, 3, "eof", TOKEN_SIZEOF);
                }
            case 't': 
                //第三个字母
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

    // 标识符 identifier
    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    // IDENTIFIER包含: 字母，数字和下划线
    while (isAlpha(peek()) || isDigit(peek())) {
        advance();
    }
    // 这样的Token可能是标识符, 也可能是关键字, identifierType()是用来确定Token类型的
    return makeToken(identifierType());
}

static Token number() {
    // 简单起见，我们将NUMBER的规则定义如下:
    // 1. NUMBER可以包含数字和最多一个'.'号
    // 2. '.'号前面要有数字
    // 3. '.'号后面也要有数字
    // 这些都是合法的NUMBER: 123, 3.14
    // 这些都是不合法的NUMBER: 123., .14

    //跳过所有数字
    while (isDigit(peek())) {
        advance();
    }

    //跳过小数点
    if (peek() == '.') {
        if (isDigit(peekNext())) {
            //跳过小数点
            advance();
            //跳过小数点后所有数字
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
    // 字符串以"开头，以"结尾，而且不能跨行
     
    //跳过开头的左双引号
    advance();

    while (1) {
        if (peek() != '"' && peek() != '\n' && !isAtEnd()) {
            advance();
        }
        if (peek() == '"') {
            advance();  //跳过结束的双引号
            return makeToken(TOKEN_STRING);
        }   
        if (peek() == '\n')   return errorToken("string must be in one line.");
    }
}

static Token character() {
    // 字符'开头，以'结尾，而且不能跨行   
    
    //跳过开头的左单引号
    advance();
    
    //判断中间是标识符
    if (isAlpha(peek()) || isDigit(peek())) {
        advance();
    }

    if (peek() == '\'') {
        advance();
        return makeToken(TOKEN_CHARACTER);
    }else         return errorToken("unterminated character.");
}

/***************************************************************************************
 *                                   	分词											  *
 ***************************************************************************************/

//扫描token
Token scanToken() {
    // 跳过前置空白字符和注释
    skipWhitespace();
    // 记录下一个Token的起始位置
    scanner.start = scanner.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();
    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        //1.单字符TOKEN：single-character tokens 
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
        //2.单字符或双字符TOKEN：one or two characters tokens
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
        //3.多字符TOKEN：various-character tokens
    case '"': return string();
    case '\'': return character();
    }
        //4.其他字符：非法字符
    return errorToken("Unexpected character.");
}