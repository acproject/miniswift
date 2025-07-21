#ifndef MINISWIFT_STMT_NODES_H
#define MINISWIFT_STMT_NODES_H
#include "ExprNodes.h"
#include "AccessControl.h"

namespace miniswift {

// Forward declaration for Expr
class Expr;
class ExprVisitor;
// Forward declarations
struct Parameter;
struct ExprStmt;
struct PrintStmt; // For testing
struct VarStmt;
struct BlockStmt;
struct IfStmt;
struct WhileStmt;
struct ForStmt;
struct FunctionStmt;
struct ReturnStmt;
struct EnumStmt;
struct StructStmt;
struct ClassStmt;
struct InitStmt;
struct DeinitStmt;
struct SubscriptStmt;

// Visitor for Stmt
class StmtVisitor {
public:
  virtual ~StmtVisitor() = default;
  virtual void visit(const ExprStmt &stmt) = 0;
  virtual void visit(const PrintStmt &stmt) = 0;
  virtual void visit(const VarStmt &stmt) = 0;
  virtual void visit(const BlockStmt &stmt) = 0;
  virtual void visit(const IfStmt &stmt) = 0;
  virtual void visit(const WhileStmt &stmt) = 0;
  virtual void visit(const ForStmt &stmt) = 0;
  virtual void visit(const FunctionStmt &stmt) = 0;
  virtual void visit(const ReturnStmt &stmt) = 0;
  virtual void visit(const EnumStmt &stmt) = 0;
  virtual void visit(const StructStmt &stmt) = 0;
  virtual void visit(const ClassStmt &stmt) = 0;
  virtual void visit(const InitStmt &stmt) = 0;
  virtual void visit(const DeinitStmt &stmt) = 0;
  virtual void visit(const SubscriptStmt &stmt) = 0;
};

// Base class for Stmt
class Stmt {
public:
  virtual ~Stmt() = default;
  virtual void accept(StmtVisitor &visitor) const = 0;
  virtual std::unique_ptr<Stmt> clone() const = 0;
};

// Concrete statement classes
struct ExprStmt : Stmt {
  explicit ExprStmt(std::unique_ptr<Expr> expression)
      : expression(std::move(expression)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<ExprStmt>(expression->clone());
  }

  const std::unique_ptr<Expr> expression;
};

struct PrintStmt : Stmt {
  explicit PrintStmt(std::unique_ptr<Expr> expression)
      : expression(std::move(expression)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<PrintStmt>(expression->clone());
  }

  const std::unique_ptr<Expr> expression;
};

struct VarStmt : Stmt {
  VarStmt(Token name, std::unique_ptr<Expr> initializer, bool isConst,
          Token type, AccessLevel accessLevel = AccessLevel::INTERNAL,
          AccessLevel setterAccessLevel = AccessLevel::INTERNAL)
      : name(name), initializer(std::move(initializer)), isConst(isConst),
        type(type), accessLevel(accessLevel), setterAccessLevel(setterAccessLevel) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<VarStmt>(
        name, initializer ? initializer->clone() : nullptr, isConst, type,
        accessLevel, setterAccessLevel);
  }

  const Token name;
  const std::unique_ptr<Expr> initializer;
  const bool isConst;
  const Token type;
  const AccessLevel accessLevel;
  const AccessLevel setterAccessLevel;
};

// Block statement: { statements }
struct BlockStmt : Stmt {
  explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
      : statements(std::move(statements)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    std::vector<std::unique_ptr<Stmt>> clonedStatements;
    for (const auto &stmt : statements) {
      clonedStatements.push_back(stmt->clone());
    }
    return std::make_unique<BlockStmt>(std::move(clonedStatements));
  }

  const std::vector<std::unique_ptr<Stmt>> statements;
};

// If statement: if condition { thenBranch } else { elseBranch }
struct IfStmt : Stmt {
  IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> thenBranch,
         std::unique_ptr<Stmt> elseBranch)
      : condition(std::move(condition)), thenBranch(std::move(thenBranch)),
        elseBranch(std::move(elseBranch)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<IfStmt>(condition->clone(), thenBranch->clone(),
                                    elseBranch ? elseBranch->clone() : nullptr);
  }

  const std::unique_ptr<Expr> condition;
  const std::unique_ptr<Stmt> thenBranch;
  const std::unique_ptr<Stmt> elseBranch; // Can be null
};

