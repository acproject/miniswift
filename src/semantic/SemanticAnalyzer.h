#ifndef MINISWIFT_SEMANTIC_ANALYZER_H
#define MINISWIFT_SEMANTIC_ANALYZER_H

#include "../parser/AST.h"
#include "SymbolTable.h"
#include "TypeSystem.h"
#include "TypedAST.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>

namespace miniswift {

// 语义分析错误类型
struct SemanticError {
    std::string message;
    int line;
    int column;
    std::string file;
    
    SemanticError(const std::string& msg, int l = -1, int c = -1, const std::string& f = "")
        : message(msg), line(l), column(c), file(f) {}
};

// 并发上下文类型
enum class ConcurrencyContext {
    NONE,           // 非并发上下文
    ASYNC_FUNCTION, // 异步函数内
    ACTOR_METHOD,   // Actor方法内
    TASK_CLOSURE,   // Task闭包内
    TASK_GROUP,     // TaskGroup内
    ASYNC_SEQUENCE  // AsyncSequence迭代内
};

struct ConcurrencyError {
    std::string message;
    int line;
    int column;
    std::string file;
    ConcurrencyContext context;
    std::string actorName;
    
    ConcurrencyError(const std::string& msg = "", int l = -1, int c = -1, const std::string& f = "")
        : message(msg), line(l), column(c), file(f), context(ConcurrencyContext::NONE), actorName("") {}
};

// 并发安全级别定义在 TypeSystem.h 中

// 并发变量信息
struct ConcurrentVariable {
    std::string name;
    std::shared_ptr<Type> type;
    ConcurrencySafetyLevel safetyLevel;
    bool isIsolated;
    std::string actorName; // 如果是actor隔离的变量
    bool isAsync;
    
    // 默认构造函数
    ConcurrentVariable() : name(""), type(nullptr), safetyLevel(ConcurrencySafetyLevel::UNSAFE), isIsolated(false), isAsync(false) {}
    
    ConcurrentVariable(const std::string& n, std::shared_ptr<Type> t, 
                      ConcurrencySafetyLevel level = ConcurrencySafetyLevel::UNSAFE)
        : name(n), type(t), safetyLevel(level), isIsolated(false), isAsync(false) {}
};

// 并发函数信息
struct ConcurrentFunction {
    std::string name;
    std::shared_ptr<FunctionType> type;
    bool isAsync;
    bool isActorMethod;
    std::string actorName;
    std::vector<std::string> asyncParameters;
    
    // 默认构造函数
    ConcurrentFunction() : name(""), type(nullptr), isAsync(false), isActorMethod(false) {}
    
    ConcurrentFunction(const std::string& n, std::shared_ptr<FunctionType> t, bool async = false)
        : name(n), type(t), isAsync(async), isActorMethod(false) {}
};

// Actor信息
struct ActorInfo {
    std::string name;
    std::unordered_map<std::string, ConcurrentVariable> properties;
    std::unordered_map<std::string, ConcurrentFunction> methods;
    bool isGlobalActor;
    
    // 默认构造函数
    ActorInfo() : name(""), isGlobalActor(false) {}
    
    ActorInfo(const std::string& n, bool global = false)
        : name(n), isGlobalActor(global) {}
};

// 语义分析结果
struct SemanticAnalysisResult {
    std::unique_ptr<TypedProgram> typedAST;
    std::vector<SemanticError> errors;
    std::vector<ConcurrencyError> concurrencyErrors;
    std::vector<std::string> warnings;
    bool hasErrors() const { return !errors.empty() || !concurrencyErrors.empty(); }
};

// 语义分析器主类
class SemanticAnalyzer : public ExprVisitor, public StmtVisitor {
public:
    SemanticAnalyzer();
    virtual ~SemanticAnalyzer() = default;
    
    // 主要分析接口
    SemanticAnalysisResult analyze(const std::vector<std::unique_ptr<Stmt>>& statements);
    
    // 并发分析接口
    std::vector<ConcurrencyError> analyzeConcurrency(const std::vector<std::unique_ptr<Stmt>>& statements);
    
