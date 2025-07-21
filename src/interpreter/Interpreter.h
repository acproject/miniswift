#ifndef MINISWIFT_INTERPRETER_H
#define MINISWIFT_INTERPRETER_H

#include "../parser/AST.h"
#include "Value.h"
#include "Environment.h"
#include "OOP/Property.h"
#include "OOP/Inheritance.h"
#include "OOP/Subscript.h"
#include "OOP/Optional.h"
#include <memory>
#include <unordered_map>

namespace miniswift {

class Interpreter : public ExprVisitor, public StmtVisitor {
public:
    Interpreter();
    virtual ~Interpreter() = default;
public:
    void interpret(const std::vector<std::unique_ptr<Stmt>>& statements);

    void visit(const ExprStmt& stmt) override;
    void visit(const PrintStmt& stmt) override;
    void visit(const VarStmt& stmt) override;
    void visit(const BlockStmt& stmt) override;
    void visit(const IfStmt& stmt) override;
    void visit(const WhileStmt& stmt) override;
    void visit(const ForStmt& stmt) override;
    void visit(const ForInStmt& stmt) override;
    void visit(const FunctionStmt& stmt) override;
    void visit(const ReturnStmt& stmt) override;
    void visit(const EnumStmt& stmt) override;
    void visit(const StructStmt& stmt) override;
    void visit(const ClassStmt& stmt) override;
    void visit(const InitStmt& stmt) override;
    void visit(const DeinitStmt& stmt) override;
    void visit(const SubscriptStmt& stmt) override;
    void visit(const ProtocolStmt& stmt) override;

    void visit(const Binary& expr) override;
    void visit(const Grouping& expr) override;
    void visit(const Literal& expr) override;
    void visit(const Unary& expr) override;
    void visit(const VarExpr& expr) override;
    void visit(const Assign& expr) override;
    void visit(const ArrayLiteral& expr) override;
    void visit(const DictionaryLiteral& expr) override;
    void visit(const IndexAccess& expr) override;
    void visit(const SubscriptAccess& expr) override;
    void visit(const Call& expr) override;
    void visit(const Closure& expr) override;
    void visit(const EnumAccess& expr) override;
    void visit(const MemberAccess& expr) override;
    void visit(const StructInit& expr) override;
    void visit(const Super& expr) override;
    void visit(const StringInterpolation& expr) override;
    void visit(const OptionalChaining& expr) override;
    void visit(const Range& expr) override;

public:
    // Public methods for property system
    Value evaluate(const Expr& expr);
    void executeWithEnvironment(const Stmt& stmt, std::shared_ptr<Environment> env);
    
    // Environment access methods for OOP system
    std::shared_ptr<Environment> getCurrentEnvironment() const { return environment; }
    void setCurrentEnvironment(std::shared_ptr<Environment> env) { environment = env; }
    
protected:
    std::shared_ptr<Environment> environment;
    std::shared_ptr<Environment> globals;
    
private:
    
    // Property managers for types
    std::unordered_map<std::string, std::unique_ptr<PropertyManager>> structPropertyManagers;
    std::unordered_map<std::string, std::unique_ptr<PropertyManager>> classPropertyManagers;
    
    // Inheritance management
    std::unique_ptr<InheritanceManager> inheritanceManager;
    std::unique_ptr<SuperHandler> superHandler;
    
    // Subscript management
    std::unique_ptr<StaticSubscriptManager> staticSubscriptManager;
    
    bool isTruthy(const Value& value);
    void printArray(const Array& arr);
    void printDictionary(const Dictionary& dict);
    void printValue(const Value& val);
    
    // Property system helpers
    PropertyManager* getStructPropertyManager(const std::string& structName);
    PropertyManager* getClassPropertyManager(const std::string& className);
    void registerStructProperties(const std::string& structName, const std::vector<StructMember>& members);
    void registerClassProperties(const std::string& className, const std::vector<StructMember>& members);
    

    
public:
    // Enhanced member access with property support
    virtual Value getMemberValue(const Value& object, const std::string& memberName);
    void setMemberValue(Value& object, const std::string& memberName, const Value& value);

private:

    Value result;
};

} // namespace miniswift

#endif // MINISWIFT_INTERPRETER_H