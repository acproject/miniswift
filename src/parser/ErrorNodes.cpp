#include "ErrorNodes.h"
#include "../interpreter/Interpreter.h"

namespace miniswift {

// ThrowStmt accept implementation
void ThrowStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

// TryExpr accept implementation
void TryExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

// DoCatchStmt accept implementation
void DoCatchStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

// DeferStmt accept implementation
void DeferStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

// GuardStmt accept implementation
void GuardStmt::accept(StmtVisitor& visitor) const {
    visitor.visit(*this);
}

// ResultTypeExpr accept implementation
void ResultTypeExpr::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

// ErrorLiteral accept implementation
void ErrorLiteral::accept(ExprVisitor& visitor) const {
    visitor.visit(*this);
}

} // namespace miniswift