#ifndef MINISWIFT_STMT_NODES_H
#define MINISWIFT_STMT_NODES_H
#include "ExprNodes.h"
#include "AccessControl.h"
#include "Parameter.h"
#include "GenericNodes.h"

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
struct IfLetStmt;
struct WhileStmt;
struct ForStmt;
struct ForInStmt;
struct FunctionStmt;
struct ReturnStmt;
struct EnumStmt;
struct StructStmt;
struct ClassStmt;
struct InitStmt;
struct DeinitStmt;
struct SubscriptStmt;
struct ProtocolStmt;
struct ExtensionStmt;
// Error handling statements
struct ThrowStmt;
struct DoCatchStmt;
struct DeferStmt;
struct GuardStmt;
struct GuardLetStmt;
// Switch statement
struct SwitchStmt;
// Advanced operator statements
struct CustomOperatorStmt;
struct OperatorPrecedenceStmt;
// Result Builder statements
struct ResultBuilderStmt;
// Concurrency statements
struct ActorStmt;
// Macro statements
struct MacroStmt;
struct ExternalMacroStmt;
struct FreestandingMacroStmt;
struct AttachedMacroStmt;

// Visitor for Stmt
class StmtVisitor {
public:
  virtual ~StmtVisitor() = default;
  virtual void visit(const ExprStmt &stmt) = 0;
  virtual void visit(const PrintStmt &stmt) = 0;
  virtual void visit(const VarStmt &stmt) = 0;
  virtual void visit(const BlockStmt &stmt) = 0;
  virtual void visit(const IfStmt &stmt) = 0;
  virtual void visit(const IfLetStmt &stmt) = 0;
  virtual void visit(const WhileStmt &stmt) = 0;
  virtual void visit(const ForStmt &stmt) = 0;
  virtual void visit(const ForInStmt &stmt) = 0;
  virtual void visit(const FunctionStmt &stmt) = 0;
  virtual void visit(const ReturnStmt &stmt) = 0;
  virtual void visit(const EnumStmt &stmt) = 0;
  virtual void visit(const StructStmt &stmt) = 0;
  virtual void visit(const ClassStmt &stmt) = 0;
  virtual void visit(const InitStmt &stmt) = 0;
  virtual void visit(const DeinitStmt &stmt) = 0;
  virtual void visit(const SubscriptStmt &stmt) = 0;
  virtual void visit(const ProtocolStmt &stmt) = 0;
  virtual void visit(const ExtensionStmt &stmt) = 0;
  // Error handling statements
  virtual void visit(const ThrowStmt &stmt) = 0;
  virtual void visit(const DoCatchStmt &stmt) = 0;
  virtual void visit(const DeferStmt &stmt) = 0;
  virtual void visit(const GuardStmt &stmt) = 0;
  virtual void visit(const GuardLetStmt &stmt) = 0;
  virtual void visit(const SwitchStmt &stmt) = 0;
  // Advanced operator statements
  virtual void visit(const CustomOperatorStmt &stmt) = 0;
  virtual void visit(const OperatorPrecedenceStmt &stmt) = 0;
  // Result Builder statements
  virtual void visit(const ResultBuilderStmt &stmt) = 0;
  // Concurrency statements
  virtual void visit(const ActorStmt &stmt) = 0;
  // Macro statements
  virtual void visit(const MacroStmt &stmt) = 0;
  virtual void visit(const ExternalMacroStmt &stmt) = 0;
  virtual void visit(const FreestandingMacroStmt &stmt) = 0;
  virtual void visit(const AttachedMacroStmt &stmt) = 0;
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

  void accept(miniswift::StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<ExprStmt>(expression->clone());
  }

  const std::unique_ptr<Expr> expression;
};

struct PrintStmt : Stmt {
  explicit PrintStmt(std::vector<std::unique_ptr<Expr>> expressions)
      : expressions(std::move(expressions)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    std::vector<std::unique_ptr<Expr>> clonedExpressions;
    for (const auto& expr : expressions) {
      clonedExpressions.push_back(expr->clone());
    }
    return std::make_unique<PrintStmt>(std::move(clonedExpressions));
  }

