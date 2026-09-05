#include "Parser.hpp"

Parser::Parser(std::vector<Token> toks)
    : tokens(std::move(toks)) {
    preScanFunctions();
}

void Parser::preScanFunctions() {
    declaredFunctions.insert("len");
    declaredFunctions.insert("to_int");
    declaredFunctions.insert("to_float");
    declaredFunctions.insert("to_str");
    declaredFunctions.insert("to_byte");
    declaredFunctions.insert("ord");
    declaredFunctions.insert("chr");
    declaredFunctions.insert("abs");
    declaredFunctions.insert("min");
    declaredFunctions.insert("max");
    declaredFunctions.insert("pow");
    declaredFunctions.insert("sqrt");
    declaredFunctions.insert("floor");
    declaredFunctions.insert("ceil");
    declaredFunctions.insert("round");
    declaredFunctions.insert("sin");
    declaredFunctions.insert("cos");
    declaredFunctions.insert("tan");
    declaredFunctions.insert("log");
    declaredFunctions.insert("log10");
    declaredFunctions.insert("random");
    declaredFunctions.insert("substr");
    declaredFunctions.insert("find");
    declaredFunctions.insert("contains");
    declaredFunctions.insert("replace");
    declaredFunctions.insert("upper");
    declaredFunctions.insert("lower");
    declaredFunctions.insert("trim");
    declaredFunctions.insert("to_bin");
    declaredFunctions.insert("from_bin");
    declaredFunctions.insert("time");
    declaredFunctions.insert("clock");
    declaredFunctions.insert("sleep");
    declaredFunctions.insert("file_exists");
    declaredFunctions.insert("file_read");
    declaredFunctions.insert("file_write");
    declaredFunctions.insert("file_append");
    declaredFunctions.insert("file_delete");
    declaredFunctions.insert("file_remove");
    declaredFunctions.insert("file_size");
    declaredFunctions.insert("file_copy");
    declaredFunctions.insert("file_rename");
    declaredFunctions.insert("dir_create");
    declaredFunctions.insert("dir_exists");
    declaredFunctions.insert("dir_remove");
    declaredFunctions.insert("dir_list");
    declaredFunctions.insert("console_clear");
    declaredFunctions.insert("io_flush");
    declaredFunctions.insert("io_read_line");
    declaredFunctions.insert("sys_exec");
    declaredFunctions.insert("http_get");
    declaredFunctions.insert("http_post");
    declaredFunctions.insert("url_encode");
    declaredFunctions.insert("url_decode");
    declaredFunctions.insert("net_ip_lookup");
    declaredFunctions.insert("net_connect");
    declaredFunctions.insert("net_send");
    declaredFunctions.insert("net_recv");
    declaredFunctions.insert("net_close");
    declaredFunctions.insert("net_listen");
    declaredFunctions.insert("net_accept");
    declaredFunctions.insert("io_read_char");
    declaredFunctions.insert("io_getch");
    declaredFunctions.insert("io_kbhit");
    declaredFunctions.insert("console_title");
    declaredFunctions.insert("console_cursor");
    declaredFunctions.insert("console_color");
    declaredFunctions.insert("env_get");
    declaredFunctions.insert("env_set");
    declaredFunctions.insert("path_join");
    declaredFunctions.insert("path_ext");
    declaredFunctions.insert("path_stem");
    declaredFunctions.insert("file_is_dir");
    declaredFunctions.insert("file_is_file");
    declaredFunctions.insert("file_lines_count");

    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].type == TokenType::TOKEN_AT) {
            if (i + 2 < tokens.size() && tokens[i + 2].type == TokenType::TOKEN_LANGLE && tokens[i + 1].type == TokenType::TOKEN_IDENTIFIER) {
                declaredFunctions.insert(tokens[i + 1].lexeme);
            } else if (i + 3 < tokens.size() && tokens[i + 3].type == TokenType::TOKEN_LANGLE && tokens[i + 2].type == TokenType::TOKEN_IDENTIFIER) {
                declaredFunctions.insert(tokens[i + 2].lexeme);
            }
        }
    }
}

bool Parser::isAtEnd() const {
    return peek().type == TokenType::TOKEN_EOF;
}

const Token& Parser::peek() const {
    return tokens[current];
}

const Token& Parser::peekNext() const {
    if (current + 1 < tokens.size()) {
        return tokens[current + 1];
    }
    return tokens.back();
}

