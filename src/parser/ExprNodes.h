#ifndef MINISWIFT_EXPR_NODES_H
#define MINISWIFT_EXPR_NODES_H

#include "../lexer/Token.h"
#include <vector>

namespace miniswift {
// Forward declarations
struct Binary;
struct Ternary;
struct Grouping;
struct Literal;
struct Unary;
struct VarExpr;
struct Assign;
struct ArrayLiteral;
struct DictionaryLiteral;
struct TupleLiteral;
struct IndexAccess;
struct Call;
struct LabeledCall;
struct Closure;
struct EnumAccess;
struct MemberAccess;
struct StructInit;
struct Super;
struct StringInterpolation;
struct SubscriptAccess;
struct OptionalChaining;
struct Range;
struct GenericTypeInstantiationExpr;
struct TypeCheck;
struct TypeCast;
// Error handling expressions
struct TryExpr;
struct ResultTypeExpr;
struct ErrorLiteral;
// Advanced operator expressions
struct CustomOperatorExpr;
struct BitwiseExpr;
struct OverflowExpr;
// Result Builder expressions
struct ResultBuilderExpr;
// Concurrency expressions
struct AwaitExpr;
struct TaskExpr;

// Visitor interface for expressions
class ExprVisitor {
public:
  virtual ~ExprVisitor() = default;
  virtual void visit(const Binary &expr) = 0;
  virtual void visit(const Ternary &expr) = 0;
  virtual void visit(const Grouping &expr) = 0;
  virtual void visit(const Literal &expr) = 0;
  virtual void visit(const Unary &expr) = 0;
  virtual void visit(const VarExpr &expr) = 0;
  virtual void visit(const Assign &expr) = 0;
  virtual void visit(const ArrayLiteral &expr) = 0;
  virtual void visit(const DictionaryLiteral &expr) = 0;
  virtual void visit(const TupleLiteral &expr) = 0;
  virtual void visit(const IndexAccess &expr) = 0;
  virtual void visit(const SubscriptAccess &expr) = 0;
  virtual void visit(const Call &expr) = 0;
  virtual void visit(const LabeledCall &expr) = 0;
  virtual void visit(const Closure &expr) = 0;
  virtual void visit(const EnumAccess &expr) = 0;
  virtual void visit(const MemberAccess &expr) = 0;
  virtual void visit(const StructInit &expr) = 0;
  virtual void visit(const Super &expr) = 0;
  virtual void visit(const StringInterpolation &expr) = 0;
  virtual void visit(const OptionalChaining &expr) = 0;
  virtual void visit(const Range &expr) = 0;
  virtual void visit(const GenericTypeInstantiationExpr &expr) = 0;
  virtual void visit(const TypeCheck &expr) = 0;
  virtual void visit(const TypeCast &expr) = 0;
  // Error handling expressions
  virtual void visit(const TryExpr &expr) = 0;
  virtual void visit(const ResultTypeExpr &expr) = 0;
  virtual void visit(const ErrorLiteral &expr) = 0;
  // Advanced operator expressions
  virtual void visit(const CustomOperatorExpr &expr) = 0;
  virtual void visit(const BitwiseExpr &expr) = 0;
  virtual void visit(const OverflowExpr &expr) = 0;
  // Result Builder expressions
  virtual void visit(const ResultBuilderExpr &expr) = 0;
  // Concurrency expressions
  virtual void visit(const AwaitExpr &expr) = 0;
  virtual void visit(const TaskExpr &expr) = 0;
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

struct Ternary : Expr {
  Ternary(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> thenBranch, std::unique_ptr<Expr> elseBranch)
      : condition(std::move(condition)), thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<Ternary>(condition->clone(), thenBranch->clone(), elseBranch->clone());
  }

  const std::unique_ptr<Expr> condition;
  const std::unique_ptr<Expr> thenBranch;
  const std::unique_ptr<Expr> elseBranch;
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

// Tuple literal expression: ("hello", 42)
struct TupleLiteral : Expr {
  explicit TupleLiteral(std::vector<std::unique_ptr<Expr>> elements)
      : elements(std::move(elements)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    std::vector<std::unique_ptr<Expr>> clonedElements;
    for (const auto &element : elements) {
      clonedElements.push_back(element->clone());
    }
    return std::make_unique<TupleLiteral>(std::move(clonedElements));
  }