  const std::vector<std::unique_ptr<Expr>> expressions;
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

// If-let statement: if let variable = expression { thenBranch } else { elseBranch }
struct IfLetStmt : Stmt {
  IfLetStmt(Token variable, std::unique_ptr<Expr> expression, 
            std::unique_ptr<Stmt> thenBranch, std::unique_ptr<Stmt> elseBranch)
      : variable(variable), expression(std::move(expression)), 
        thenBranch(std::move(thenBranch)), elseBranch(std::move(elseBranch)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<IfLetStmt>(variable, expression->clone(), 
                                       thenBranch->clone(),
                                       elseBranch ? elseBranch->clone() : nullptr);
  }

  const Token variable;                   // Variable to bind to
  const std::unique_ptr<Expr> expression; // Expression to unwrap
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

// For-in statement: for variable in collection { body } or for (var1, var2) in collection { body }
struct ForInStmt : Stmt {
  ForInStmt(std::vector<Token> variables, std::unique_ptr<Expr> collection,
            std::unique_ptr<Stmt> body)
      : variables(std::move(variables)), collection(std::move(collection)),
        body(std::move(body)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<ForInStmt>(variables, collection->clone(), body->clone());
  }

  const std::vector<Token> variables; // Loop variables (can be multiple for tuple destructuring)
  const std::unique_ptr<Expr> collection; // Collection to iterate over
  const std::unique_ptr<Stmt> body;
};

// Parameter is now defined in Parameter.h

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

// Function declaration: func name<T>(parameters) async throws -> returnType where T: Equatable { body }
struct FunctionStmt : Stmt {
  FunctionStmt(Token name, std::vector<Parameter> parameters, Token returnType,
               std::unique_ptr<Stmt> body, AccessLevel accessLevel = AccessLevel::INTERNAL,
               GenericParameterClause genericParams = GenericParameterClause({}),
               WhereClause whereClause = WhereClause({}), bool isMutating = false,
               bool canThrow = false, bool isAsync = false, bool isMain = false)
      : name(name), parameters(std::move(parameters)), returnType(returnType),
        body(std::move(body)), accessLevel(accessLevel),
        genericParams(std::move(genericParams)), whereClause(std::move(whereClause)),
        isMutating(isMutating), canThrow(canThrow), isAsync(isAsync), isMain(isMain) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    return std::make_unique<FunctionStmt>(name, parameters, returnType,
                                          body->clone(), accessLevel,
                                          genericParams, whereClause, isMutating, canThrow, isAsync, isMain);
  }

  const Token name;
  const std::vector<Parameter> parameters;
  const Token returnType; // Can be empty for Void
  const std::unique_ptr<Stmt> body;
  AccessLevel accessLevel;
  GenericParameterClause genericParams; // Generic type parameters
  WhereClause whereClause;              // Generic constraints
  bool isMutating;                      // Whether this is a mutating function
  bool canThrow;                        // Whether this function can throw errors
  bool isAsync;                         // Whether this is an async function
  bool isMain;                          // Whether this function has @main attribute
  
  // Check if this is a generic function
  bool isGeneric() const { return !genericParams.isEmpty(); }
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
// EnumStmt moved after SubscriptStmt definition to avoid forward declaration issues

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

// Struct declaration: struct Name<T>: Protocol1, Protocol2 where T: Equatable { members }
struct StructStmt : Stmt {
  Token name;
  AccessLevel accessLevel; // Access level for the struct
  GenericParameterClause genericParams; // Generic type parameters
  std::vector<Token> conformedProtocols; // Protocols this struct conforms to
  WhereClause whereClause; // Generic constraints
  std::vector<StructMember> members;
  std::vector<std::unique_ptr<FunctionStmt>>
      methods; // Methods defined in the struct
  std::vector<std::unique_ptr<InitStmt>> initializers; // Constructors
  std::unique_ptr<DeinitStmt> deinitializer;           // Destructor
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts; // Subscripts
  std::vector<std::unique_ptr<Stmt>> nestedTypes; // Nested types (structs, classes, enums)
  bool isMain; // Whether this struct has @main attribute