// While statement: while condition { body }
struct WhileStmt : Stmt {
  WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<Stmt> body)
      : condition(std::move(condition)), body(std::move(body)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<WhileStmt>(condition->clone(), body->clone());
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

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<ForStmt>(
        initializer ? initializer->clone() : nullptr,
        condition ? condition->clone() : nullptr,
        increment ? increment->clone() : nullptr, body->clone());
  }

  const std::unique_ptr<Stmt> initializer; // Can be null
  const std::unique_ptr<Expr> condition;   // Can be null
  const std::unique_ptr<Expr> increment;   // Can be null
  const std::unique_ptr<Stmt> body;
};

// Function parameter
struct Parameter {
  Token name;
  Token type; // Can be empty for type inference

  Parameter(Token n, Token t) : name(n), type(t) {}
};

// Closure expression: { (parameters) -> ReturnType in body }
struct Closure : Expr {
  Closure(std::vector<Parameter> parameters, Token returnType,
          std::vector<std::unique_ptr<Stmt>> body)
      : parameters(std::move(parameters)), returnType(returnType),
        body(std::move(body)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    // For now, we'll create a shallow copy that shares the same body
    // This is a temporary solution until we implement proper Stmt cloning
    std::vector<std::unique_ptr<Stmt>> clonedBody;
    // We can't clone statements easily, so we'll avoid cloning closures for now
    // This is a limitation that should be addressed in a full implementation
    return std::make_unique<Closure>(parameters, returnType,
                                     std::move(clonedBody));
  }

  const std::vector<Parameter> parameters;
  const Token returnType; // Can be empty for type inference
  const std::vector<std::unique_ptr<Stmt>> body;
};

// Function declaration: func name(parameters) -> returnType { body }
struct FunctionStmt : Stmt {
  FunctionStmt(Token name, std::vector<Parameter> parameters, Token returnType,
               std::unique_ptr<Stmt> body, AccessLevel accessLevel = AccessLevel::INTERNAL)
      : name(name), parameters(std::move(parameters)), returnType(returnType),
        body(std::move(body)), accessLevel(accessLevel) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<FunctionStmt>(name, parameters, returnType,
                                          body->clone(), accessLevel);
  }

  const Token name;
  const std::vector<Parameter> parameters;
  const Token returnType; // Can be empty for Void
  const std::unique_ptr<Stmt> body;
  AccessLevel accessLevel;
};

// Return statement: return expression
struct ReturnStmt : Stmt {
  explicit ReturnStmt(std::unique_ptr<Expr> value) : value(std::move(value)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<ReturnStmt>(value ? value->clone() : nullptr);
  }

  const std::unique_ptr<Expr> value; // Can be null for void return
};

// Enum case definition
struct EnumCase {
  Token name;
  std::vector<Token> associatedTypes; // For associated values like case
                                      // upc(Int, Int, Int, Int)
  std::unique_ptr<Expr> rawValue;     // For raw values like case mercury = 1

  EnumCase(Token n, std::vector<Token> types = {},
           std::unique_ptr<Expr> value = nullptr)
      : name(n), associatedTypes(std::move(types)), rawValue(std::move(value)) {
  }
};

// Enum declaration: enum Name: RawType { cases }
struct EnumStmt : Stmt {
  EnumStmt(Token name, Token rawType, std::vector<EnumCase> cases,
           std::vector<std::unique_ptr<SubscriptStmt>> subscripts = {})
      : name(name), rawType(rawType), cases(std::move(cases)), subscripts(std::move(subscripts)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    std::vector<EnumCase> clonedCases;
    for (const auto &enumCase : cases) {
      clonedCases.emplace_back(enumCase.name, enumCase.associatedTypes,
                               enumCase.rawValue ? enumCase.rawValue->clone()
                                                 : nullptr);
    }
    // For simplicity, skip cloning subscripts in this basic implementation
    return std::make_unique<EnumStmt>(name, rawType, std::move(clonedCases));
  }

  const Token name;
  const Token rawType; // Can be empty for no raw type
  const std::vector<EnumCase> cases;
  const std::vector<std::unique_ptr<SubscriptStmt>> subscripts; // Static subscripts
};

// Enum case access expression: EnumName.caseName or .caseName
struct EnumAccess : Expr {
  std::unique_ptr<Expr> enumType;
  Token caseName;
  std::vector<std::unique_ptr<Expr>> arguments;

