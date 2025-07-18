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
struct Call;
struct Closure;
struct EnumAccess;
struct MemberAccess;
struct StructInit;

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
    virtual void visit(const Call& expr) = 0;
    virtual void visit(const Closure& expr) = 0;
    virtual void visit(const EnumAccess& expr) = 0;
    virtual void visit(const MemberAccess& expr) = 0;
    virtual void visit(const StructInit& expr) = 0;
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
    Assign(std::unique_ptr<Expr> target, std::unique_ptr<Expr> value)
        : target(std::move(target)), value(std::move(value)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<Assign>(target->clone(), value->clone());
    }

    const std::unique_ptr<Expr> target;  // Can be VarExpr or MemberAccess
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

// Function call expression: functionName(arguments)
struct Call : Expr {
    Call(std::unique_ptr<Expr> callee, std::vector<std::unique_ptr<Expr>> arguments)
        : callee(std::move(callee)), arguments(std::move(arguments)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        std::vector<std::unique_ptr<Expr>> clonedArguments;
        for (const auto& arg : arguments) {
            clonedArguments.push_back(arg->clone());
        }
        return std::make_unique<Call>(callee->clone(), std::move(clonedArguments));
    }

    const std::unique_ptr<Expr> callee;
    const std::vector<std::unique_ptr<Expr>> arguments;
};



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
    virtual void visit(const FunctionStmt& stmt) = 0;
    virtual void visit(const ReturnStmt& stmt) = 0;
    virtual void visit(const EnumStmt& stmt) = 0;
    virtual void visit(const StructStmt& stmt) = 0;
    virtual void visit(const ClassStmt& stmt) = 0;
};

// Base class for Stmt
class Stmt {
public:
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) const = 0;
    virtual std::unique_ptr<Stmt> clone() const = 0;
};

// Concrete statement classes
struct ExprStmt : Stmt {
    explicit ExprStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<ExprStmt>(expression->clone());
    }

    const std::unique_ptr<Expr> expression;
};

struct PrintStmt : Stmt {
    explicit PrintStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<PrintStmt>(expression->clone());
    }

    const std::unique_ptr<Expr> expression;
};

struct VarStmt : Stmt {
    VarStmt(Token name, std::unique_ptr<Expr> initializer, bool isConst, Token type)
        : name(name), initializer(std::move(initializer)), isConst(isConst), type(type) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<VarStmt>(name, initializer ? initializer->clone() : nullptr, isConst, type);
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

    std::unique_ptr<Stmt> clone() const override {
        std::vector<std::unique_ptr<Stmt>> clonedStatements;
        for (const auto& stmt : statements) {
            clonedStatements.push_back(stmt->clone());
        }
        return std::make_unique<BlockStmt>(std::move(clonedStatements));
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

    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<IfStmt>(condition->clone(), thenBranch->clone(), elseBranch ? elseBranch->clone() : nullptr);
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

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<ForStmt>(
            initializer ? initializer->clone() : nullptr,
            condition ? condition->clone() : nullptr,
            increment ? increment->clone() : nullptr,
            body->clone()
        );
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
    Closure(std::vector<Parameter> parameters, Token returnType, std::vector<std::unique_ptr<Stmt>> body)
        : parameters(std::move(parameters)), returnType(returnType), body(std::move(body)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        // For now, we'll create a shallow copy that shares the same body
        // This is a temporary solution until we implement proper Stmt cloning
        std::vector<std::unique_ptr<Stmt>> clonedBody;
        // We can't clone statements easily, so we'll avoid cloning closures for now
        // This is a limitation that should be addressed in a full implementation
        return std::make_unique<Closure>(parameters, returnType, std::move(clonedBody));
    }

    const std::vector<Parameter> parameters;
    const Token returnType; // Can be empty for type inference
    const std::vector<std::unique_ptr<Stmt>> body;
};

// Function declaration: func name(parameters) -> returnType { body }
struct FunctionStmt : Stmt {
    FunctionStmt(Token name, std::vector<Parameter> parameters, Token returnType, std::unique_ptr<Stmt> body)
        : name(name), parameters(std::move(parameters)), returnType(returnType), body(std::move(body)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<FunctionStmt>(name, parameters, returnType, body->clone());
    }

    const Token name;
    const std::vector<Parameter> parameters;
    const Token returnType; // Can be empty for Void
    const std::unique_ptr<Stmt> body;
};

// Return statement: return expression
struct ReturnStmt : Stmt {
    explicit ReturnStmt(std::unique_ptr<Expr> value)
        : value(std::move(value)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Stmt> clone() const override {
        return std::make_unique<ReturnStmt>(value ? value->clone() : nullptr);
    }

    const std::unique_ptr<Expr> value; // Can be null for void return
};

// Enum case definition
struct EnumCase {
    Token name;
    std::vector<Token> associatedTypes; // For associated values like case upc(Int, Int, Int, Int)
    std::unique_ptr<Expr> rawValue; // For raw values like case mercury = 1
    