const Token& Parser::previous() const {
    return tokens[current - 1];
}

Token Parser::advance() {
    if (!isAtEnd()) current++;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    if (type == TokenType::TOKEN_RANGLE && peek().type == TokenType::TOKEN_SHR) return true;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        if (type == TokenType::TOKEN_RANGLE && !isAtEnd() && tokens[current].type == TokenType::TOKEN_SHR) {
            tokens[current].type = TokenType::TOKEN_RANGLE;
            tokens[current].lexeme = ">";
            return true;
        }
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& errorMsg) {
    if (type == TokenType::TOKEN_RANGLE && !isAtEnd() && tokens[current].type == TokenType::TOKEN_SHR) {
        tokens[current].type = TokenType::TOKEN_RANGLE;
        tokens[current].lexeme = ">";
        Token t{TokenType::TOKEN_RANGLE, ">", Value::makeVoid(), tokens[current].line, tokens[current].column};
        return t;
    }
    if (check(type)) return advance();
    throw SyntaxError(errorMsg, peek().line, peek().column);
}

bool Parser::isTypeToken(TokenType type) const {
    return type == TokenType::TOKEN_KW_INT ||
           type == TokenType::TOKEN_KW_FLOAT ||
           type == TokenType::TOKEN_KW_BOOL ||
           type == TokenType::TOKEN_KW_STR ||
           type == TokenType::TOKEN_KW_BYTE ||
           type == TokenType::TOKEN_KW_VOID ||
           type == TokenType::TOKEN_HASH ||
           type == TokenType::TOKEN_TILDE ||
           type == TokenType::TOKEN_CARET ||
           type == TokenType::TOKEN_STAR ||
           type == TokenType::TOKEN_PERCENT ||
           type == TokenType::TOKEN_UNDERSCORE;
}

bool Parser::canStartOperand(TokenType type) const {
    return type == TokenType::TOKEN_INT_LITERAL ||
           type == TokenType::TOKEN_FLOAT_LITERAL ||
           type == TokenType::TOKEN_STR_LITERAL ||
           type == TokenType::TOKEN_TRUE ||
           type == TokenType::TOKEN_FALSE ||
           type == TokenType::TOKEN_IDENTIFIER ||
           type == TokenType::TOKEN_MINUS ||
           type == TokenType::TOKEN_NOT ||
           type == TokenType::TOKEN_HASH ||
           type == TokenType::TOKEN_CARET ||
           type == TokenType::TOKEN_INPUT ||
           type == TokenType::TOKEN_LPAREN ||
           type == TokenType::TOKEN_LANGLE;
}

bool Parser::isClosingAngle(size_t pos) const {
    if (pos >= tokens.size()) return false;
    if (tokens[pos].type != TokenType::TOKEN_RANGLE && tokens[pos].type != TokenType::TOKEN_SHR) {
        return false;
    }
    if (tokens[pos].type == TokenType::TOKEN_SHR) {
        return true;
    }
    size_t nextPos = pos + 1;
    if (nextPos >= tokens.size()) {
        return true;
    }
    TokenType nextType = tokens[nextPos].type;
    return nextType == TokenType::TOKEN_LBRACKET ||
           nextType == TokenType::TOKEN_SEMICOLON ||
           nextType == TokenType::TOKEN_COMMA ||
           nextType == TokenType::TOKEN_RBRACKET ||
           nextType == TokenType::TOKEN_RPAREN ||
           nextType == TokenType::TOKEN_RANGLE ||
           nextType == TokenType::TOKEN_SHR ||
           nextType == TokenType::TOKEN_ARROW ||
           nextType == TokenType::TOKEN_ASSIGN ||
           nextType == TokenType::TOKEN_EOF;
}

DataType Parser::tokenToDataType(TokenType type) const {
    switch (type) {
        case TokenType::TOKEN_KW_INT:
        case TokenType::TOKEN_HASH:
            return DataType::INT;
        case TokenType::TOKEN_KW_FLOAT:
        case TokenType::TOKEN_TILDE:
            return DataType::FLOAT;
        case TokenType::TOKEN_KW_BOOL:
        case TokenType::TOKEN_CARET:
            return DataType::BOOL;
        case TokenType::TOKEN_KW_STR:
        case TokenType::TOKEN_STAR:
            return DataType::STR;
        case TokenType::TOKEN_KW_BYTE:
        case TokenType::TOKEN_PERCENT:
            return DataType::BYTE;
        case TokenType::TOKEN_KW_VOID:
        case TokenType::TOKEN_UNDERSCORE:
            return DataType::VOID;
        default:
            throw SyntaxError("Syntax Error: Expected valid type glyph or name.", peek().line, peek().column);
    }
}

