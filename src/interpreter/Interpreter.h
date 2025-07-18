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
    void visit(const BlockStmt& stmt) override;
    void visit(const IfStmt& stmt) override;
    void visit(const WhileStmt& stmt) override;
    void visit(const ForStmt& stmt) override;

    void visit(const Binary& expr) override;
    void visit(const Grouping& expr) override;
    void visit(const Literal& expr) override;
    void visit(const Unary& expr) override;
    void visit(const VarExpr& expr) override;
    void visit(const Assign& expr) override;
    void visit(const ArrayLiteral& expr) override;
    void visit(const DictionaryLiteral& expr) override;
    void visit(const IndexAccess& expr) override;

private:
    std::shared_ptr<Environment> environment;
    Value evaluate(const Expr& expr);
    bool isTruthy(const Value& value);
    void printArray(const Array& arr);
    void printDictionary(const Dictionary& dict);
    void printValue(const Value& val);

    Value result;
};

} // namespace miniswift

#endif // MINISWIFT_INTERPRETER_H