    EnumCase(Token n, std::vector<Token> types = {}, std::unique_ptr<Expr> value = nullptr)
        : name(n), associatedTypes(std::move(types)), rawValue(std::move(value)) {}
};

// Enum declaration: enum Name: RawType { cases }
struct EnumStmt : Stmt {
    EnumStmt(Token name, Token rawType, std::vector<EnumCase> cases)
        : name(name), rawType(rawType), cases(std::move(cases)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Stmt> clone() const override {
        std::vector<EnumCase> clonedCases;
        for (const auto& enumCase : cases) {
            clonedCases.emplace_back(
                enumCase.name,
                enumCase.associatedTypes,
                enumCase.rawValue ? enumCase.rawValue->clone() : nullptr
            );
        }
        return std::make_unique<EnumStmt>(name, rawType, std::move(clonedCases));
    }

    const Token name;
    const Token rawType; // Can be empty for no raw type
    const std::vector<EnumCase> cases;
};

// Enum case access expression: EnumName.caseName or .caseName
struct EnumAccess : Expr {
    std::unique_ptr<Expr> enumType;
    Token caseName;
    std::vector<std::unique_ptr<Expr>> arguments;
    
    EnumAccess(std::unique_ptr<Expr> enumType, Token caseName, std::vector<std::unique_ptr<Expr>> arguments = {})
        : enumType(std::move(enumType)), caseName(caseName), arguments(std::move(arguments)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        std::vector<std::unique_ptr<Expr>> clonedValues;
        for (const auto& value : arguments) {
            clonedValues.push_back(value->clone());
        }
        return std::make_unique<EnumAccess>(enumType ? enumType->clone() : nullptr, caseName, std::move(clonedValues));
    }

    
};

// Member access expression: object.member
struct MemberAccess : Expr {
    std::unique_ptr<Expr> object;
    Token member;
    
    MemberAccess(std::unique_ptr<Expr> object, Token member)
        : object(std::move(object)), member(member) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        return std::make_unique<MemberAccess>(object->clone(), member);
    }
};

// Struct initialization expression: StructName(member1: value1, member2: value2)
struct StructInit : Expr {
    Token structName;
    std::vector<std::pair<Token, std::unique_ptr<Expr>>> members; // (memberName, value)
    
    StructInit(Token structName, std::vector<std::pair<Token, std::unique_ptr<Expr>>> members)
        : structName(structName), members(std::move(members)) {}

    void accept(ExprVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Expr> clone() const override {
        std::vector<std::pair<Token, std::unique_ptr<Expr>>> clonedMembers;
        for (const auto& member : members) {
            clonedMembers.emplace_back(member.first, member.second->clone());
        }
        return std::make_unique<StructInit>(structName, std::move(clonedMembers));
    }
};

// Property accessor types
enum class AccessorType {
    GET,         // getter
    SET,         // setter
    WILL_SET,    // willSet observer
    DID_SET      // didSet observer
};

// Property accessor definition
struct PropertyAccessor {
    AccessorType type;
    std::unique_ptr<Stmt> body;
    std::string parameterName; // Optional parameter name for willSet/didSet
    
    PropertyAccessor(AccessorType t, std::unique_ptr<Stmt> b = nullptr, const std::string& param = "")
        : type(t), body(std::move(b)), parameterName(param) {}
};

// Enhanced struct member definition with property support
struct StructMember {
    Token name;
    Token type;
    std::unique_ptr<Expr> defaultValue; // Optional default value
    bool isVar; // true for var, false for let
    bool isStatic; // true for static properties
    bool isLazy; // true for lazy properties
    
    // Property accessors (for computed properties and observers)
    std::vector<PropertyAccessor> accessors;
    
    StructMember(Token name, Token type, std::unique_ptr<Expr> defaultValue = nullptr, bool isVar = true)
        : name(name), type(type), defaultValue(std::move(defaultValue)), isVar(isVar), 
          isStatic(false), isLazy(false) {}
    
    // Check if this is a computed property
    bool isComputedProperty() const {
        return hasAccessor(AccessorType::GET) || hasAccessor(AccessorType::SET);
    }
    
    // Check if this member has a specific accessor
    bool hasAccessor(AccessorType type) const {
        for (const auto& accessor : accessors) {
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
    std::vector<StructMember> members;
    std::vector<std::unique_ptr<FunctionStmt>> methods; // Methods defined in the struct
    
    StructStmt(Token name, std::vector<StructMember> members, std::vector<std::unique_ptr<FunctionStmt>> methods = {})
        : name(name), members(std::move(members)), methods(std::move(methods)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Stmt> clone() const override {
        // For simplicity, we'll create a basic clone without deep copying members and methods
        // This is sufficient for our current use case
        return std::make_unique<StructStmt>(name, std::vector<StructMember>(), std::vector<std::unique_ptr<FunctionStmt>>());
    }
};

// Class declaration: class Name { members }
struct ClassStmt : Stmt {
    Token name;
    Token superclass; // Optional superclass
    std::vector<StructMember> members;
    std::vector<std::unique_ptr<FunctionStmt>> methods;
    
    ClassStmt(Token name, Token superclass, std::vector<StructMember> members, std::vector<std::unique_ptr<FunctionStmt>> methods = {})
        : name(name), superclass(superclass), members(std::move(members)), methods(std::move(methods)) {}

    void accept(StmtVisitor& visitor) const override {
        visitor.visit(*this);
    }

    std::unique_ptr<Stmt> clone() const override {
        // For simplicity, we'll create a basic clone without deep copying members and methods
        // This is sufficient for our current use case
        return std::make_unique<ClassStmt>(name, superclass, std::vector<StructMember>(), std::vector<std::unique_ptr<FunctionStmt>>());
    }
};

} // namespace miniswift

#endif // MINISWIFT_AST_H