#ifndef BYSHARP_LEXER_HPP
#define BYSHARP_LEXER_HPP

#include "Common.hpp"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
public:
    explicit Lexer(std::string src);
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t current = 0;
    int line = 1;
    int column = 1;
    std::unordered_map<std::string, TokenType> keywords;

    bool isAtEnd() const;
    char peek() const;
    char peekNext() const;
    char advance();
    bool match(char expected);
    void skipWhitespace();
    Token scanToken();
    Token scanNumber();
    Token scanIdentifier();
    Token scanBinaryString();
};

#endif