  StructStmt(Token name, std::vector<StructMember> members,
             std::vector<std::unique_ptr<FunctionStmt>> methods = {},
             std::vector<std::unique_ptr<InitStmt>> initializers = {},
             std::unique_ptr<DeinitStmt> deinitializer = nullptr,
             std::vector<std::unique_ptr<SubscriptStmt>> subscripts = {},
             std::vector<Token> conformedProtocols = {},
             AccessLevel accessLevel = AccessLevel::INTERNAL,
             GenericParameterClause genericParams = GenericParameterClause({}),
             WhereClause whereClause = WhereClause({}),
             std::vector<std::unique_ptr<Stmt>> nestedTypes = {},
             bool isMain = false)
      : name(name), accessLevel(accessLevel), genericParams(std::move(genericParams)),
        conformedProtocols(std::move(conformedProtocols)), whereClause(std::move(whereClause)),
        members(std::move(members)), methods(std::move(methods)),
        initializers(std::move(initializers)),
        deinitializer(std::move(deinitializer)), subscripts(std::move(subscripts)),
        nestedTypes(std::move(nestedTypes)), isMain(isMain) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    // For simplicity, we'll create a basic clone without deep copying members
    // and methods This is sufficient for our current use case
    return std::make_unique<StructStmt>(
        name, std::vector<StructMember>(),
        std::vector<std::unique_ptr<FunctionStmt>>());
  }
  
  // Check if this is a generic struct
  bool isGeneric() const { return !genericParams.isEmpty(); }
};

// Class declaration: class Name<T>: Superclass, Protocol1, Protocol2 where T: Equatable { members }
struct ClassStmt : Stmt {
  Token name;
  Token superclass; // Optional superclass
  AccessLevel accessLevel; // Access level for the class
  GenericParameterClause genericParams; // Generic type parameters
  std::vector<Token> conformedProtocols; // Protocols this class conforms to
  WhereClause whereClause; // Generic constraints
  std::vector<StructMember> members;
  std::vector<std::unique_ptr<FunctionStmt>> methods;
  std::vector<std::unique_ptr<InitStmt>> initializers; // Constructors
  std::unique_ptr<DeinitStmt> deinitializer;           // Destructor
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts; // Subscripts
  std::vector<std::unique_ptr<Stmt>> nestedTypes; // Nested types (structs, classes, enums)
  bool isMain; // Whether this class has @main attribute

  ClassStmt(Token name, Token superclass, std::vector<StructMember> members,
            std::vector<std::unique_ptr<FunctionStmt>> methods = {},
            std::vector<std::unique_ptr<InitStmt>> initializers = {},
            std::unique_ptr<DeinitStmt> deinitializer = nullptr,
            std::vector<std::unique_ptr<SubscriptStmt>> subscripts = {},
            std::vector<Token> conformedProtocols = {},
            AccessLevel accessLevel = AccessLevel::INTERNAL,
            GenericParameterClause genericParams = GenericParameterClause({}),
            WhereClause whereClause = WhereClause({}),
            std::vector<std::unique_ptr<Stmt>> nestedTypes = {},
            bool isMain = false)
      : name(name), superclass(superclass), accessLevel(accessLevel),
        genericParams(std::move(genericParams)), conformedProtocols(std::move(conformedProtocols)),
        whereClause(std::move(whereClause)), members(std::move(members)),
        methods(std::move(methods)), initializers(std::move(initializers)),
        deinitializer(std::move(deinitializer)), subscripts(std::move(subscripts)),
        nestedTypes(std::move(nestedTypes)), isMain(isMain) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    // For simplicity, we'll create a basic clone without deep copying members
    // and methods This is sufficient for our current use case
    return std::make_unique<ClassStmt>(
        name, superclass, std::vector<StructMember>(),
        std::vector<std::unique_ptr<FunctionStmt>>());
  }
  
