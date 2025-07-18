#include "AstPrinter.h"

namespace miniswift {

std::string AstPrinter::print(const Expr& expr) {
    expr.accept(*this);
    return result;
}

void AstPrinter::visit(const Binary& expr) {
    parenthesize(expr.op.lexeme, {expr.left.get(), expr.right.get()});
}

void AstPrinter::visit(const Grouping& expr) {
    parenthesize("group", {expr.expression.get()});
}

void AstPrinter::visit(const Literal& expr) {
    result += expr.value.lexeme;
}

void AstPrinter::visit(const Unary& expr) {
    parenthesize(expr.op.lexeme, {expr.right.get()});
}

void AstPrinter::visit(const VarExpr& expr) {
    result += expr.name.lexeme;
}

void AstPrinter::parenthesize(const std::string& name, const std::vector<const Expr*>& exprs) {
    result += "(" + name;
    for (const auto& expr : exprs) {
        result += " ";
        expr->accept(*this);
    }
    result += ")";
}

} // namespace miniswift