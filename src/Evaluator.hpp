#ifndef BYSHARP_EVALUATOR_HPP
#define BYSHARP_EVALUATOR_HPP

#include "Common.hpp"
#include "AST.hpp"
#include "Environment.hpp"
#include <memory>
#include <vector>

class Evaluator : public ASTVisitor {
public:
    std::shared_ptr<Environment> environment;

    explicit Evaluator(std::shared_ptr<Environment> env = nullptr);

    Value evaluate(Expr* expr);
    void execute(Stmt* stmt);
    void executeProgram(const std::vector<std::unique_ptr<Stmt>>& program);

    Value visitLiteralExpr(LiteralExpr* expr) override;
    Value visitVariableExpr(VariableExpr* expr) override;
    Value visitUnaryExpr(UnaryExpr* expr) override;
    Value visitBinaryExpr(BinaryExpr* expr) override;
    Value visitAssignExpr(AssignExpr* expr) override;
    Value visitCallExpr(CallExpr* expr) override;
    Value visitIndexExpr(IndexExpr* expr) override;
    Value visitInputExpr(InputExpr* expr) override;

    void visitExprStmt(ExprStmt* stmt) override;
    void visitVarDeclStmt(VarDeclStmt* stmt) override;
    void visitOutputStmt(OutputStmt* stmt) override;
    void visitRawOutputStmt(RawOutputStmt* stmt) override;
    void visitIfStmt(IfStmt* stmt) override;
    void visitWhileStmt(WhileStmt* stmt) override;
    void visitBlockStmt(BlockStmt* stmt) override;
    void visitReturnStmt(ReturnStmt* stmt) override;
    void visitBreakStmt(BreakStmt* stmt) override;
    void visitContinueStmt(ContinueStmt* stmt) override;
    void visitFnDeclStmt(FnDeclStmt* stmt) override;

private:
    void registerNatives();
    bool isNumeric(const Value& val) const;
    double toDouble(const Value& val) const;
    int64_t toInt64(const Value& val) const;
};

#endif