  // Check if this is a generic class
  bool isGeneric() const { return !genericParams.isEmpty(); }
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

// Enum declaration: enum Name: RawType { cases }
struct EnumStmt : Stmt {
  EnumStmt(Token name, Token rawType, std::vector<EnumCase> cases,
           std::vector<std::unique_ptr<SubscriptStmt>> subscripts = {},
           std::vector<std::unique_ptr<Stmt>> nestedTypes = {})
      : name(name), rawType(rawType), cases(std::move(cases)), 
        subscripts(std::move(subscripts)), nestedTypes(std::move(nestedTypes)) {}

  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }

  std::unique_ptr<Stmt> clone() const override {
    std::vector<EnumCase> clonedCases;
    for (const auto &enumCase : cases) {
      clonedCases.emplace_back(enumCase.name, enumCase.associatedTypes,
                               enumCase.rawValue ? enumCase.rawValue->clone()
                                                 : nullptr);
    }
    
    std::vector<std::unique_ptr<SubscriptStmt>> clonedSubscripts;
    for (const auto &subscript : subscripts) {
      auto clonedStmt = subscript->clone();
      clonedSubscripts.push_back(std::unique_ptr<SubscriptStmt>(static_cast<SubscriptStmt*>(clonedStmt.release())));
    }
    
    std::vector<std::unique_ptr<Stmt>> clonedNestedTypes;
    for (const auto &nestedType : nestedTypes) {
      clonedNestedTypes.push_back(nestedType->clone());
    }
    
    return std::make_unique<EnumStmt>(name, rawType, std::move(clonedCases), std::move(clonedSubscripts), std::move(clonedNestedTypes));
  }

  const Token name;
  const Token rawType; // Can be empty for no raw type
  const std::vector<EnumCase> cases;
  const std::vector<std::unique_ptr<SubscriptStmt>> subscripts; // Static subscripts
  const std::vector<std::unique_ptr<Stmt>> nestedTypes; // Nested types (structs, classes, enums)
};

// Protocol requirement types
enum class ProtocolRequirementType {
  PROPERTY,    // var/let property requirement
  METHOD,      // func method requirement
  INITIALIZER, // init requirement
  SUBSCRIPT    // subscript requirement
};

// Protocol property requirement
struct ProtocolPropertyRequirement {
  Token name;
  Token type;
  bool isVar;        // true for var, false for let
  bool isStatic;     // true for static properties
  bool hasGetter;    // { get }
  bool hasSetter;    // { get set }
  
  ProtocolPropertyRequirement(Token name, Token type, bool isVar = true, 
                             bool isStatic = false, bool hasGetter = true, 
                             bool hasSetter = false)
      : name(name), type(type), isVar(isVar), isStatic(isStatic),
        hasGetter(hasGetter), hasSetter(hasSetter) {}
};

// Protocol method requirement
struct ProtocolMethodRequirement {
  Token name;
  std::vector<Parameter> parameters;
  Token returnType;
  bool isStatic;     // true for static methods
  bool isMutating;   // true for mutating methods
  
  ProtocolMethodRequirement(Token name, std::vector<Parameter> parameters,
                           Token returnType, bool isStatic = false,
                           bool isMutating = false)
      : name(name), parameters(std::move(parameters)), returnType(returnType),
        isStatic(isStatic), isMutating(isMutating) {}
};

// Protocol initializer requirement
struct ProtocolInitRequirement {
  std::vector<Parameter> parameters;
  bool isFailable;   // true for init?
  
  ProtocolInitRequirement(std::vector<Parameter> parameters, bool isFailable = false)
      : parameters(std::move(parameters)), isFailable(isFailable) {}
};

// Protocol subscript requirement
struct ProtocolSubscriptRequirement {
  std::vector<Parameter> parameters;
  Token returnType;
  bool isStatic;     // true for static subscripts
  bool hasGetter;    // { get }
  bool hasSetter;    // { get set }
  
