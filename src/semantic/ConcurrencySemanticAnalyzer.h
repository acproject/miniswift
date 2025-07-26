#ifndef MINISWIFT_CONCURRENCY_SEMANTIC_ANALYZER_H
#define MINISWIFT_CONCURRENCY_SEMANTIC_ANALYZER_H

#include "ConcurrencyTypeSystem.h"
#include "../ast/ASTNodes.h"
#include "../ast/StmtNodes.h"
#include "../ast/ExprNodes.h"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace miniswift {

// 并发语义分析器
class ConcurrencySemanticAnalyzer {
public:
    ConcurrencySemanticAnalyzer() 
        : typeChecker_(std::make_unique<ConcurrencyTypeChecker>()),
          contextManager_(std::make_unique<AsyncContextManager>()) {}
    
    // 分析 AST 节点的并发语义
    bool analyzeNode(std::shared_ptr<ASTNode> node);
    
    // 分析函数声明
    bool analyzeFunctionDecl(std::shared_ptr<FunctionDeclNode> funcDecl);
    
    // 分析 Actor 声明
    bool analyzeActorDecl(std::shared_ptr<ActorStmt> actorStmt);
    
    // 分析异步函数调用
    bool analyzeAsyncCall(std::shared_ptr<CallExprNode> callExpr);
    
    // 分析 await 表达式
    bool analyzeAwaitExpr(std::shared_ptr<AwaitExprNode> awaitExpr);
    
    // 分析 Task 创建
    bool analyzeTaskCreation(std::shared_ptr<TaskExprNode> taskExpr);
    
    // 分析 TaskGroup 操作
    bool analyzeTaskGroupOperation(std::shared_ptr<TaskGroupExprNode> taskGroupExpr);
    
    // 分析 AsyncSequence 表达式
    bool analyzeAsyncSequenceExpr(std::shared_ptr<AsyncSequenceExprNode> asyncSeqExpr);
    
    // 分析 AsyncLet 表达式
    bool analyzeAsyncLetExpr(std::shared_ptr<AsyncLetExprNode> asyncLetExpr);
    
    // 分析 AsyncSequence 迭代
    bool analyzeAsyncSequenceIteration(std::shared_ptr<ForAwaitStmt> forAwaitStmt);
    
    // 分析 Actor 成员访问
    bool analyzeActorMemberAccess(std::shared_ptr<MemberAccessExprNode> memberAccess);
    
    // 分析变量声明的 Sendable 约束
    bool analyzeSendableConstraint(std::shared_ptr<VarDeclNode> varDecl);
    
    // 获取类型检查器
    ConcurrencyTypeChecker* getTypeChecker() const { return typeChecker_.get(); }
    
    // 获取上下文管理器
    AsyncContextManager* getContextManager() const { return contextManager_.get(); }
    
    // 获取分析结果
    std::vector<std::string> getDiagnostics() const;
    bool hasErrors() const;
    void clearDiagnostics();
    
    // 注册类型信息
    void registerActorType(const std::string& name, std::shared_ptr<ActorType> actorType);
    void registerAsyncFunction(const std::string& name, std::shared_ptr<AsyncFunctionType> funcType);
    
private:
    std::unique_ptr<ConcurrencyTypeChecker> typeChecker_;
    std::unique_ptr<AsyncContextManager> contextManager_;
    
    // 类型注册表
    std::unordered_map<std::string, std::shared_ptr<ActorType>> actorTypes_;
    std::unordered_map<std::string, std::shared_ptr<AsyncFunctionType>> asyncFunctions_;
    std::unordered_map<std::string, std::shared_ptr<Type>> variableTypes_;
    
    // 错误和警告收集
    std::vector<std::string> errors_;
    std::vector<std::string> warnings_;
    
    // 辅助方法
    std::shared_ptr<Type> resolveType(const std::string& typeName);
    std::shared_ptr<ActorType> resolveActorType(const std::string& actorName);
    bool isInAsyncContext() const;
    std::shared_ptr<ActorType> getCurrentActor() const;
    
    // 具体的分析方法
    bool analyzeStatement(std::shared_ptr<StmtNode> stmt);
    bool analyzeExpression(std::shared_ptr<ExprNode> expr);
    bool analyzeDeclaration(std::shared_ptr<DeclNode> decl);
    
