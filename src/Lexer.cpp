#include "Lexer.hpp"
#include <cctype>

Lexer::Lexer(std::string src)
    : source(std::move(src)) {
    keywords["int"] = TokenType::TOKEN_KW_INT;
    keywords["float"] = TokenType::TOKEN_KW_FLOAT;
    keywords["bool"] = TokenType::TOKEN_KW_BOOL;
    keywords["str"] = TokenType::TOKEN_KW_STR;
    keywords["byte"] = TokenType::TOKEN_KW_BYTE;
    keywords["void"] = TokenType::TOKEN_KW_VOID;
    keywords["true"] = TokenType::TOKEN_TRUE;
    keywords["false"] = TokenType::TOKEN_FALSE;
    keywords["v"] = TokenType::TOKEN_FALSE;
    keywords["break"] = TokenType::TOKEN_KW_BREAK;
    keywords["continue"] = TokenType::TOKEN_KW_CONTINUE;
    keywords["exit"] = TokenType::TOKEN_KW_EXIT;
}

bool Lexer::isAtEnd() const {
    return current >= source.size();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source[current];
}

char Lexer::peekNext() const {
    if (current + 1 >= source.size()) return '\0';
    return source[current + 1];
}

char Lexer::advance() {
    char c = source[current++];
    column++;
    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source[current] != expected) return false;
    current++;
    column++;
    return true;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '\n') {
            line++;
            column = 1;
            current++;
        } else {
            break;
        }
    }
}

Token Lexer::scanNumber() {
    int startCol = column;
    size_t startPos = current;
    while (!isAtEnd() && std::isdigit(peek())) {
        advance();
    }
    bool isFloat = false;
    if (!isAtEnd() && peek() == '.' && std::isdigit(peekNext())) {
        isFloat = true;
        advance();
        while (!isAtEnd() && std::isdigit(peek())) {
            advance();
        }
    }
    std::string text = source.substr(startPos, current - startPos);
    if (isFloat) {
        double val = std::stod(text);
        return Token{TokenType::TOKEN_FLOAT_LITERAL, text, Value::makeFloat(val), line, startCol};
    } else {
        int64_t val = std::stoll(text);
        return Token{TokenType::TOKEN_INT_LITERAL, text, Value::makeInt(val), line, startCol};
    }
}

Token Lexer::scanIdentifier() {
    int startCol = column;
    size_t startPos = current;
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        advance();
    }
    std::string text = source.substr(startPos, current - startPos);
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        if (it->second == TokenType::TOKEN_TRUE) {
            return Token{TokenType::TOKEN_TRUE, text, Value::makeBool(true), line, startCol};
        }
        if (it->second == TokenType::TOKEN_FALSE) {
            return Token{TokenType::TOKEN_FALSE, text, Value::makeBool(false), line, startCol};
        }
        return Token{it->second, text, Value::makeVoid(), line, startCol};
    }
    return Token{TokenType::TOKEN_IDENTIFIER, text, Value::makeVoid(), line, startCol};
}

Token Lexer::scanBinaryString() {
    int startLine = line;
    int startCol = column;
    advance();
    advance();
    std::string bits = "";
    bool closed = false;
    while (!isAtEnd()) {
        if (peek() == ':' && peekNext() == ']') {
            advance();
            advance();
            closed = true;
            break;
        }
        char c = peek();
        if (c == '\n') {
            line++;
            column = 1;
            current++;
        } else if (c == '0' || c == '1') {
            bits.push_back(c);
            advance();
        } else if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else {
            throw SyntaxError("Syntax Error: Invalid Binary String Literal - unexpected character '" + std::string(1, c) + "' inside [: :]", line, column);
        }
    }
    if (!closed) {
        throw SyntaxError("Syntax Error: Invalid Binary String Literal - unclosed binary string literal", startLine, startCol);
    }
    if (bits.length() % 8 != 0) {
        throw SyntaxError("Syntax Error: Invalid Binary String Literal - bit count (" + std::to_string(bits.length()) + ") must be a multiple of 8", startLine, startCol);
    }
    std::string decoded = "";
    for (size_t i = 0; i < bits.length(); i += 8) {
        uint8_t byteVal = 0;
        for (size_t j = 0; j < 8; ++j) {
            byteVal = static_cast<uint8_t>((byteVal << 1) | (bits[i + j] - '0'));
        }
        decoded.push_back(static_cast<char>(byteVal));
    }
    return Token{TokenType::TOKEN_STR_LITERAL, "[:...:]", Value::makeStr(decoded), startLine, startCol};
}