  ProtocolSubscriptRequirement(std::vector<Parameter> parameters, Token returnType,
                              bool isStatic = false, bool hasGetter = true,
                              bool hasSetter = false)
      : parameters(std::move(parameters)), returnType(returnType),
        isStatic(isStatic), hasGetter(hasGetter), hasSetter(hasSetter) {}
};

// Protocol requirement (union of all requirement types)
struct ProtocolRequirement {
  ProtocolRequirementType type;
  
  // Union-like storage for different requirement types
  std::unique_ptr<ProtocolPropertyRequirement> propertyReq;
  std::unique_ptr<ProtocolMethodRequirement> methodReq;
  std::unique_ptr<ProtocolInitRequirement> initReq;
  std::unique_ptr<ProtocolSubscriptRequirement> subscriptReq;
  
  // Constructors for different requirement types
  explicit ProtocolRequirement(std::unique_ptr<ProtocolPropertyRequirement> req)
      : type(ProtocolRequirementType::PROPERTY), propertyReq(std::move(req)) {}
      
  explicit ProtocolRequirement(std::unique_ptr<ProtocolMethodRequirement> req)
      : type(ProtocolRequirementType::METHOD), methodReq(std::move(req)) {}
      
  explicit ProtocolRequirement(std::unique_ptr<ProtocolInitRequirement> req)
      : type(ProtocolRequirementType::INITIALIZER), initReq(std::move(req)) {}
      
  explicit ProtocolRequirement(std::unique_ptr<ProtocolSubscriptRequirement> req)
      : type(ProtocolRequirementType::SUBSCRIPT), subscriptReq(std::move(req)) {}
};

// Protocol declaration: protocol Name: SuperProtocol { requirements }
struct ProtocolStmt : Stmt {
  Token name;
  std::vector<Token> inheritedProtocols;  // Protocols this protocol inherits from
  std::vector<ProtocolRequirement> requirements;
  AccessLevel accessLevel;
  
  ProtocolStmt(Token name, std::vector<Token> inheritedProtocols,
               std::vector<ProtocolRequirement> requirements,
               AccessLevel accessLevel = AccessLevel::INTERNAL)
      : name(name), inheritedProtocols(std::move(inheritedProtocols)),
        requirements(std::move(requirements)), accessLevel(accessLevel) {}
        
  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<Stmt> clone() const override {
    // For simplicity, create a basic clone without deep copying requirements
    return std::make_unique<ProtocolStmt>(name, inheritedProtocols,
                                          std::vector<ProtocolRequirement>(),
                                          accessLevel);
  }
};

// Extension declaration: extension TypeName: Protocol1, Protocol2 where T: Equatable { members }
struct ExtensionStmt : Stmt {
  Token typeName;                                    // Type being extended
  std::vector<Token> conformedProtocols;             // Protocols to conform to
  GenericParameterClause genericParams;              // Generic parameters (for generic extensions)
  WhereClause whereClause;                          // Generic constraints
  AccessLevel accessLevel;                          // Access level for the extension
  
  // Extension members
  std::vector<StructMember> properties;             // Computed properties
  std::vector<std::unique_ptr<FunctionStmt>> methods; // Methods
  std::vector<std::unique_ptr<InitStmt>> initializers; // Convenience initializers
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts; // Subscripts
  
  ExtensionStmt(Token typeName, 
                std::vector<Token> conformedProtocols = {},
                GenericParameterClause genericParams = GenericParameterClause({}),
                WhereClause whereClause = WhereClause({}),
                AccessLevel accessLevel = AccessLevel::INTERNAL,
                std::vector<StructMember> properties = {},
                std::vector<std::unique_ptr<FunctionStmt>> methods = {},
                std::vector<std::unique_ptr<InitStmt>> initializers = {},
                std::vector<std::unique_ptr<SubscriptStmt>> subscripts = {})
      : typeName(typeName), conformedProtocols(std::move(conformedProtocols)),
        genericParams(std::move(genericParams)), whereClause(std::move(whereClause)),
        accessLevel(accessLevel), properties(std::move(properties)),
        methods(std::move(methods)), initializers(std::move(initializers)),
        subscripts(std::move(subscripts)) {}
        
  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<Stmt> clone() const override {
    // For simplicity, create a basic clone without deep copying members
    return std::make_unique<ExtensionStmt>(typeName, conformedProtocols,
                                           genericParams, whereClause,
                                           accessLevel);
  }
  
