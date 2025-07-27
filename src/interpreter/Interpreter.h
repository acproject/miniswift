#ifndef MINISWIFT_INTERPRETER_H
#define MINISWIFT_INTERPRETER_H

#include "../parser/AST.h"
#include "Value.h"
#include "Environment.h"
#include "ErrorHandling.h"
#include "OOP/Property.h"
#include "OOP/Inheritance.h"
#include "OOP/Subscript.h"
#include "OOP/Optional.h"
#include <memory>
#include <unordered_map>
#include <stack>

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
    void visit(const IfLetStmt& stmt) override;
    void visit(const WhileStmt& stmt) override;
    void visit(const ForStmt& stmt) override;
    void visit(const ForInStmt& stmt) override;
    void visit(const ForAwaitStmt& stmt) override;
    void visit(const FunctionStmt& stmt) override;
    void visit(const ReturnStmt& stmt) override;
    // Control transfer statements
    void visit(const ContinueStmt& stmt) override;
    void visit(const BreakStmt& stmt) override;
    void visit(const FallthroughStmt& stmt) override;
    void visit(const EnumStmt& stmt) override;
    void visit(const StructStmt& stmt) override;
    void visit(const ClassStmt& stmt) override;
    void visit(const InitStmt& stmt) override;
    void visit(const DeinitStmt& stmt) override;
    void visit(const SubscriptStmt& stmt) override;
    void visit(const ProtocolStmt& stmt) override;
    void visit(const ExtensionStmt& stmt) override;
    // Error handling statements
    void visit(const ThrowStmt& stmt) override;
    void visit(const DoCatchStmt& stmt) override;
    void visit(const DeferStmt& stmt) override;
    void visit(const GuardStmt& stmt) override;
    void visit(const GuardLetStmt& stmt) override;
    void visit(const SwitchStmt& stmt) override;
    
    // Advanced statement declarations
    void visit(const CustomOperatorStmt& stmt) override;
    void visit(const OperatorPrecedenceStmt& stmt) override;
    void visit(const ResultBuilderStmt& stmt) override;
    void visit(const ActorStmt& stmt) override;
    
    // Macro statements
    void visit(const MacroStmt& stmt) override;
    void visit(const ExternalMacroStmt& stmt) override;
    void visit(const FreestandingMacroStmt& stmt) override;
    void visit(const AttachedMacroStmt& stmt) override;

    void visit(const Binary& expr) override;
    void visit(const Ternary& expr) override;
    void visit(const Grouping& expr) override;
    void visit(const Literal& expr) override;
    void visit(const Unary& expr) override;
    void visit(const VarExpr& expr) override;
    void visit(const Assign& expr) override;
    void visit(const ArrayLiteral& expr) override;
    void visit(const DictionaryLiteral& expr) override;
    void visit(const TupleLiteral& expr) override;
    void visit(const IndexAccess& expr) override;
    void visit(const SubscriptAccess& expr) override;
    void visit(const Call& expr) override;
    void visit(const LabeledCall& expr) override;
    void visit(const Closure& expr) override;
    void visit(const EnumAccess& expr) override;
    void visit(const MemberAccess& expr) override;
    void visit(const StructInit& expr) override;
    void visit(const Super& expr) override;
    void visit(const StringInterpolation& expr) override;
    void visit(const OptionalChaining& expr) override;
    void visit(const Range& expr) override;
    void visit(const GenericTypeInstantiationExpr& expr) override;
    void visit(const TypeCheck& expr) override;
    void visit(const TypeCast& expr) override;
    
    // Helper methods for type casting
    Value performTypeCast(const Value& value, const std::string& targetType, bool isForced);
    std::string valueTypeToString(ValueType type);
    
    // Error handling expressions
    void visit(const TryExpr& expr) override;
    void visit(const ResultTypeExpr& expr) override;
    void visit(const ErrorLiteral& expr) override;
    
    // Advanced operator expressions
    void visit(const BitwiseExpr& expr) override;
    void visit(const OverflowExpr& expr) override;
    void visit(const CustomOperatorExpr& expr) override;
    void visit(const ResultBuilderExpr& expr) override;
    
    // Concurrency expressions
    void visit(const AwaitExpr& expr) override;
    void visit(const TaskExpr& expr) override;
    void visit(const TaskGroupExpr& expr) override;
    void visit(const AsyncSequenceExpr& expr) override;
    void visit(const AsyncLetExpr& expr) override;
    
    // Opaque and Boxed Protocol Types
    void visit(const OpaqueTypeExpr& expr) override;
    void visit(const BoxedProtocolTypeExpr& expr) override;
    
    // Macro expressions
    void visit(const MacroExpansionExpr& expr) override;
    void visit(const FreestandingMacroExpr& expr) override;
    void visit(const AttachedMacroExpr& expr) override;

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
    
    // Constructor function storage to keep them alive
    std::unordered_map<std::string, std::shared_ptr<FunctionStmt>> constructorFunctions;
    
    // Method function storage to keep them alive (with self parameter)
    std::unordered_map<std::string, std::shared_ptr<FunctionStmt>> methodFunctions;
    
    // Inheritance management
    std::unique_ptr<InheritanceManager> inheritanceManager;
    std::unique_ptr<SuperHandler> superHandler;
    
    // Subscript management
    std::unique_ptr<StaticSubscriptManager> staticSubscriptManager;
    
    // Error handling management
    std::stack<ErrorContext> errorContextStack;
    std::unique_ptr<ErrorPropagator> errorPropagator;
    std::stack<std::vector<std::unique_ptr<Stmt>>> deferStack;
    
    bool isTruthy(const Value& value);
    void printArray(const Array& arr);
    void printArrayInline(const Array& arr);
    void printDictionary(const Dictionary& dict);
    void printDictionaryInline(const Dictionary& dict);
    void printTuple(const Tuple& tuple);
    void printTupleInline(const Tuple& tuple);
    void printValue(const Value& val);
    void printValueInline(const Value& val);
    std::string valueToString(const Value& val);
    
    void registerStructProperties(const std::string& structName, const std::vector<StructMember>& members);
    void registerClassProperties(const std::string& className, const std::vector<StructMember>& members);
    
    // Error handling helpers
    void pushErrorContext(const ErrorContext& context);
    void popErrorContext();
    ErrorContext& getCurrentErrorContext();
    bool canCatchError(const ErrorValue& error) const;
    void executeDeferredStatements();
    void pushDeferredStatement(std::unique_ptr<Stmt> stmt);
    Result<Value> wrapInResult(const Value& value, bool isSuccess = true, const ErrorValue* error = nullptr);
    

    
public:
    // Enhanced member access with property support
    virtual Value getMemberValue(const Value& object, const std::string& memberName);
    void setMemberValue(Value& object, const std::string& memberName, const Value& value);
    
    // Property system helpers
    PropertyManager* getStructPropertyManager(const std::string& structName);
    PropertyManager* getClassPropertyManager(const std::string& className);
    
    // Inheritance system access
    InheritanceManager* getInheritanceManager() const { return inheritanceManager.get(); }

private:

    Value result;
};

} // namespace miniswift

#endif // MINISWIFT_INTERPRETER_H