  EnumAccess(std::unique_ptr<Expr> enumType, Token caseName,
             std::vector<std::unique_ptr<Expr>> arguments = {})
      : enumType(std::move(enumType)), caseName(caseName),
        arguments(std::move(arguments)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    std::vector<std::unique_ptr<Expr>> clonedValues;
    for (const auto &value : arguments) {
      clonedValues.push_back(value->clone());
    }
    return std::make_unique<EnumAccess>(enumType ? enumType->clone() : nullptr,
                                        caseName, std::move(clonedValues));
  }
};

// Member access expression: object.member
struct MemberAccess : Expr {
  std::unique_ptr<Expr> object;
  Token member;

  MemberAccess(std::unique_ptr<Expr> object, Token member)
      : object(std::move(object)), member(member) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    return std::make_unique<MemberAccess>(object->clone(), member);
  }
};

// Struct initialization expression: StructName(member1: value1, member2:
// value2)
struct StructInit : Expr {
  Token structName;
  std::vector<std::pair<Token, std::unique_ptr<Expr>>>
      members; // (memberName, value)

  StructInit(Token structName,
             std::vector<std::pair<Token, std::unique_ptr<Expr>>> members)
      : structName(structName), members(std::move(members)) {}

  void accept(ExprVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Expr> clone() const override {
    std::vector<std::pair<Token, std::unique_ptr<Expr>>> clonedMembers;
    for (const auto &member : members) {
      clonedMembers.emplace_back(member.first, member.second->clone());
    }
    return std::make_unique<StructInit>(structName, std::move(clonedMembers));
  }
};

// Property accessor types
enum class AccessorType {
  GET,      // getter
  SET,      // setter
  WILL_SET, // willSet observer
  DID_SET   // didSet observer
};

// Property accessor definition
struct PropertyAccessor {
  AccessorType type;
  std::unique_ptr<Stmt> body;
  std::string parameterName; // Optional parameter name for willSet/didSet

  PropertyAccessor(AccessorType t, std::unique_ptr<Stmt> b = nullptr,
                   const std::string &param = "")
      : type(t), body(std::move(b)), parameterName(param) {}
};

// Enhanced struct member definition with property support
struct StructMember {
  Token name;
  Token type;
  std::unique_ptr<Expr> defaultValue; // Optional default value
  bool isVar;                         // true for var, false for let
  bool isStatic;                      // true for static properties
  bool isLazy;                        // true for lazy properties
  AccessLevel accessLevel;            // Access level for the property
  AccessLevel setterAccessLevel;      // Access level for the setter (can be more restrictive)

  // Property accessors (for computed properties and observers)
  std::vector<PropertyAccessor> accessors;

  StructMember(Token name, Token type,
               std::unique_ptr<Expr> defaultValue = nullptr, bool isVar = true,
               AccessLevel accessLevel = AccessLevel::INTERNAL,
               AccessLevel setterAccessLevel = AccessLevel::INTERNAL)
      : name(name), type(type), defaultValue(std::move(defaultValue)),
        isVar(isVar), isStatic(false), isLazy(false),
        accessLevel(accessLevel), setterAccessLevel(setterAccessLevel) {}

  // Check if this is a computed property
  bool isComputedProperty() const {
    return hasAccessor(AccessorType::GET) || hasAccessor(AccessorType::SET);
  }

  // Check if this member has a specific accessor
  bool hasAccessor(AccessorType type) const {
    for (const auto &accessor : accessors) {
      if (accessor.type == type) {
        return true;
      }
    }
    return false;
  }
};

// Struct declaration: struct Name { members }
struct StructStmt : Stmt {
  Token name;
  AccessLevel accessLevel; // Access level for the struct
  std::vector<StructMember> members;
  std::vector<std::unique_ptr<FunctionStmt>>
      methods; // Methods defined in the struct
  std::vector<std::unique_ptr<InitStmt>> initializers; // Constructors
  std::unique_ptr<DeinitStmt> deinitializer;           // Destructor
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts; // Subscripts