  // Check if this is a conditional extension (has where clause)
  bool isConditional() const { return !whereClause.isEmpty(); }
  
  // Check if this extension adds protocol conformance
  bool addsProtocolConformance() const { return !conformedProtocols.empty(); }
};

// Switch case: case pattern: statements
struct SwitchCase {
  std::unique_ptr<Expr> pattern;  // Pattern to match (can be literal, enum case, etc.)
  std::vector<std::unique_ptr<Stmt>> statements;  // Statements to execute
  bool isDefault;  // true for default case
  
  SwitchCase(std::unique_ptr<Expr> pattern, std::vector<std::unique_ptr<Stmt>> statements, bool isDefault = false)
      : pattern(std::move(pattern)), statements(std::move(statements)), isDefault(isDefault) {}
};

// Switch statement: switch expression { cases }
struct SwitchStmt : Stmt {
  std::unique_ptr<Expr> expression;  // Expression to switch on
  std::vector<SwitchCase> cases;     // Switch cases
  
  SwitchStmt(std::unique_ptr<Expr> expression, std::vector<SwitchCase> cases)
      : expression(std::move(expression)), cases(std::move(cases)) {}
      
  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<Stmt> clone() const override {
    std::vector<SwitchCase> clonedCases;
    for (const auto &switchCase : cases) {
      std::vector<std::unique_ptr<Stmt>> clonedStatements;
      for (const auto &stmt : switchCase.statements) {
        clonedStatements.push_back(stmt->clone());
      }
      clonedCases.emplace_back(
          switchCase.pattern ? switchCase.pattern->clone() : nullptr,
          std::move(clonedStatements),
          switchCase.isDefault
      );
    }
    return std::make_unique<SwitchStmt>(expression->clone(), std::move(clonedCases));
  }
};

// Custom operator declaration: operator +++ { associativity left precedence 140 }
struct CustomOperatorStmt : Stmt {
  Token operatorSymbol;  // The operator symbol (e.g., +++, +-)
  Token operatorType;    // prefix, infix, or postfix
  
  CustomOperatorStmt(Token operatorSymbol, Token operatorType)
      : operatorSymbol(operatorSymbol), operatorType(operatorType) {}
      
  void accept(miniswift::StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<miniswift::Stmt> clone() const override {
    return std::make_unique<CustomOperatorStmt>(operatorSymbol, operatorType);
  }
};

// Operator precedence declaration: precedencegroup MyPrecedence { associativity: left higherThan: AdditionPrecedence }
struct OperatorPrecedenceStmt : Stmt {
  miniswift::Token precedenceGroupName;  // Name of the precedence group
  miniswift::Token associativity;        // left, right, or none
  int precedenceLevel;        // Numeric precedence level
  std::vector<miniswift::Token> higherThan;  // Precedence groups this is higher than
  std::vector<miniswift::Token> lowerThan;   // Precedence groups this is lower than
  
  OperatorPrecedenceStmt(miniswift::Token precedenceGroupName, miniswift::Token associativity, int precedenceLevel,
                        std::vector<miniswift::Token> higherThan = {}, std::vector<miniswift::Token> lowerThan = {})
      : precedenceGroupName(precedenceGroupName), associativity(associativity),
        precedenceLevel(precedenceLevel), higherThan(std::move(higherThan)),
        lowerThan(std::move(lowerThan)) {}
        
