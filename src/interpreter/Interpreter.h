#ifndef MINISWIFT_INTERPRETER_H
#define MINISWIFT_INTERPRETER_H

#include "../parser/AST.h"
#include "Value.h"
#include "Environment.h"
#include <memory>

namespace miniswift {

class Interpreter : public ExprVisitor, public StmtVisitor {
public:
    Interpreter();
public:
    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements);

    void visit(const ExprStmt& stmt) override;
    void visit(const PrintStmt& stmt) override;
    void visit(const VarStmt& stmt) override;

    void visit(const Binary& expr) override;
    void visit(const Grouping& expr) override;
    void visit(const Literal& expr) override;
 virtual void visit(const Unary& expr) override;
    virtual void visit(const VarExpr& expr) override;
    virtual void visit(const Assign& expr) override;

private:
    std::shared_ptr<Environment> environment;
    Value evaluate(const Expr& expr);
    bool isTruthy(const Value& value);

    Value result;
};

} // namespace miniswift

#endif // MINISWIFT_INTERPRETER_H