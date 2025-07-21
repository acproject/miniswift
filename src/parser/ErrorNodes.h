#ifndef MINISWIFT_ERROR_NODES_H
#define MINISWIFT_ERROR_NODES_H

#include "ExprNodes.h"
#include "StmtNodes.h"
#include "../lexer/Token.h"
#include <vector>
#include <memory>

namespace miniswift {

// Forward declarations
class Expr;
class Stmt;
class ExprVisitor;
class StmtVisitor;

// Throw statement: throw error
struct ThrowStmt : Stmt {
    std::unique_ptr<Expr> error;
    
    explicit ThrowStmt(std::unique_ptr<Expr> errorExpr)
        : error(std::move(errorExpr)) {}
    
    void accept(StmtVisitor& visitor) const override;
    
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<ThrowStmt>(error->clone());
    }
};

// Try expression: try expression
struct TryExpr : Expr {
    std::unique_ptr<Expr> expression;
    bool isOptional; // try? vs try!
    bool isForced;   // try! vs try
    
    TryExpr(std::unique_ptr<Expr> expr, bool optional = false, bool forced = false)
        : expression(std::move(expr)), isOptional(optional), isForced(forced) {}
    
    void accept(ExprVisitor& visitor) const override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<TryExpr>(expression->clone(), isOptional, isForced);
    }
};

// Catch clause for do-catch statements
struct CatchClause {
    Token pattern;                    // Error pattern (can be empty for catch-all)
    std::string errorType;           // Type of error to catch (empty for any)
    Token variable;                  // Variable to bind the error to
    std::unique_ptr<Expr> condition; // Optional where condition
    std::unique_ptr<Stmt> body;      // Catch block body
    
    CatchClause(Token pat, const std::string& type, Token var, 
                std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> catchBody)
        : pattern(pat), errorType(type), variable(var), 
          condition(std::move(cond)), body(std::move(catchBody)) {}
    
    // Copy constructor
    CatchClause(const CatchClause& other)
        : pattern(other.pattern), errorType(other.errorType), variable(other.variable),
          condition(other.condition ? other.condition->clone() : nullptr),
          body(other.body ? other.body->clone() : nullptr) {}
    
    // Move constructor
    CatchClause(CatchClause&& other) noexcept
        : pattern(other.pattern), errorType(std::move(other.errorType)), 
          variable(other.variable), condition(std::move(other.condition)),
          body(std::move(other.body)) {}
    
    // Assignment operators
    CatchClause& operator=(const CatchClause& other) {
        if (this != &other) {
            pattern = other.pattern;
            errorType = other.errorType;
            variable = other.variable;
            condition = other.condition ? other.condition->clone() : nullptr;
            body = other.body ? other.body->clone() : nullptr;
        }
        return *this;
    }
    
    CatchClause& operator=(CatchClause&& other) noexcept {
        if (this != &other) {
            pattern = other.pattern;
            errorType = std::move(other.errorType);
            variable = other.variable;
            condition = std::move(other.condition);
            body = std::move(other.body);
        }
        return *this;
    }
};

// Do-catch statement: do { ... } catch { ... }
struct DoCatchStmt : Stmt {
    std::unique_ptr<Stmt> doBody;
    std::vector<CatchClause> catchClauses;
    
    DoCatchStmt(std::unique_ptr<Stmt> body, std::vector<CatchClause> clauses)
        : doBody(std::move(body)), catchClauses(std::move(clauses)) {}
    
    void accept(StmtVisitor& visitor) const override;
    
    std::unique_ptr<Stmt> clone() const override {
        std::vector<CatchClause> clonedClauses;
        for (const auto& clause : catchClauses) {
            clonedClauses.emplace_back(clause); // Uses copy constructor
        }
        return std::make_unique<DoCatchStmt>(doBody->clone(), std::move(clonedClauses));
    }
};

// Defer statement: defer { ... }
struct DeferStmt : Stmt {
    std::unique_ptr<Stmt> body;
    
    explicit DeferStmt(std::unique_ptr<Stmt> deferBody)
        : body(std::move(deferBody)) {}
    
    void accept(StmtVisitor& visitor) const override;
    
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<DeferStmt>(body->clone());
    }
};

// Guard statement: guard condition else { ... }
struct GuardStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> elseBody;
    
    GuardStmt(std::unique_ptr<Expr> cond, std::unique_ptr<Stmt> body)
        : condition(std::move(cond)), elseBody(std::move(body)) {}
    
    void accept(StmtVisitor& visitor) const override;
    
    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<GuardStmt>(condition->clone(), elseBody->clone());
    }
};

// Result type expression: Result<T, Error>
struct ResultTypeExpr : Expr {
    std::unique_ptr<Expr> successType;
    std::unique_ptr<Expr> errorType;
    
    ResultTypeExpr(std::unique_ptr<Expr> success, std::unique_ptr<Expr> error)
        : successType(std::move(success)), errorType(std::move(error)) {}
    
    void accept(ExprVisitor& visitor) const override;
    
    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<ResultTypeExpr>(successType->clone(), errorType->clone());
    }
};

// Error literal expression for creating error instances
struct ErrorLiteral : Expr {
    std::string errorType;
    std::string message;
    std::vector<std::unique_ptr<Expr>> arguments;
    
    ErrorLiteral(const std::string& type, const std::string& msg, 
                 std::vector<std::unique_ptr<Expr>> args = {})
        : errorType(type), message(msg), arguments(std::move(args)) {}
    
    void accept(ExprVisitor& visitor) const override;
    
    std::unique_ptr<Expr> clone() const override {
        std::vector<std::unique_ptr<Expr>> clonedArgs;
        for (const auto& arg : arguments) {
            clonedArgs.push_back(arg->clone());
        }
        return std::make_unique<ErrorLiteral>(errorType, message, std::move(clonedArgs));
    }
};

} // namespace miniswift

#endif // MINISWIFT_ERROR_NODES_H