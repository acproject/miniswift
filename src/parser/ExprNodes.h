#ifndef MINISWIFT_EXPR_NODES_H
#define MINISWIFT_EXPR_NODES_H

#include "../lexer/Token.h"
#include <vector>

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
struct Call;
struct Closure;
struct EnumAccess;
struct MemberAccess;
struct StructInit;
struct Super;

// Visitor interface for expressions
class ExprVisitor {
public:
  virtual ~ExprVisitor() = default;
  virtual void visit(const Binary &expr) = 0;
  virtual void visit(const Grouping &expr) = 0;
  virtual void visit(const Literal &expr) = 0;
  virtual void visit(const Unary &expr) = 0;
  virtual void visit(const VarExpr &expr) = 0;
  virtual void visit(const Assign &expr) = 0;
  virtual void visit(const ArrayLiteral &expr) = 0;
  virtual void visit(const DictionaryLiteral &expr) = 0;
  virtual void visit(const IndexAccess &expr) = 0;
  virtual void visit(const Call &expr) = 0;
  virtual void visit(const Closure &expr) = 0;
  virtual void visit(const EnumAccess &expr) = 0;
  virtual void visit(const MemberAccess &expr) = 0;
  virtual void visit(const StructInit &expr) = 0;
  virtual void visit(const Super &expr) = 0;
};

// Base class for all expression nodes
class Expr {
public:
  virtual ~Expr() = default;
  virtual void accept(ExprVisitor &visitor) const = 0;
  virtual std::unique_ptr<Expr> clone() const = 0;
};

// Concrete expression classes
struct Binary : Expr {
  Binary(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
      : left(std::move(left)), op(op), right(std::move(right)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<Binary>(left->clone(), op, right->clone());
  }

  const std::unique_ptr<Expr> left;
  const Token op;
  const std::unique_ptr<Expr> right;
};

struct VarExpr : Expr {
  explicit VarExpr(Token name) : name(name) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<VarExpr>(name);
  }

  const Token name;
};

struct Assign : Expr {
  Assign(std::unique_ptr<Expr> target, std::unique_ptr<Expr> value)
      : target(std::move(target)), value(std::move(value)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<Assign>(target->clone(), value->clone());
  }

  const std::unique_ptr<Expr> target; // Can be VarExpr or MemberAccess
  const std::unique_ptr<Expr> value;
};

struct Grouping : Expr {
  explicit Grouping(std::unique_ptr<Expr> expression)
      : expression(std::move(expression)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<Grouping>(expression->clone());
  }

  const std::unique_ptr<Expr> expression;
};

struct Literal : Expr {
  explicit Literal(Token value) : value(value) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<Literal>(value);
  }

  const Token value;
};

struct Unary : Expr {
  Unary(Token op, std::unique_ptr<Expr> right)
      : op(op), right(std::move(right)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

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

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    std::vector<std::unique_ptr<Expr>> clonedElements;
    for (const auto &element : elements) {
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

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    std::vector<KeyValuePair> clonedPairs;
    for (const auto &pair : pairs) {
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

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<IndexAccess>(object->clone(), index->clone());
  }

  const std::unique_ptr<Expr> object;
  const std::unique_ptr<Expr> index;
};

// Function call expression: functionName(arguments)
struct Call : Expr {
  Call(std::unique_ptr<Expr> callee,
       std::vector<std::unique_ptr<Expr>> arguments)
      : callee(std::move(callee)), arguments(std::move(arguments)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    std::vector<std::unique_ptr<Expr>> clonedArguments;
    for (const auto &arg : arguments) {
      clonedArguments.push_back(arg->clone());
    }
    return std::make_unique<Call>(callee->clone(), std::move(clonedArguments));
  }

  const std::unique_ptr<Expr> callee;
  const std::vector<std::unique_ptr<Expr>> arguments;
};

// Super expression: super.method() or super.property
struct Super : Expr {
  Token keyword; // The 'super' token
  Token method;  // The method or property name

  Super(Token keyword, Token method)
      : keyword(keyword), method(method) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<Super>(keyword, method);
  }
};

}; // namespace miniswift

#endif // MINISWIFT_EXPR_NODES_H