  const std::vector<std::unique_ptr<Expr>> elements;
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

// Labeled function call expression: functionName(label1: arg1, label2: arg2)
struct LabeledCall : Expr {
  LabeledCall(std::unique_ptr<Expr> callee,
              std::vector<std::unique_ptr<Expr>> arguments,
              std::vector<Token> argumentLabels)
      : callee(std::move(callee)), arguments(std::move(arguments)), argumentLabels(std::move(argumentLabels)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    std::vector<std::unique_ptr<Expr>> clonedArguments;
    for (const auto &arg : arguments) {
      clonedArguments.push_back(arg->clone());
    }
    return std::make_unique<LabeledCall>(callee->clone(), std::move(clonedArguments), argumentLabels);
  }

  const std::unique_ptr<Expr> callee;
  const std::vector<std::unique_ptr<Expr>> arguments;
  const std::vector<Token> argumentLabels;
};

// String interpolation expression: "Hello \(name)!"
struct StringInterpolation : Expr {
  struct InterpolationPart {
    std::string text;  // String literal part
    std::unique_ptr<Expr> expression;  // Expression to interpolate (null for text parts)
    
    InterpolationPart(std::string t) : text(std::move(t)), expression(nullptr) {}
    InterpolationPart(std::unique_ptr<Expr> expr) : expression(std::move(expr)) {}
  };

  explicit StringInterpolation(std::vector<InterpolationPart> parts)
      : parts(std::move(parts)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    std::vector<InterpolationPart> clonedParts;
    for (const auto &part : parts) {
      if (part.expression) {
        clonedParts.emplace_back(part.expression->clone());
      } else {
        clonedParts.emplace_back(part.text);
      }
    }
    return std::make_unique<StringInterpolation>(std::move(clonedParts));
  }

  const std::vector<InterpolationPart> parts;
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

// Subscript access expression: object[index1, index2, ...]
struct SubscriptAccess : Expr {
  SubscriptAccess(std::unique_ptr<Expr> object, std::vector<std::unique_ptr<Expr>> indices)
      : object(std::move(object)), indices(std::move(indices)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    std::vector<std::unique_ptr<Expr>> clonedIndices;
    for (const auto &index : indices) {
      clonedIndices.push_back(index->clone());
    }
    return std::make_unique<SubscriptAccess>(object->clone(), std::move(clonedIndices));
  }

  const std::unique_ptr<Expr> object;
  const std::vector<std::unique_ptr<Expr>> indices;
};

// Optional chaining expression: object?.property or object?.method() or object?[index]
struct OptionalChaining : Expr {
  enum class ChainType {
    Property,    // object?.property
    Method,      // object?.method()
    Subscript    // object?[index]
  };
  
  OptionalChaining(std::unique_ptr<Expr> object, ChainType chainType, std::unique_ptr<Expr> accessor)
      : object(std::move(object)), chainType(chainType), accessor(std::move(accessor)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<OptionalChaining>(object->clone(), chainType, accessor->clone());
  }

  const std::unique_ptr<Expr> object;     // The optional object being chained
  const ChainType chainType;              // Type of chaining (property, method, subscript)
  const std::unique_ptr<Expr> accessor;   // The property/method/subscript being accessed
};

// Range expression: start..<end or start...end
struct Range : Expr {
  enum class RangeType {
    HalfOpen,  // start..<end
    Closed     // start...end
  };
  
  Range(std::unique_ptr<Expr> start, std::unique_ptr<Expr> end, RangeType rangeType)
      : start(std::move(start)), end(std::move(end)), rangeType(rangeType) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<Range>(start->clone(), end->clone(), rangeType);
  }

  const std::unique_ptr<Expr> start;
  const std::unique_ptr<Expr> end;
  const RangeType rangeType;
};

// Generic type instantiation expression: Container<String>
struct GenericTypeInstantiationExpr : Expr {
  GenericTypeInstantiationExpr(Token typeName, std::vector<Token> typeArguments)
      : typeName(typeName), typeArguments(std::move(typeArguments)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<GenericTypeInstantiationExpr>(typeName, typeArguments);
  }

