#ifndef MINISWIFT_ASTPRINTER_H
#define MINISWIFT_ASTPRINTER_H

#include "AST.h"
#include <string>

namespace miniswift {

class AstPrinter : public ExprVisitor {
public:
    std::string print(const Expr& expr);
    void visit(const Binary& expr) override;
    void visit(const Grouping& expr) override;
    void visit(const Literal& expr) override;
    void visit(const Unary& expr) override;
    void visit(const VarExpr& expr) override;

private:
    void parenthesize(const std::string& name, const std::vector<const Expr*>& exprs);
    std::string result;
};

} // namespace miniswift

#endif // MINISWIFT_ASTPRINTER_H