  StructStmt(Token name, std::vector<StructMember> members,
             std::vector<std::unique_ptr<FunctionStmt>> methods = {},
             std::vector<std::unique_ptr<InitStmt>> initializers = {},
             std::unique_ptr<DeinitStmt> deinitializer = nullptr,
             std::vector<std::unique_ptr<SubscriptStmt>> subscripts = {},
             AccessLevel accessLevel = AccessLevel::INTERNAL)
      : name(name), accessLevel(accessLevel), members(std::move(members)), methods(std::move(methods)),
        initializers(std::move(initializers)),
        deinitializer(std::move(deinitializer)), subscripts(std::move(subscripts)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    // For simplicity, we'll create a basic clone without deep copying members
    // and methods This is sufficient for our current use case
    return std::make_unique<StructStmt>(
        name, std::vector<StructMember>(),
        std::vector<std::unique_ptr<FunctionStmt>>());
  }
};

// Class declaration: class Name { members }
struct ClassStmt : Stmt {
  Token name;
  Token superclass; // Optional superclass
  AccessLevel accessLevel; // Access level for the class
  std::vector<StructMember> members;
  std::vector<std::unique_ptr<FunctionStmt>> methods;
  std::vector<std::unique_ptr<InitStmt>> initializers; // Constructors
  std::unique_ptr<DeinitStmt> deinitializer;           // Destructor
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts; // Subscripts

  ClassStmt(Token name, Token superclass, std::vector<StructMember> members,
            std::vector<std::unique_ptr<FunctionStmt>> methods = {},
            std::vector<std::unique_ptr<InitStmt>> initializers = {},
            std::unique_ptr<DeinitStmt> deinitializer = nullptr,
            std::vector<std::unique_ptr<SubscriptStmt>> subscripts = {},
            AccessLevel accessLevel = AccessLevel::INTERNAL)
      : name(name), superclass(superclass), accessLevel(accessLevel), members(std::move(members)),
        methods(std::move(methods)), initializers(std::move(initializers)),
        deinitializer(std::move(deinitializer)), subscripts(std::move(subscripts)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    // For simplicity, we'll create a basic clone without deep copying members
    // and methods This is sufficient for our current use case
    return std::make_unique<ClassStmt>(
        name, superclass, std::vector<StructMember>(),
        std::vector<std::unique_ptr<FunctionStmt>>());
  }
};

// Constructor types
enum class InitType {
  DESIGNATED,  // 指定构造器
  CONVENIENCE, // 便利构造器
  FAILABLE     // 可失败构造器
};

// Constructor declaration: init(parameters) { body }
struct InitStmt : Stmt {
  InitType initType;
  std::vector<Parameter> parameters;
  std::unique_ptr<Stmt> body;
  bool isRequired; // 是否为required构造器
  AccessLevel accessLevel; // Access level for the initializer

  InitStmt(InitType type, std::vector<Parameter> parameters,
           std::unique_ptr<Stmt> body, bool required = false,
           AccessLevel accessLevel = AccessLevel::INTERNAL)
      : initType(type), parameters(std::move(parameters)),
        body(std::move(body)), isRequired(required), accessLevel(accessLevel) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<InitStmt>(initType, parameters, body->clone(),
                                      isRequired, accessLevel);
  }
};

// Destructor declaration: deinit { body }
struct DeinitStmt : Stmt {
  std::unique_ptr<Stmt> body;

  explicit DeinitStmt(std::unique_ptr<Stmt> body) : body(std::move(body)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<DeinitStmt>(body->clone());
  }
};

// Subscript declaration: subscript(parameters) -> ReturnType { get set }
struct SubscriptStmt : Stmt {
  std::vector<Parameter> parameters;
  Token returnType;
  std::vector<PropertyAccessor> accessors; // get/set accessors
  bool isStatic; // true for static subscripts
  AccessLevel accessLevel; // Access level for the subscript
  AccessLevel setterAccessLevel; // Access level for the setter

  SubscriptStmt(std::vector<Parameter> parameters, Token returnType,
                std::vector<PropertyAccessor> accessors, bool isStatic = false,
                AccessLevel accessLevel = AccessLevel::INTERNAL,
                AccessLevel setterAccessLevel = AccessLevel::INTERNAL)
      : parameters(std::move(parameters)), returnType(returnType),
        accessors(std::move(accessors)), isStatic(isStatic),
        accessLevel(accessLevel), setterAccessLevel(setterAccessLevel) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    // Clone accessors
    std::vector<PropertyAccessor> clonedAccessors;
    for (const auto &accessor : accessors) {
      clonedAccessors.emplace_back(
          accessor.type,
          accessor.body ? accessor.body->clone() : nullptr,
          accessor.parameterName);
    }
    return std::make_unique<SubscriptStmt>(parameters, returnType,
                                           std::move(clonedAccessors), isStatic,
                                           accessLevel, setterAccessLevel);
  }
};

} // namespace miniswift

#endif // MINISWIFT_STMT_NODES_H