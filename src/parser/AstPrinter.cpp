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

void AstPrinter::visit(const Assign& expr) {
    parenthesize("assign", {expr.target.get(), expr.value.get()});
}

void AstPrinter::visit(const ArrayLiteral& expr) {
    result += "[";
    for (size_t i = 0; i < expr.elements.size(); ++i) {
        if (i > 0) result += ", ";
        expr.elements[i]->accept(*this);
    }
    result += "]";
}

void AstPrinter::visit(const DictionaryLiteral& expr) {
    result += "{";
    for (size_t i = 0; i < expr.pairs.size(); ++i) {
        if (i > 0) result += ", ";
        expr.pairs[i].key->accept(*this);
        result += ": ";
        expr.pairs[i].value->accept(*this);
    }
    result += "}";
}

void AstPrinter::visit(const TupleLiteral& expr) {
    result += "(";
    for (size_t i = 0; i < expr.elements.size(); ++i) {
        if (i > 0) result += ", ";
        expr.elements[i]->accept(*this);
    }
    result += ")";
}

void AstPrinter::visit(const IndexAccess& expr) {
    parenthesize("index", {expr.object.get(), expr.index.get()});
}

void AstPrinter::visit(const SubscriptAccess& expr) {
    result += "(subscript ";
    expr.object->accept(*this);
    for (const auto& index : expr.indices) {
        result += " ";
        index->accept(*this);
    }
    result += ")";
}

void AstPrinter::visit(const Call& expr) {
    result += "(call ";
    expr.callee->accept(*this);
    for (const auto& arg : expr.arguments) {
        result += " ";
        arg->accept(*this);
    }
    result += ")";
}

void AstPrinter::visit(const Closure& expr) {
    result += "(closure)";
}

void AstPrinter::visit(const EnumAccess& expr) {
    result += "(enum ." + expr.caseName.lexeme + ")";
}

void AstPrinter::visit(const MemberAccess& expr) {
    parenthesize("member", {expr.object.get()});
    result += "." + expr.member.lexeme;
}

void AstPrinter::visit(const StructInit& expr) {
    result += "(struct " + expr.structName.lexeme + ")";
}

void AstPrinter::visit(const Super& expr) {
    result += "(super." + expr.method.lexeme + ")";
}

void AstPrinter::visit(const StringInterpolation& expr) {
    result += "(interpolation)";
}

void AstPrinter::visit(const OptionalChaining& expr) {
    result += "(optional ";
    expr.object->accept(*this);
    result += "?.";
    expr.accessor->accept(*this);
    result += ")";
}

void AstPrinter::visit(const Range& expr) {
    result += "(range ";
    expr.start->accept(*this);
    if (expr.rangeType == Range::RangeType::HalfOpen) {
        result += "..<";
    } else {
        result += "...";
    }
    expr.end->accept(*this);
    result += ")";
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