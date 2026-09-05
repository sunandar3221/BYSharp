#ifndef BYSHARP_PARSER_HPP
#define BYSHARP_PARSER_HPP

#include "Common.hpp"
#include "AST.hpp"
#include <vector>
#include <memory>
#include <unordered_set>

class Parser {
public:
    explicit Parser(std::vector<Token> toks);
    std::vector<std::unique_ptr<Stmt>> parseProgram();

private:
    std::vector<Token> tokens;
    size_t current = 0;
    std::unordered_set<std::string> declaredFunctions;

    void preScanFunctions();

    bool isAtEnd() const;
    const Token& peek() const;
    const Token& peekNext() const;
    const Token& previous() const;
    Token advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& errorMsg);

    bool isTypeToken(TokenType type) const;
    bool canStartOperand(TokenType type) const;
    bool isClosingAngle(size_t pos) const;

    std::unique_ptr<Stmt> parseDeclaration();
    std::unique_ptr<Stmt> parseVarDeclaration(DataType type, const std::string& name);
    std::unique_ptr<Stmt> parseFnDeclaration(DataType returnType, const std::string& name);
    std::unique_ptr<Stmt> parseOutputStatement();
    std::unique_ptr<Stmt> parseRawOutputStatement();
    std::unique_ptr<Stmt> parseIfStatement();
    std::unique_ptr<Stmt> parseWhileStatement();
    std::unique_ptr<BlockStmt> parseBlock();
    std::unique_ptr<Stmt> parseReturnStatement();
    std::unique_ptr<Stmt> parseStatement();

    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseAssignment();
    std::unique_ptr<Expr> parseLogicalOr();
    std::unique_ptr<Expr> parseLogicalAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseRelational();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();

    DataType tokenToDataType(TokenType type) const;
};

#endif