Token Lexer::scanToken() {
    skipWhitespace();
    if (isAtEnd()) {
        return Token{TokenType::TOKEN_EOF, "", Value::makeVoid(), line, column};
    }

    int startCol = column;
    char c = peek();

    if (c == '[' && peekNext() == ':') {
        return scanBinaryString();
    }

    if (c == '"' || c == '\'') {
        throw SyntaxError("Syntax Error: Invalid Binary String Literal - plain text strings are prohibited, all strings must use [: binary :] format", line, column);
    }

    if (std::isdigit(c)) {
        return scanNumber();
    }

    if (c == '_' && !std::isalnum(peekNext()) && peekNext() != '_') {
        advance();
        return Token{TokenType::TOKEN_UNDERSCORE, "_", Value::makeVoid(), line, startCol};
    }

    if (std::isalpha(c) || c == '_') {
        return scanIdentifier();
    }

    c = advance();
    switch (c) {
        case '@':
            return Token{TokenType::TOKEN_AT, "@", Value::makeVoid(), line, startCol};
        case '#':
            return Token{TokenType::TOKEN_HASH, "#", Value::makeVoid(), line, startCol};
        case '^':
            return Token{TokenType::TOKEN_CARET, "^", Value::makeBool(true), line, startCol};
        case '<':
            if (match('-')) {
                return Token{TokenType::TOKEN_ASSIGN, "<-", Value::makeVoid(), line, startCol};
            }
            if (match('?')) {
                return Token{TokenType::TOKEN_INPUT, "<?", Value::makeVoid(), line, startCol};
            }
            if (match('=')) {
                return Token{TokenType::TOKEN_LE, "<=", Value::makeVoid(), line, startCol};
            }
            return Token{TokenType::TOKEN_LANGLE, "<", Value::makeVoid(), line, startCol};
        case '-':
            if (match('>')) {
                return Token{TokenType::TOKEN_ARROW, "->", Value::makeVoid(), line, startCol};
            }
            return Token{TokenType::TOKEN_MINUS, "-", Value::makeVoid(), line, startCol};
        case '>':
            if (match('>')) {
                return Token{TokenType::TOKEN_SHR, ">>", Value::makeVoid(), line, startCol};
            }
            if (match('=')) {
                return Token{TokenType::TOKEN_GE, ">=", Value::makeVoid(), line, startCol};
            }
            return Token{TokenType::TOKEN_RANGLE, ">", Value::makeVoid(), line, startCol};
        case '?':
            return Token{TokenType::TOKEN_QUESTION, "?", Value::makeVoid(), line, startCol};
        case '~':
            return Token{TokenType::TOKEN_TILDE, "~", Value::makeVoid(), line, startCol};
        case '[':
            return Token{TokenType::TOKEN_LBRACKET, "[", Value::makeVoid(), line, startCol};
        case ']':
            return Token{TokenType::TOKEN_RBRACKET, "]", Value::makeVoid(), line, startCol};
        case '(':
            return Token{TokenType::TOKEN_LPAREN, "(", Value::makeVoid(), line, startCol};
        case ')':
            return Token{TokenType::TOKEN_RPAREN, ")", Value::makeVoid(), line, startCol};
        case ';':
            return Token{TokenType::TOKEN_SEMICOLON, ";", Value::makeVoid(), line, startCol};
        case ',':
            return Token{TokenType::TOKEN_COMMA, ",", Value::makeVoid(), line, startCol};
        case '+':
            return Token{TokenType::TOKEN_PLUS, "+", Value::makeVoid(), line, startCol};
        case '*':
            return Token{TokenType::TOKEN_STAR, "*", Value::makeVoid(), line, startCol};
        case '/':
            return Token{TokenType::TOKEN_SLASH, "/", Value::makeVoid(), line, startCol};
        case '%':
            return Token{TokenType::TOKEN_PERCENT, "%", Value::makeVoid(), line, startCol};
        case '=':
            if (match('=')) {
                return Token{TokenType::TOKEN_EQ, "==", Value::makeVoid(), line, startCol};
            }
            throw SyntaxError("Syntax Error: Unexpected '='. Use '<-' for assignment or '==' for equality.", line, startCol);
        case '!':
            if (match('=')) {
                return Token{TokenType::TOKEN_NEQ, "!=", Value::makeVoid(), line, startCol};
            }
            return Token{TokenType::TOKEN_NOT, "!", Value::makeVoid(), line, startCol};
        case '&':
            if (match('&')) {
                return Token{TokenType::TOKEN_AND, "&&", Value::makeVoid(), line, startCol};
            }
            throw SyntaxError("Syntax Error: Unexpected '&'. Did you mean '&&'?", line, startCol);
        case '|':
            if (match('|')) {
                return Token{TokenType::TOKEN_OR, "||", Value::makeVoid(), line, startCol};
            }
            throw SyntaxError("Syntax Error: Unexpected '|'. Did you mean '||'?", line, startCol);
        default:
            throw SyntaxError("Syntax Error: Unrecognized character '" + std::string(1, c) + "'", line, startCol);
    }
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (true) {
        Token tok = scanToken();
        tokens.push_back(tok);
        if (tok.type == TokenType::TOKEN_EOF) {
            break;
        }
    }
    return tokens;
}