    // 并发特定的分析方法
    void analyzeAsyncFunction(const FunctionStmt& func);
    void analyzeActor(const ActorStmt& actor);
    void analyzeTaskCreation(const TaskExpr& task);
    void analyzeTaskGroup(const TaskGroupExpr& taskGroup);
    void analyzeAwaitExpression(const AwaitExpr& await);
    void analyzeAsyncSequence(const AsyncSequenceExpr& asyncSeq);
    void analyzeAsyncLet(const AsyncLetExpr& asyncLet);
    
    // 并发安全检查
    void checkSendableCompliance(const std::shared_ptr<Type>& type);
    void checkActorIsolation(const Expr& expr);
    void checkDataRaceConditions(const Expr& expr);
    void checkDeadlockPotential(const Stmt& stmt);
    
    // AST访问者模式实现 - 语句
    void visit(const ExprStmt& stmt) override;
    void visit(const PrintStmt& stmt) override;
    void visit(const VarStmt& stmt) override;
    void visit(const BlockStmt& stmt) override;
    void visit(const IfStmt& stmt) override;
    void visit(const IfLetStmt& stmt) override;
    void visit(const WhileStmt& stmt) override;
    void visit(const ForStmt& stmt) override;
    void visit(const ForInStmt& stmt) override;
    void visit(const FunctionStmt& stmt) override;
    void visit(const ReturnStmt& stmt) override;
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
    void visit(const ThrowStmt& stmt) override;
    void visit(const DoCatchStmt& stmt) override;
    void visit(const DeferStmt& stmt) override;
    void visit(const GuardStmt& stmt) override;
    void visit(const GuardLetStmt& stmt) override;
    void visit(const SwitchStmt& stmt) override;
    void visit(const CustomOperatorStmt& stmt) override;
    void visit(const OperatorPrecedenceStmt& stmt) override;
    void visit(const ResultBuilderStmt& stmt) override;
    void visit(const ActorStmt& stmt) override;
    void visit(const MacroStmt& stmt) override;
    void visit(const ExternalMacroStmt& stmt) override;
    void visit(const FreestandingMacroStmt& stmt) override;
    void visit(const AttachedMacroStmt& stmt) override;
    void visit(const ForAwaitStmt& stmt) override;
    void visit(const ImportStmt& stmt) override;
    
    // AST访问者模式实现 - 表达式
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
    void visit(const TryExpr& expr) override;
    void visit(const ResultTypeExpr& expr) override;
    void visit(const ErrorLiteral& expr) override;
    void visit(const CustomOperatorExpr& expr) override;
    void visit(const BitwiseExpr& expr) override;
    void visit(const OverflowExpr& expr) override;
    void visit(const ResultBuilderExpr& expr) override;
    void visit(const AwaitExpr& expr) override;
    void visit(const TaskExpr& expr) override;
    void visit(const OpaqueTypeExpr& expr) override;
    void visit(const BoxedProtocolTypeExpr& expr) override;
    void visit(const MacroExpansionExpr& expr) override;
    void visit(const FreestandingMacroExpr& expr) override;
    void visit(const AttachedMacroExpr& expr) override;
    void visit(const TaskGroupExpr& expr) override;
    void visit(const AsyncSequenceExpr& expr) override;
    void visit(const AsyncLetExpr& expr) override;
    
private:
    // 核心组件
    std::unique_ptr<SymbolTable> symbolTable;
    std::unique_ptr<TypeSystem> typeSystem;
    std::unique_ptr<TypedProgram> currentTypedProgram;
    
    // 分析状态
    std::vector<SemanticError> errors;
    std::vector<ConcurrencyError> concurrencyErrors;
    std::vector<std::string> warnings;
    std::shared_ptr<Type> currentExpressionType;
    std::shared_ptr<Type> currentFunctionReturnType;
    bool inFunctionContext;
    bool inLoopContext;
    bool inClassContext;
    std::string currentClassName;
    
