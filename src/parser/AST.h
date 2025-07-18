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
struct ArrayLiteral;
struct DictionaryLiteral;
struct IndexAccess;

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
    virtual void visit(const ArrayLiteral& expr) = 0;
    virtual void visit(const DictionaryLiteral& expr) = 0;
    virtual void visit(const IndexAccess& expr) = 0;
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

// Array literal expression: [1, 2, 3]
struct ArrayLiteral : Expr {
    explicit ArrayLiteral(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        std::vector<std::unique_ptr<Expr>> clonedElements;
        for (const auto& element : elements) {
            clonedElements.push_back(element->clone());
        }
        return std::make_unique<ArrayLiteral>(std::move(clonedElements));
    }

    const std::vector<std::unique_ptr<Expr>> elements;
};

// Dictionary literal expression: ["key": value]
struct DictionaryLiteral : Expr {
    struct KeyValuePair {
        std::unique_ptr<Expr> key;
        std::unique_ptr<Expr> value;
        
        KeyValuePair(std::unique_ptr<Expr> k, std::unique_ptr<Expr> v)
            : key(std::move(k)), value(std::move(v)) {}
    };
    
    explicit DictionaryLiteral(std::vector<KeyValuePair> pairs)
        : pairs(std::move(pairs)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        std::vector<KeyValuePair> clonedPairs;
        for (const auto& pair : pairs) {
            clonedPairs.emplace_back(pair.key->clone(), pair.value->clone());
        }
        return std::make_unique<DictionaryLiteral>(std::move(clonedPairs));
    }

    const std::vector<KeyValuePair> pairs;
};

// Index access expression: array[index] or dict["key"]
struct IndexAccess : Expr {
    IndexAccess(std::unique_ptr<Expr> object, std::unique_ptr<Expr> index)
        : object(std::move(object)), index(std::move(index)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<IndexAccess>(object->clone(), index->clone());
    }

    const std::unique_ptr<Expr> object;
    const std::unique_ptr<Expr> index;
};

// Forward declarations for Stmt
struct ExprStmt;
struct PrintStmt; // For testing
struct VarStmt;
struct BlockStmt;
struct IfStmt;
struct WhileStmt;
struct ForStmt;

// Visitor for Stmt
class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visit(const ExprStmt& stmt) = 0;
    virtual void visit(const PrintStmt& stmt) = 0;
    virtual void visit(const VarStmt& stmt) = 0;
    virtual void visit(const BlockStmt& stmt) = 0;
    virtual void visit(const IfStmt& stmt) = 0;
    virtual void visit(const WhileStmt& stmt) = 0;
    virtual void visit(const ForStmt& stmt) = 0;
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

// Block statement: { statements }
struct BlockStmt : Stmt {
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : statements(std::move(statements)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::vector<std::unique_ptr<Stmt>> statements;
};

// If statement: if condition { thenBranch } else { elseBranch }
struct IfStmt : Stmt {
    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch)
        : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::unique_ptr<Expr> condition;
    const std::unique_ptr<Stmt> thenBranch;
    const std::unique_ptr<Stmt> elseBranch; // Can be null
};

// While statement: while condition { body }
struct WhileStmt : Stmt {
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
        : condition(std::move(condition)), body(std::move(body)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::unique_ptr<Expr> condition;
    const std::unique_ptr<Stmt> body;
};

// For statement: for initializer; condition; increment { body }
struct ForStmt : Stmt {
    ForStmt(std::unique_ptr<Stmt> initializer, std::unique_ptr<Expr> condition, 
            std::unique_ptr<Expr> increment, std::unique_ptr<Stmt> body)
        : initializer(std::move(initializer)), condition(std::move(condition)), 
          increment(std::move(increment)), body(std::move(body)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    const std::unique_ptr<Stmt> initializer; // Can be null
    const std::unique_ptr<Expr> condition;   // Can be null
    const std::unique_ptr<Expr> increment;   // Can be null
    const std::unique_ptr<Stmt> body;
};

} // namespace miniswift

#endif // MINISWIFT_AST_H