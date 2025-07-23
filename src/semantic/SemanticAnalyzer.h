#ifndef MINISWIFT_SEMANTIC_ANALYZER_H
#define MINISWIFT_SEMANTIC_ANALYZER_H

#include "../parser/AST.h"
#include "SymbolTable.h"
#include "TypeSystem.h"
#include "TypedAST.h"
#include <memory>
#include <vector>
#include <unordered_map>
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

// 语义分析结果
struct SemanticAnalysisResult {
    std::unique_ptr<TypedProgram> typedAST;
    std::vector<SemanticError> errors;
    std::vector<std::string> warnings;
    bool hasErrors() const { return !errors.empty(); }
};

// 语义分析器主类
class SemanticAnalyzer : public ExprVisitor, public StmtVisitor {
public:
    SemanticAnalyzer();
    virtual ~SemanticAnalyzer() = default;
    
    // 主要分析接口
    SemanticAnalysisResult analyze(const std::vector<std::unique_ptr<Stmt>>& statements);
    
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
    
private:
    // 核心组件
    std::unique_ptr<SymbolTable> symbolTable;
    std::unique_ptr<TypeSystem> typeSystem;
    std::unique_ptr<TypedProgram> currentTypedProgram;
    
    // 分析状态
    std::vector<SemanticError> errors;
    std::vector<std::string> warnings;
    std::shared_ptr<Type> currentExpressionType;
    std::shared_ptr<Type> currentFunctionReturnType;
    bool inFunctionContext;
    bool inLoopContext;
    bool inClassContext;
    std::string currentClassName;
    
    // 辅助方法
    void reportError(const std::string& message, int line = -1, int column = -1);
    void reportWarning(const std::string& message);
    
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
    void checkActorIsolation(const Expr& expr);
    
    // 泛型支持
    // void analyzeGenericConstraints(const std::vector<GenericParameter>& parameters);  // 暂时注释，等待GenericParameter定义
    std::shared_ptr<Type> instantiateGenericType(const std::shared_ptr<Type>& genericType, 
                                                const std::unordered_map<std::string, std::shared_ptr<Type>>& substitutions);
};

} // namespace miniswift

#endif // MINISWIFT_SEMANTIC_ANALYZER_H