    // 并发上下文管理
    std::vector<ConcurrencyContext> contextStack;
    ConcurrencyContext currentContext;
    std::string currentActorName;
    bool inAsyncContext;
    bool inTaskGroup;
    
    // 并发实体管理
    std::unordered_map<std::string, ActorInfo> actors;
    std::unordered_map<std::string, ConcurrentFunction> asyncFunctions;
    std::unordered_map<std::string, ConcurrentVariable> asyncVariables;
    std::unordered_set<std::string> sendableTypes;
    
    // 辅助方法
    void reportError(const std::string& message, int line = -1, int column = -1);
    void reportWarning(const std::string& message);
    void reportConcurrencyError(const std::string& message, int line = -1, int column = -1);
    
    // 类型检查辅助方法
    std::shared_ptr<Type> analyzeExpression(const Expr& expr);
    void analyzeStatement(const Stmt& stmt);
    bool isTypeCompatible(const std::shared_ptr<Type>& from, const std::shared_ptr<Type>& to);
    std::shared_ptr<Type> getCommonType(const std::shared_ptr<Type>& type1, const std::shared_ptr<Type>& type2);
    
    // 符号表管理
    void enterScope();
    void exitScope();
    void declareVariable(const std::string& name, const std::shared_ptr<Type>& type, bool isConst = false);
    void declareFunction(const std::string& name, const std::shared_ptr<FunctionType>& type);
    void declareType(const std::string& name, const std::shared_ptr<Type>& type);
    
    // 类型推断
    std::shared_ptr<Type> inferType(const Expr& expr);
    std::shared_ptr<Type> inferLiteralType(const Literal& literal);
    std::shared_ptr<Type> inferBinaryType(const Binary& binary);
    std::shared_ptr<Type> inferCallType(const Call& call);
    
    // 控制流分析
    void analyzeControlFlow(const Stmt& stmt);
    bool isReachable(const Stmt& stmt);
    
    // 内存安全检查
    void checkMemorySafety(const Expr& expr);
    void checkNullPointerAccess(const Expr& expr);
    void checkArrayBounds(const IndexAccess& access);
    
    // 并发安全检查
    void checkConcurrencySafety(const Stmt& stmt);
    
    // 并发上下文管理
    void enterConcurrencyContext(ConcurrencyContext context, const std::string& actorName = "");
    void exitConcurrencyContext();
    bool isInAsyncContext() const;
    bool isInActorContext() const;
    
    // 并发类型检查
    bool isSendableType(const std::shared_ptr<Type>& type);
    bool isActorIsolatedType(const std::shared_ptr<Type>& type);
    ConcurrencySafetyLevel getSafetyLevel(const std::shared_ptr<Type>& type);
    
    // 并发操作验证
    void validateAsyncCall(const Call& call);
    void validateActorAccess(const MemberAccess& access);
    void validateSendableConstraint(const Expr& expr, const std::shared_ptr<Type>& expectedType);
    void validateTaskGroupOperation(const TaskGroupExpr& taskGroup);
    void validateAsyncSequenceIteration(const AsyncSequenceExpr& asyncSeq);
    
    // 数据竞争检测
    void detectDataRace(const Expr& expr);
    void checkConcurrentAccess(const std::string& variableName);
    void checkActorBoundaryViolation(const Expr& expr);
    
    // 死锁检测
    void detectDeadlock(const Stmt& stmt);
    void checkCircularActorDependency();
    
    // 并发辅助方法
    std::string getExpressionActorContext(const Expr& expr);
    bool requiresAsyncContext(const Expr& expr);
    bool canCrossActorBoundary(const std::shared_ptr<Type>& type);
    
    // 泛型支持
    // void analyzeGenericConstraints(const std::vector<GenericParameter>& parameters);  // 暂时注释，等待GenericParameter定义
    std::shared_ptr<Type> instantiateGenericType(const std::shared_ptr<Type>& genericType, 
                                                const std::unordered_map<std::string, std::shared_ptr<Type>>& substitutions);
};

} // namespace miniswift

#endif // MINISWIFT_SEMANTIC_ANALYZER_H