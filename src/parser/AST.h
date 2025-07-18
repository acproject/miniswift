#ifndef MINISWIFT_AST_H
#define MINISWIFT_AST_H

#include "../lexer/Token.h"
#include <vector>
#include <memory>

namespace miniswift {

// Forward declarations
struct Binary;
struct Grouping;
struct Literal;
struct Unary;
struct VarExpr;
struct Assign;

// Visitor interface for expressions
class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual void visit(const Binary& expr) = 0;
    virtual void visit(const Grouping& expr) = 0;
    virtual void visit(const Literal& expr) = 0;
    virtual void visit(const Unary& expr) = 0;
    virtual void visit(const VarExpr& expr) = 0;
    virtual void visit(const Assign& expr) = 0;
};

// Base class for all expression nodes
class Expr {
public:
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor& visitor) const = 0;
    virtual std::unique_ptr<Expr> clone() const = 0;
};

// Concrete expression classes
struct Binary : Expr {
    Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
        : left(std::move(left)), op(op), right(std::move(right)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<Binary>(left->clone(), op, right->clone());
    }

    const std::unique_ptr<Expr> left;
    const Token op;
    const std::unique_ptr<Expr> right;
};

struct VarExpr : Expr {
    explicit VarExpr(Token name) : name(name) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<VarExpr>(name);
    }

    const Token name;
};

struct Assign : Expr {
    Assign(Token name, std::unique_ptr<Expr> value)
        : name(name), value(std::move(value)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<Assign>(name, value->clone());
    }

    const Token name;
    const std::unique_ptr<Expr> value;
};

struct Grouping : Expr {
    explicit Grouping(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<Grouping>(expression->clone());
    }

    const std::unique_ptr<Expr> expression;
};

struct Literal : Expr {
    explicit Literal(Token value) : value(value) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<Literal>(value);
    }

    const Token value;
};

struct Unary : Expr {
    Unary(Token op, std::unique_ptr<Expr> right)
        : op(op), right(std::move(right)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<Unary>(op, right->clone());
    }

    const Token op;
    const std::unique_ptr<Expr> right;
};

// Forward declarations for Stmt
struct ExprStmt;
struct PrintStmt; // For testing
struct VarStmt;

// Visitor for Stmt
class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visit(const ExprStmt& stmt) = 0;
    virtual void visit(const PrintStmt& stmt) = 0;
    virtual void visit(const VarStmt& stmt) = 0;
};

// Base class for Stmt
class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) const = 0;
};

// Concrete statement classes
struct ExprStmt : Stmt {
    explicit ExprStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::unique_ptr<Expr> expression;
};

struct PrintStmt : Stmt {
    explicit PrintStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::unique_ptr<Expr> expression;
};

struct VarStmt : Stmt {
    VarStmt(Token name, std::unique_ptr<Expr> initializer, bool isConst, Token type)
        : name(name), initializer(std::move(initializer)), isConst(isConst), type(type) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    const Token name;
    const std::unique_ptr<Expr> initializer;
    const bool isConst;
    const Token type;
};

} // namespace miniswift

#endif // MINISWIFT_AST_H