  const Token typeName;
  const std::vector<Token> typeArguments;
};

// Type checking expression: value is Type
struct TypeCheck : Expr {
  TypeCheck(std::unique_ptr<Expr> expression, Token targetType)
      : expression(std::move(expression)), targetType(targetType) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<TypeCheck>(expression->clone(), targetType);
  }

  const std::unique_ptr<Expr> expression;
  const Token targetType;
};

// Type casting expression: value as Type or value as? Type or value as! Type
struct TypeCast : Expr {
  enum class CastType {
    Forced,      // as!
    Optional,    // as?
    Safe         // as
  };
  
  TypeCast(std::unique_ptr<Expr> expression, Token targetType, CastType castType)
      : expression(std::move(expression)), targetType(targetType), castType(castType) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<TypeCast>(expression->clone(), targetType, castType);
  }

  const std::unique_ptr<Expr> expression;
  const Token targetType;
  const CastType castType;
};

// Custom operator expression: left customOp right (or customOp right for prefix)
struct CustomOperatorExpr : Expr {
  CustomOperatorExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
      : left(std::move(left)), op(op), right(std::move(right)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    auto leftClone = left ? left->clone() : nullptr;
    return std::make_unique<CustomOperatorExpr>(std::move(leftClone), op, right->clone());
  }

  const std::unique_ptr<Expr> left;  // Can be null for prefix operators
  const Token op;
  const std::unique_ptr<Expr> right;
};

// Bitwise operation expression: left & right, left | right, etc.
struct BitwiseExpr : Expr {
  BitwiseExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
      : left(std::move(left)), op(op), right(std::move(right)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<BitwiseExpr>(left->clone(), op, right->clone());
  }

  const std::unique_ptr<Expr> left;
  const Token op;
  const std::unique_ptr<Expr> right;
};

// Overflow operation expression: left &+ right, left &- right, etc.
struct OverflowExpr : Expr {
  OverflowExpr(std::unique_ptr<Expr> left, Token op, std::unique_ptr<Expr> right)
      : left(std::move(left)), op(op), right(std::move(right)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<OverflowExpr>(left->clone(), op, right->clone());
  }

  const std::unique_ptr<Expr> left;
  const Token op;
  const std::unique_ptr<Expr> right;
};

// Result Builder expression: @BuilderType { ... }
struct ResultBuilderExpr : Expr {
  ResultBuilderExpr(Token builderType, std::vector<std::unique_ptr<Expr>> components)
      : builderType(builderType), components(std::move(components)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    std::vector<std::unique_ptr<Expr>> clonedComponents;
    for (const auto &component : components) {
      clonedComponents.push_back(component->clone());
    }
    return std::make_unique<ResultBuilderExpr>(builderType, std::move(clonedComponents));
  }

  const Token builderType;
  const std::vector<std::unique_ptr<Expr>> components;
};

// Await expression: await expression
struct AwaitExpr : Expr {
  AwaitExpr(Token awaitKeyword, std::unique_ptr<Expr> expression)
      : awaitKeyword(awaitKeyword), expression(std::move(expression)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<AwaitExpr>(awaitKeyword, expression->clone());
  }

  const Token awaitKeyword;
  const std::unique_ptr<Expr> expression;
};

// Task expression: Task { ... } or Task.detached { ... }
struct TaskExpr : Expr {
  enum class TaskType {
    Regular,   // Task { ... }
    Detached   // Task.detached { ... }
  };
  
  TaskExpr(Token taskKeyword, TaskType taskType, std::unique_ptr<Expr> closure)
      : taskKeyword(taskKeyword), taskType(taskType), closure(std::move(closure)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<TaskExpr>(taskKeyword, taskType, closure->clone());
  }

  const Token taskKeyword;
  const TaskType taskType;
  const std::unique_ptr<Expr> closure;
};

}; // namespace miniswift

#endif // MINISWIFT_EXPR_NODES_H
