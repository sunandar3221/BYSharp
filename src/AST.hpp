#ifndef BYSHARP_AST_HPP
#define BYSHARP_AST_HPP

#include "Common.hpp"
#include <string>
#include <vector>
#include <memory>
#include <utility>

class LiteralExpr;
class VariableExpr;
class UnaryExpr;
class BinaryExpr;
class AssignExpr;
class CallExpr;
class IndexExpr;
class InputExpr;

class ExprStmt;
class VarDeclStmt;
class OutputStmt;
class RawOutputStmt;
class IfStmt;
class WhileStmt;
class BlockStmt;
class ReturnStmt;
class BreakStmt;
class ContinueStmt;
class FnDeclStmt;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual Value visitLiteralExpr(LiteralExpr* expr) = 0;
    virtual Value visitVariableExpr(VariableExpr* expr) = 0;
    virtual Value visitUnaryExpr(UnaryExpr* expr) = 0;
    virtual Value visitBinaryExpr(BinaryExpr* expr) = 0;
    virtual Value visitAssignExpr(AssignExpr* expr) = 0;
    virtual Value visitCallExpr(CallExpr* expr) = 0;
    virtual Value visitIndexExpr(IndexExpr* expr) = 0;
    virtual Value visitInputExpr(InputExpr* expr) = 0;

    virtual void visitExprStmt(ExprStmt* stmt) = 0;
    virtual void visitVarDeclStmt(VarDeclStmt* stmt) = 0;
    virtual void visitOutputStmt(OutputStmt* stmt) = 0;
    virtual void visitRawOutputStmt(RawOutputStmt* stmt) = 0;
    virtual void visitIfStmt(IfStmt* stmt) = 0;
    virtual void visitWhileStmt(WhileStmt* stmt) = 0;
    virtual void visitBlockStmt(BlockStmt* stmt) = 0;
    virtual void visitReturnStmt(ReturnStmt* stmt) = 0;
    virtual void visitBreakStmt(BreakStmt* stmt) = 0;
    virtual void visitContinueStmt(ContinueStmt* stmt) = 0;
    virtual void visitFnDeclStmt(FnDeclStmt* stmt) = 0;
};

class Expr {
public:
    virtual ~Expr() = default;
    virtual Value accept(ASTVisitor* visitor) = 0;
};

class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(ASTVisitor* visitor) = 0;
};

class LiteralExpr : public Expr {
public:
    Value value;
    explicit LiteralExpr(Value val) : value(std::move(val)) {}
    Value accept(ASTVisitor* visitor) override {
        return visitor->visitLiteralExpr(this);
    }
};

class VariableExpr : public Expr {
public:
    std::string name;
    explicit VariableExpr(std::string n) : name(std::move(n)) {}
    Value accept(ASTVisitor* visitor) override {
        return visitor->visitVariableExpr(this);
    }
};

class UnaryExpr : public Expr {
public:
    TokenType op;
    std::unique_ptr<Expr> operand;
    UnaryExpr(TokenType o, std::unique_ptr<Expr> opnd)
        : op(o), operand(std::move(opnd)) {}
    Value accept(ASTVisitor* visitor) override {
        return visitor->visitUnaryExpr(this);
    }
};

class BinaryExpr : public Expr {
public:
    std::unique_ptr<Expr> left;
    TokenType op;
    std::unique_ptr<Expr> right;
    BinaryExpr(std::unique_ptr<Expr> l, TokenType o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
    Value accept(ASTVisitor* visitor) override {
        return visitor->visitBinaryExpr(this);
    }
};

class AssignExpr : public Expr {
public:
    std::string name;
    std::unique_ptr<Expr> value;
    AssignExpr(std::string n, std::unique_ptr<Expr> v)
        : name(std::move(n)), value(std::move(v)) {}
    Value accept(ASTVisitor* visitor) override {
        return visitor->visitAssignExpr(this);
    }
};

class CallExpr : public Expr {
public:
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;
    CallExpr(std::string c, std::vector<std::unique_ptr<Expr>> a)
        : callee(std::move(c)), args(std::move(a)) {}
    Value accept(ASTVisitor* visitor) override {
        return visitor->visitCallExpr(this);
    }
};

class IndexExpr : public Expr {
public:
    std::unique_ptr<Expr> target;
    std::unique_ptr<Expr> index;
    IndexExpr(std::unique_ptr<Expr> t, std::unique_ptr<Expr> idx)
        : target(std::move(t)), index(std::move(idx)) {}
    Value accept(ASTVisitor* visitor) override {
        return visitor->visitIndexExpr(this);
    }
};

class InputExpr : public Expr {
public:
    Value accept(ASTVisitor* visitor) override {
        return visitor->visitInputExpr(this);
    }
};

class ExprStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void accept(ASTVisitor* visitor) override {
        visitor->visitExprStmt(this);
    }
};

class VarDeclStmt : public Stmt {
public:
    DataType type;
    std::string name;
    std::unique_ptr<Expr> init;
    VarDeclStmt(DataType t, std::string n, std::unique_ptr<Expr> i)
        : type(t), name(std::move(n)), init(std::move(i)) {}
    void accept(ASTVisitor* visitor) override {
        visitor->visitVarDeclStmt(this);
    }
};

class OutputStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    explicit OutputStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void accept(ASTVisitor* visitor) override {
        visitor->visitOutputStmt(this);
    }
};

class RawOutputStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    explicit RawOutputStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
    void accept(ASTVisitor* visitor) override {
        visitor->visitRawOutputStmt(this);
    }
};

class IfStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch;
    IfStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> thenB, std::unique_ptr<Stmt> elseB = nullptr)
        : condition(std::move(cond)), thenBranch(std::move(thenB)), elseBranch(std::move(elseB)) {}
    void accept(ASTVisitor* visitor) override {
        visitor->visitIfStmt(this);
    }
};

class WhileStmt : public Stmt {
public:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
    WhileStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> b)
        : condition(std::move(cond)), body(std::move(b)) {}
    void accept(ASTVisitor* visitor) override {
        visitor->visitWhileStmt(this);
    }
};

class BlockStmt : public Stmt {
public:
    std::vector<std::unique_ptr<Stmt>> statements;
    BlockStmt() = default;
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts)
        : statements(std::move(stmts)) {}
    void accept(ASTVisitor* visitor) override {
        visitor->visitBlockStmt(this);
    }
};

class ReturnStmt : public Stmt {
public:
    std::unique_ptr<Expr> expr;
    explicit ReturnStmt(std::unique_ptr<Expr> e = nullptr) : expr(std::move(e)) {}
    void accept(ASTVisitor* visitor) override {
        visitor->visitReturnStmt(this);
    }
};

class BreakStmt : public Stmt {
public:
    void accept(ASTVisitor* visitor) override {
        visitor->visitBreakStmt(this);
    }
};

class ContinueStmt : public Stmt {
public:
    void accept(ASTVisitor* visitor) override {
        visitor->visitContinueStmt(this);
    }
};

class FnDeclStmt : public Stmt {
public:
    DataType returnType;
    std::string name;
    std::vector<std::pair<DataType, std::string>> params;
    std::shared_ptr<BlockStmt> body;
    FnDeclStmt(DataType ret, std::string n, std::vector<std::pair<DataType, std::string>> p, std::shared_ptr<BlockStmt> b)
        : returnType(ret), name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
    void accept(ASTVisitor* visitor) override {
        visitor->visitFnDeclStmt(this);
    }
};

#endif