  void accept(miniswift::StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<miniswift::Stmt> clone() const override {
    return std::make_unique<OperatorPrecedenceStmt>(precedenceGroupName, associativity,
                                                   precedenceLevel, higherThan, lowerThan);
  }
};

// Result Builder declaration: @resultBuilder struct DrawingBuilder { ... }
struct ResultBuilderStmt : Stmt {
  miniswift::Token name;  // Name of the result builder
  std::vector<std::unique_ptr<FunctionStmt>> buildMethods;  // buildBlock, buildIf, etc.
  AccessLevel accessLevel;
  
  ResultBuilderStmt(miniswift::Token name, std::vector<std::unique_ptr<FunctionStmt>> buildMethods,
                   AccessLevel accessLevel = AccessLevel::INTERNAL)
      : name(name), buildMethods(std::move(buildMethods)), accessLevel(accessLevel) {}
      
  void accept(miniswift::StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<miniswift::Stmt> clone() const override {
    std::vector<std::unique_ptr<FunctionStmt>> clonedMethods;
    for (const auto &method : buildMethods) {
      clonedMethods.push_back(std::unique_ptr<FunctionStmt>(static_cast<FunctionStmt*>(method->clone().release())));
    }
    return std::make_unique<ResultBuilderStmt>(name, std::move(clonedMethods), accessLevel);
  }
};

// Actor declaration: actor ActorName { properties and methods }
struct ActorStmt : Stmt {
  Token name;                                        // Actor name
  std::vector<StructMember> properties;             // Actor properties
  std::vector<std::unique_ptr<FunctionStmt>> methods; // Actor methods
  std::vector<std::unique_ptr<InitStmt>> initializers; // Actor initializers
  AccessLevel accessLevel;                          // Access level
  bool isGlobalActor;                              // Whether this is a global actor
  
  ActorStmt(Token name, 
            std::vector<StructMember> properties = {},
            std::vector<std::unique_ptr<FunctionStmt>> methods = {},
            std::vector<std::unique_ptr<InitStmt>> initializers = {},
            AccessLevel accessLevel = AccessLevel::INTERNAL,
            bool isGlobalActor = false)
      : name(name), properties(std::move(properties)),
        methods(std::move(methods)), initializers(std::move(initializers)),
        accessLevel(accessLevel), isGlobalActor(isGlobalActor) {}
        
  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<Stmt> clone() const override {
    // For simplicity, create a basic clone without deep copying members
    return std::make_unique<ActorStmt>(name, std::vector<StructMember>(),
                                       std::vector<std::unique_ptr<FunctionStmt>>(),
                                       std::vector<std::unique_ptr<InitStmt>>(),
                                       accessLevel, isGlobalActor);
  }
};

// Macro declaration: macro name(parameters) -> ReturnType = #externalMacro(module: "ModuleName", type: "TypeName")
struct MacroStmt : Stmt {
  Token name;                                        // Macro name
  std::vector<Parameter> parameters;                 // Macro parameters
  Token returnType;                                  // Return type
  std::unique_ptr<Expr> implementation;              // Macro implementation (can be external reference)
  AccessLevel accessLevel;                          // Access level
  bool isFreestanding;                              // Whether this is a freestanding macro
  std::vector<Token> roles;                         // Macro roles (e.g., expression, declaration)
  
  MacroStmt(Token name, 
            std::vector<Parameter> parameters = {},
            Token returnType = Token(),
            std::unique_ptr<Expr> implementation = nullptr,
            AccessLevel accessLevel = AccessLevel::INTERNAL,
            bool isFreestanding = true,
            std::vector<Token> roles = {})
      : name(name), parameters(std::move(parameters)),
        returnType(returnType), implementation(std::move(implementation)),
        accessLevel(accessLevel), isFreestanding(isFreestanding),
        roles(std::move(roles)) {}
        
  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<Stmt> clone() const override {
    std::vector<Parameter> clonedParams;
    for (const auto &param : parameters) {
      clonedParams.push_back(param);
    }
    return std::make_unique<MacroStmt>(name, std::move(clonedParams),
                                       returnType, 
                                       implementation ? implementation->clone() : nullptr,
                                       accessLevel, isFreestanding, roles);
  }
};

// External macro declaration: macro name(parameters) -> ReturnType = #externalMacro(module: "ModuleName", type: "TypeName")
struct ExternalMacroStmt : Stmt {
  Token name;                                        // Macro name
  std::vector<Parameter> parameters;                 // Macro parameters
  Token returnType;                                  // Return type
  std::string moduleName;                           // External module name
  std::string typeName;                             // External type name
  AccessLevel accessLevel;                          // Access level
  std::vector<Token> roles;                         // Macro roles
  