std::unique_ptr<Stmt> Parser::parseDeclaration() {
    advance();
    DataType type = DataType::VOID;
    std::string name;

    if (isTypeToken(peek().type)) {
        type = tokenToDataType(advance().type);
        name = consume(TokenType::TOKEN_IDENTIFIER, "Syntax Error: Expected identifier after type in declaration.").lexeme;
    } else if (check(TokenType::TOKEN_IDENTIFIER)) {
        name = advance().lexeme;
        type = DataType::VOID;
    } else {
        throw SyntaxError("Syntax Error: Expected type or function name after '@'", peek().line, peek().column);
    }

    if (check(TokenType::TOKEN_LANGLE)) {
        return parseFnDeclaration(type, name);
    } else {
        return parseVarDeclaration(type, name);
    }
}

std::unique_ptr<Stmt> Parser::parseVarDeclaration(DataType type, const std::string& name) {
    std::unique_ptr<Expr> init = nullptr;
    if (match(TokenType::TOKEN_ASSIGN)) {
        init = parseExpression();
    }
    consume(TokenType::TOKEN_SEMICOLON, "Syntax Error: Expected ';' after variable declaration.");
    return std::make_unique<VarDeclStmt>(type, name, std::move(init));
}

std::unique_ptr<Stmt> Parser::parseFnDeclaration(DataType returnType, const std::string& name) {
    declaredFunctions.insert(name);
    consume(TokenType::TOKEN_LANGLE, "Syntax Error: Expected '<' for parameter list.");
    std::vector<std::pair<DataType, std::string>> params;
    if (!check(TokenType::TOKEN_RANGLE)) {
        do {
            DataType pType = tokenToDataType(advance().type);
            std::string pName = consume(TokenType::TOKEN_IDENTIFIER, "Syntax Error: Expected parameter name.").lexeme;
            params.push_back({pType, pName});
        } while (match(TokenType::TOKEN_COMMA));
    }
    consume(TokenType::TOKEN_RANGLE, "Syntax Error: Expected '>' to close parameter list.");

    if (match(TokenType::TOKEN_ARROW)) {
        returnType = tokenToDataType(advance().type);
    }

    auto body = parseBlock();
    return std::make_unique<FnDeclStmt>(returnType, name, params, std::move(body));
}