    // 错误报告
    void reportError(const std::string& message);
    void reportWarning(const std::string& message);
    
    // 类型推断
    std::shared_ptr<Type> inferExpressionType(std::shared_ptr<ExprNode> expr);
    std::shared_ptr<Type> inferCallExpressionType(std::shared_ptr<CallExprNode> callExpr);
    std::shared_ptr<Type> inferMemberAccessType(std::shared_ptr<MemberAccessExprNode> memberAccess);
};

// 并发语义分析的辅助类
class ConcurrencyDiagnostic {
public:
    enum class Level {
        Error,
        Warning,
        Note
    };
    
    ConcurrencyDiagnostic(Level level, const std::string& message, 
                          const std::string& location = "")
        : level_(level), message_(message), location_(location) {}
    
    Level getLevel() const { return level_; }
    const std::string& getMessage() const { return message_; }
    const std::string& getLocation() const { return location_; }
    
    std::string toString() const {
        std::string levelStr;
        switch (level_) {
            case Level::Error: levelStr = "Error"; break;
            case Level::Warning: levelStr = "Warning"; break;
            case Level::Note: levelStr = "Note"; break;
        }
        
        if (!location_.empty()) {
            return levelStr + " at " + location_ + ": " + message_;
        }
        return levelStr + ": " + message_;
    }
    
private:
    Level level_;
    std::string message_;
    std::string location_;
};

// 并发特性检测器
class ConcurrencyFeatureDetector {
public:
    struct ConcurrencyFeatures {
        bool hasAsyncFunctions = false;
        bool hasActors = false;
        bool hasTaskGroups = false;
        bool hasAsyncSequences = false;
        bool hasAwaitExpressions = false;
        bool hasSendableTypes = false;
        bool hasAsyncLet = false;
        bool hasForAwaitLoops = false;
        
        std::vector<std::string> asyncFunctionNames;
        std::vector<std::string> actorNames;
        std::vector<std::string> sendableTypeNames;
    };
    
    ConcurrencyFeatures detectFeatures(std::shared_ptr<ASTNode> rootNode);
    
private:
    void visitNode(std::shared_ptr<ASTNode> node, ConcurrencyFeatures& features);
    void visitFunctionDecl(std::shared_ptr<FunctionDeclNode> funcDecl, ConcurrencyFeatures& features);
    void visitActorStmt(std::shared_ptr<ActorStmt> actorStmt, ConcurrencyFeatures& features);
    void visitForAwaitStmt(std::shared_ptr<ForAwaitStmt> forAwaitStmt, ConcurrencyFeatures& features);
    void visitExpression(std::shared_ptr<ExprNode> expr, ConcurrencyFeatures& features);
};

// 并发代码生成辅助器
class ConcurrencyCodeGenHelper {
public:
    // 生成异步函数的 LLVM IR 辅助信息
    struct AsyncFunctionInfo {
        std::string functionName;
        std::vector<std::string> parameterTypes;
        std::string returnType;
        bool needsAsyncContext;
        bool needsActorIsolation;
    };
    
    // 生成 Actor 的 LLVM IR 辅助信息
    struct ActorInfo {
        std::string actorName;
        std::vector<std::string> isolatedMembers;
        std::vector<std::string> nonisolatedMembers;
        bool isGlobalActor;
    };
    
    // 生成 TaskGroup 的 LLVM IR 辅助信息
    struct TaskGroupInfo {
        std::string groupType;
        std::string childTaskReturnType;
        std::vector<std::string> taskFunctions;
    };
    
    AsyncFunctionInfo generateAsyncFunctionInfo(std::shared_ptr<AsyncFunctionType> funcType);
    ActorInfo generateActorInfo(std::shared_ptr<ActorType> actorType);
    TaskGroupInfo generateTaskGroupInfo(std::shared_ptr<TaskGroupType> taskGroupType);
    
    // 生成运行时调用的函数名
    std::string getAsyncRuntimeFunction(const std::string& operation);
    std::string getActorRuntimeFunction(const std::string& operation);
    std::string getTaskGroupRuntimeFunction(const std::string& operation);
};

} // namespace miniswift

#endif // MINISWIFT_CONCURRENCY_SEMANTIC_ANALYZER_H