  ExternalMacroStmt(Token name, 
                    std::vector<Parameter> parameters,
                    Token returnType,
                    std::string moduleName,
                    std::string typeName,
                    AccessLevel accessLevel = AccessLevel::INTERNAL,
                    std::vector<Token> roles = {})
      : name(name), parameters(std::move(parameters)),
        returnType(returnType), moduleName(std::move(moduleName)),
        typeName(std::move(typeName)), accessLevel(accessLevel),
        roles(std::move(roles)) {}
        
  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<Stmt> clone() const override {
    std::vector<Parameter> clonedParams;
    for (const auto &param : parameters) {
      clonedParams.push_back(param);
    }
    return std::make_unique<ExternalMacroStmt>(name, std::move(clonedParams),
                                               returnType, moduleName, typeName,
                                               accessLevel, roles);
  }
};

// Freestanding macro declaration: @freestanding(expression) macro name(parameters) -> ReturnType
struct FreestandingMacroStmt : Stmt {
  Token name;                                        // Macro name
  std::vector<Parameter> parameters;                 // Macro parameters
  Token returnType;                                  // Return type
  std::unique_ptr<Expr> body;                       // Macro body implementation
  AccessLevel accessLevel;                          // Access level
  Token role;                                       // Macro role (expression, declaration, etc.)
  
  FreestandingMacroStmt(Token name, 
                        std::vector<Parameter> parameters,
                        Token returnType,
                        std::unique_ptr<Expr> body,
                        AccessLevel accessLevel = AccessLevel::INTERNAL,
                        Token role = Token())
      : name(name), parameters(std::move(parameters)),
        returnType(returnType), body(std::move(body)),
        accessLevel(accessLevel), role(role) {}
        
  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<Stmt> clone() const override {
    std::vector<Parameter> clonedParams;
    for (const auto &param : parameters) {
      clonedParams.push_back(param);
    }
    return std::make_unique<FreestandingMacroStmt>(name, std::move(clonedParams),
                                                    returnType, 
                                                    body ? body->clone() : nullptr,
                                                    accessLevel, role);
  }
};

// Attached macro declaration: @attached(member) macro name(parameters)
struct AttachedMacroStmt : Stmt {
  Token name;                                        // Macro name
  std::vector<Parameter> parameters;                 // Macro parameters
  std::unique_ptr<Expr> body;                       // Macro body implementation
  AccessLevel accessLevel;                          // Access level
  Token attachmentKind;                             // Attachment kind (member, memberAttribute, etc.)
  std::vector<Token> names;                         // Names introduced by the macro
  
  AttachedMacroStmt(Token name, 
                    std::vector<Parameter> parameters,
                    std::unique_ptr<Expr> body,
                    AccessLevel accessLevel = AccessLevel::INTERNAL,
                    Token attachmentKind = Token(),
                    std::vector<Token> names = {})
      : name(name), parameters(std::move(parameters)),
        body(std::move(body)), accessLevel(accessLevel),
        attachmentKind(attachmentKind), names(std::move(names)) {}
        
  void accept(StmtVisitor &visitor) const override { visitor.visit(*this); }
  
  std::unique_ptr<Stmt> clone() const override {
    std::vector<Parameter> clonedParams;
    for (const auto &param : parameters) {
      clonedParams.push_back(param);
    }
    return std::make_unique<AttachedMacroStmt>(name, std::move(clonedParams),
                                               body ? body->clone() : nullptr,
                                               accessLevel, attachmentKind, names);
  }
};

} // namespace miniswift

#endif // MINISWIFT_STMT_NODES_H