std::unique_ptr<Stmt> Parser::parseOutputStatement() {
    advance();
    std::unique_ptr<Expr> expr;
    if (check(TokenType::TOKEN_LANGLE)) {
        advance();
        expr = parseExpression();
        consume(TokenType::TOKEN_RANGLE, "Syntax Error: Expected '>' after output expression.");
        consume(TokenType::TOKEN_SEMICOLON, "Syntax Error: Expected ';' after output statement.");
    } else {
        expr = parseExpression();
        consume(TokenType::TOKEN_SEMICOLON, "Syntax Error: Expected ';' after output statement.");
    }
    return std::make_unique<OutputStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseRawOutputStatement() {
    advance();
    std::unique_ptr<Expr> expr;
    if (check(TokenType::TOKEN_LANGLE)) {
        advance();
        expr = parseExpression();
        consume(TokenType::TOKEN_RANGLE, "Syntax Error: Expected '>' after output expression.");
        consume(TokenType::TOKEN_SEMICOLON, "Syntax Error: Expected ';' after output statement.");
    } else {
        expr = parseExpression();
        consume(TokenType::TOKEN_SEMICOLON, "Syntax Error: Expected ';' after output statement.");
    }
    return std::make_unique<RawOutputStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseIfStatement() {
    advance();
    consume(TokenType::TOKEN_LANGLE, "Syntax Error: Expected '<' before condition.");
    auto condition = parseExpression();
    consume(TokenType::TOKEN_RANGLE, "Syntax Error: Expected '>' after condition.");
    auto thenBranch = parseBlock();

    std::unique_ptr<Stmt> elseBranch = nullptr;
    if (check(TokenType::TOKEN_NOT)) {
        advance();
        if (check(TokenType::TOKEN_QUESTION)) {
            elseBranch = parseIfStatement();
        } else {
            elseBranch = parseBlock();
        }
    }
    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

std::unique_ptr<Stmt> Parser::parseWhileStatement() {
    advance();
    std::unique_ptr<Expr> condition;
    if (check(TokenType::TOKEN_LANGLE)) {
        advance();
        condition = parseExpression();
        consume(TokenType::TOKEN_RANGLE, "Syntax Error: Expected '>' after loop condition.");
    } else if (check(TokenType::TOKEN_LBRACKET)) {
        condition = std::make_unique<LiteralExpr>(Value::makeBool(true));
    } else {
        throw SyntaxError("Syntax Error: Expected '<' condition or '[' block after '~'.", peek().line, peek().column);
    }
    auto body = parseBlock();
    return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    consume(TokenType::TOKEN_LBRACKET, "Syntax Error: Expected '[' to begin block.");
    std::vector<std::unique_ptr<Stmt>> statements;
    while (!check(TokenType::TOKEN_RBRACKET) && !isAtEnd()) {
        statements.push_back(parseStatement());
    }
    consume(TokenType::TOKEN_RBRACKET, "Syntax Error: Expected ']' to close block.");
    return std::make_unique<BlockStmt>(std::move(statements));
}

std::unique_ptr<Stmt> Parser::parseReturnStatement() {
    advance();
    std::unique_ptr<Expr> expr = nullptr;
    if (!check(TokenType::TOKEN_SEMICOLON)) {
        expr = parseExpression();
    }
    consume(TokenType::TOKEN_SEMICOLON, "Syntax Error: Expected ';' after return statement.");
    return std::make_unique<ReturnStmt>(std::move(expr));
}

std::unique_ptr<Stmt> Parser::parseStatement() {
    if (check(TokenType::TOKEN_AT)) {
        return parseDeclaration();
    }
    if (check(TokenType::TOKEN_SHR)) {
        return parseRawOutputStatement();
    }
    if (check(TokenType::TOKEN_RANGLE)) {
        return parseOutputStatement();
    }
    if (check(TokenType::TOKEN_QUESTION)) {
        return parseIfStatement();
    }
    if (check(TokenType::TOKEN_TILDE)) {
        return parseWhileStatement();
    }
    if (check(TokenType::TOKEN_ARROW)) {
        return parseReturnStatement();
    }
    if (check(TokenType::TOKEN_LBRACKET)) {
        return parseBlock();
    }
    if (match(TokenType::TOKEN_KW_BREAK)) {
        consume(TokenType::TOKEN_SEMICOLON, "Syntax Error: Expected ';' after break.");
        return std::make_unique<BreakStmt>();
    }
    if (match(TokenType::TOKEN_KW_CONTINUE)) {
        consume(TokenType::TOKEN_SEMICOLON, "Syntax Error: Expected ';' after continue.");
        return std::make_unique<ContinueStmt>();
    }
    if (check(TokenType::TOKEN_IDENTIFIER) && peekNext().type == TokenType::TOKEN_ASSIGN) {
        std::string name = advance().lexeme;
        advance();
        auto val = parseExpression();
        consume(TokenType::TOKEN_SEMICOLON, "Syntax Error: Expected ';' after assignment.");
        return std::make_unique<ExprStmt>(std::make_unique<AssignExpr>(name, std::move(val)));
    }
    auto expr = parseExpression();
    consume(TokenType::TOKEN_SEMICOLON, "Syntax Error: Expected ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::vector<std::unique_ptr<Stmt>> Parser::parseProgram() {
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!isAtEnd()) {
        stmts.push_back(parseStatement());
    }
    return stmts;
}

std::unique_ptr<Expr> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<Expr> Parser::parseAssignment() {
    auto expr = parseLogicalOr();
    if (match(TokenType::TOKEN_ASSIGN)) {
        auto val = parseAssignment();
        if (auto v = dynamic_cast<VariableExpr*>(expr.get())) {
            return std::make_unique<AssignExpr>(v->name, std::move(val));
        }
        throw SyntaxError("Syntax Error: Invalid assignment target.", peek().line, peek().column);
    }
    return expr;
}

std::unique_ptr<Expr> Parser::parseLogicalOr() {
    auto left = parseLogicalAnd();
    while (match(TokenType::TOKEN_OR)) {
        TokenType op = previous().type;
        auto right = parseLogicalAnd();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseLogicalAnd() {
    auto left = parseEquality();
    while (match(TokenType::TOKEN_AND)) {
        TokenType op = previous().type;
        auto right = parseEquality();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto left = parseRelational();
    while (match(TokenType::TOKEN_EQ) || match(TokenType::TOKEN_NEQ)) {
        TokenType op = previous().type;
        auto right = parseRelational();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseRelational() {
    auto left = parseAdditive();
    while (!isAtEnd()) {
        if (check(TokenType::TOKEN_LE) || check(TokenType::TOKEN_GE)) {
            TokenType op = advance().type;
            auto right = parseAdditive();
            left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
        } else if (check(TokenType::TOKEN_LANGLE)) {
            TokenType op = advance().type;
            auto right = parseAdditive();
            left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
        } else if (check(TokenType::TOKEN_RANGLE)) {
            if (isClosingAngle(current)) {
                break;
            }
            TokenType op = advance().type;
            auto right = parseAdditive();
            left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
        } else {
            break;
        }
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseAdditive() {
    auto left = parseMultiplicative();
    while (match(TokenType::TOKEN_PLUS) || match(TokenType::TOKEN_MINUS)) {
        TokenType op = previous().type;
        auto right = parseMultiplicative();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseMultiplicative() {
    auto left = parseUnary();
    while (match(TokenType::TOKEN_STAR) || match(TokenType::TOKEN_SLASH) || match(TokenType::TOKEN_PERCENT)) {
        TokenType op = previous().type;
        auto right = parseUnary();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenType::TOKEN_NOT) || match(TokenType::TOKEN_MINUS) || match(TokenType::TOKEN_HASH)) {
        TokenType op = previous().type;
        auto right = parseUnary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    std::unique_ptr<Expr> expr;

    if (match(TokenType::TOKEN_INPUT)) {
        expr = std::make_unique<InputExpr>();
    } else if (match(TokenType::TOKEN_CARET)) {
        expr = std::make_unique<LiteralExpr>(Value::makeBool(true));
    } else if (check(TokenType::TOKEN_INT_LITERAL) || check(TokenType::TOKEN_FLOAT_LITERAL) ||
        check(TokenType::TOKEN_STR_LITERAL) || check(TokenType::TOKEN_TRUE) ||
        check(TokenType::TOKEN_FALSE)) {
        expr = std::make_unique<LiteralExpr>(advance().literal);
    } else if (check(TokenType::TOKEN_IDENTIFIER)) {
        std::string name = advance().lexeme;
        if (check(TokenType::TOKEN_LANGLE) && declaredFunctions.count(name) > 0) {
            advance();
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::TOKEN_RANGLE)) {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenType::TOKEN_COMMA));
            }
            consume(TokenType::TOKEN_RANGLE, "Syntax Error: Expected '>' to close function call.");
            expr = std::make_unique<CallExpr>(name, std::move(args));
        } else if (match(TokenType::TOKEN_LPAREN)) {
            std::vector<std::unique_ptr<Expr>> args;
            if (!check(TokenType::TOKEN_RPAREN)) {
                do {
                    args.push_back(parseExpression());
                } while (match(TokenType::TOKEN_COMMA));
            }
            consume(TokenType::TOKEN_RPAREN, "Syntax Error: Expected ')' to close function call.");
            expr = std::make_unique<CallExpr>(name, std::move(args));
        } else {
            expr = std::make_unique<VariableExpr>(name);
        }
    } else if (match(TokenType::TOKEN_LANGLE)) {
        expr = parseExpression();
        consume(TokenType::TOKEN_RANGLE, "Syntax Error: Expected '>' after grouped expression.");
    } else if (match(TokenType::TOKEN_LPAREN)) {
        expr = parseExpression();
        consume(TokenType::TOKEN_RPAREN, "Syntax Error: Expected ')' after expression.");
    } else {
        throw SyntaxError("Syntax Error: Unexpected token '" + peek().lexeme + "'", peek().line, peek().column);
    }

    while (match(TokenType::TOKEN_LBRACKET)) {
        auto idx = parseExpression();
        consume(TokenType::TOKEN_RBRACKET, "Syntax Error: Expected ']' after index.");
        expr = std::make_unique<IndexExpr>(std::move(expr), std::move(idx));
    }

    return expr;
}
