#include "ConcurrencySemanticAnalyzer.h"
#include <iostream>
#include <algorithm>

namespace miniswift {

// ConcurrencySemanticAnalyzer 实现

bool ConcurrencySemanticAnalyzer::analyzeNode(std::shared_ptr<ASTNode> node) {
    if (!node) {
        return false;
    }
    
    // 根据节点类型进行分析
    if (auto stmt = std::dynamic_pointer_cast<StmtNode>(node)) {
        return analyzeStatement(stmt);
    } else if (auto expr = std::dynamic_pointer_cast<ExprNode>(node)) {
        return analyzeExpression(expr);
    } else if (auto decl = std::dynamic_pointer_cast<DeclNode>(node)) {
        return analyzeDeclaration(decl);
    }
    
    return true;
}

bool ConcurrencySemanticAnalyzer::analyzeFunctionDecl(std::shared_ptr<FunctionDeclNode> funcDecl) {
    if (!funcDecl) {
        return false;
    }
    
    // 检查函数是否是异步的
    bool isAsync = funcDecl->isAsync();
    
    if (isAsync) {
        // 进入异步上下文
        contextManager_->enterAsyncContext();
        
        // 创建异步函数类型
        std::vector<std::shared_ptr<Type>> paramTypes;
        for (const auto& param : funcDecl->getParameters()) {
            auto paramType = resolveType(param.type);
            if (paramType) {
                paramTypes.push_back(paramType);
            }
        }
        
        auto returnType = resolveType(funcDecl->getReturnType());
        if (!returnType) {
            reportError("Cannot resolve return type for async function: " + funcDecl->getName());
            return false;
        }
        
        auto asyncFuncType = std::make_shared<AsyncFunctionType>(
            paramTypes, returnType, funcDecl->isThrows());
        
        // 注册异步函数
        registerAsyncFunction(funcDecl->getName(), asyncFuncType);
        
        // 分析函数体
        bool result = true;
        if (funcDecl->getBody()) {
            result = analyzeStatement(funcDecl->getBody());
        }
        
        // 退出异步上下文
        contextManager_->exitAsyncContext();
        
        return result;
    } else {
        // 普通函数，直接分析函数体
        if (funcDecl->getBody()) {
            return analyzeStatement(funcDecl->getBody());
        }
    }
    
    return true;
}

bool ConcurrencySemanticAnalyzer::analyzeActorDecl(std::shared_ptr<ActorStmt> actorStmt) {
    if (!actorStmt) {
        return false;
    }
    
    // 创建 Actor 类型
    auto actorType = std::make_shared<ActorType>(actorStmt->getName());
    actorType->setGlobalActor(actorStmt->isGlobalActor());
    
    // 分析 Actor 成员
    for (const auto& member : actorStmt->getMembers()) {
        if (auto property = std::dynamic_pointer_cast<PropertyStmt>(member)) {
            // 属性默认是隔离的，除非标记为 nonisolated
            bool isIsolated = !property->isNonisolated();
            actorType->setMemberIsolation(property->getName(), isIsolated);
            
            // 添加属性类型到 Actor
            auto propType = resolveType(property->getType());
            if (propType) {
                actorType->addMember(property->getName(), propType);
            }
        } else if (auto method = std::dynamic_pointer_cast<FunctionDeclNode>(member)) {
            // 方法默认是隔离的，除非标记为 nonisolated
            bool isIsolated = !method->isNonisolated();
            actorType->setMemberIsolation(method->getName(), isIsolated);
            
            // 进入 Actor 上下文分析方法
            contextManager_->enterAsyncContext(actorType);
            
            bool methodResult = analyzeFunctionDecl(method);
            
            contextManager_->exitAsyncContext();
            
            if (!methodResult) {
                reportError("Error analyzing method " + method->getName() + " in actor " + actorStmt->getName());
                return false;
            }
        }
    }
    
    // 注册 Actor 类型
    registerActorType(actorStmt->getName(), actorType);
    
    return true;
}

bool ConcurrencySemanticAnalyzer::analyzeAsyncCall(std::shared_ptr<CallExprNode> callExpr) {
    if (!callExpr) {
        return false;
    }
    
    // 解析被调用的函数类型
    auto funcType = inferCallExpressionType(callExpr);
    if (!funcType) {
        reportError("Cannot resolve function type for call expression");
        return false;
    }
    
    // 检查是否是异步函数调用
    if (auto asyncFuncType = std::dynamic_pointer_cast<AsyncFunctionType>(funcType)) {
        // 验证异步调用
        std::vector<std::shared_ptr<Type>> argTypes;
        for (const auto& arg : callExpr->getArguments()) {
            auto argType = inferExpressionType(arg);
            if (argType) {
                argTypes.push_back(argType);
            }
        }
        
        bool isInAsync = contextManager_->isInAsyncContext();
        return typeChecker_->validateAsyncCall(asyncFuncType, argTypes, isInAsync);
    }
    
    return true;
}

bool ConcurrencySemanticAnalyzer::analyzeAwaitExpr(std::shared_ptr<AwaitExprNode> awaitExpr) {
    if (!awaitExpr) {
        return false;
    }
    
    // await 表达式只能在异步上下文中使用
    if (!contextManager_->isInAsyncContext()) {
        reportError("'await' can only be used in async context");
        return false;
    }
    
    // 分析被 await 的表达式
    auto expr = awaitExpr->getExpression();
    if (!expr) {
        reportError("Invalid await expression");
        return false;
    }
    
    // 检查表达式类型是否是异步的
    auto exprType = inferExpressionType(expr);
    if (!exprType) {
        reportError("Cannot resolve type for await expression");
        return false;
    }
    
    // 如果是函数调用，检查是否是异步函数
    if (auto callExpr = std::dynamic_pointer_cast<CallExprNode>(expr)) {
        return analyzeAsyncCall(callExpr);
    }
    
    return analyzeExpression(expr);
}

bool ConcurrencySemanticAnalyzer::analyzeTaskCreation(std::shared_ptr<TaskExprNode> taskExpr) {
    if (!taskExpr) {
        return false;
    }
    
    // Task 创建需要在异步上下文中
    if (!contextManager_->isInAsyncContext()) {
        reportError("Task creation requires async context");
        return false;
    }
    
    // 分析 Task 的闭包
    auto closure = taskExpr->getClosure();
    if (!closure) {
        reportError("Task requires a closure");
        return false;
    }
    
    // 进入新的异步上下文分析闭包
    contextManager_->enterAsyncContext();
    
    bool result = analyzeExpression(closure);
    
    contextManager_->exitAsyncContext();
    
    return result;
}

bool ConcurrencySemanticAnalyzer::analyzeTaskGroupOperation(std::shared_ptr<TaskGroupExprNode> taskGroupExpr) {
    if (!taskGroupExpr) {
        return false;
    }
    
    // TaskGroup 操作需要在异步上下文中
    if (!contextManager_->isInAsyncContext()) {
        reportError("TaskGroup operations require async context");
        return false;
    }
    
    // 进入 TaskGroup 上下文
    std::string groupId = "taskgroup_" + std::to_string(reinterpret_cast<uintptr_t>(taskGroupExpr.get()));
    contextManager_->enterTaskGroup(groupId);
    
    // 分析 TaskGroup 的操作
    bool result = true;
    
    if (taskGroupExpr->getOperation() == "addTask") {
        // 分析添加的任务
        auto taskClosure = taskGroupExpr->getTaskClosure();
        if (taskClosure) {
            // 进入新的异步上下文分析任务闭包
            contextManager_->enterAsyncContext();
            
            result = analyzeExpression(taskClosure);
            
            contextManager_->exitAsyncContext();
            
            // 添加任务到组
            std::string taskId = "task_" + std::to_string(reinterpret_cast<uintptr_t>(taskClosure.get()));
            contextManager_->addTaskToGroup(taskId);
        }
    } else if (taskGroupExpr->getOperation() == "waitForAll") {
        // 等待所有任务完成
        // 这里可以添加额外的验证逻辑
    }
    
    // 退出 TaskGroup 上下文
    contextManager_->exitTaskGroup();
    
    return result;
}

bool ConcurrencySemanticAnalyzer::analyzeAsyncSequenceExpr(std::shared_ptr<AsyncSequenceExprNode> asyncSeqExpr) {
    if (!asyncSeqExpr) {
        return false;
    }
    
    // AsyncSequence 表达式需要在异步上下文中
    if (!contextManager_->isInAsyncContext()) {
        reportError("AsyncSequence operations require async context");
        return false;
    }
    
    // 分析序列的元素类型
    auto elementType = asyncSeqExpr->getElementType();
    if (!elementType.empty()) {
        auto resolvedElementType = resolveType(elementType);
        if (!resolvedElementType) {
            reportError("Cannot resolve element type for AsyncSequence: " + elementType);
            return false;
        }
        
        // 验证元素类型是否符合 Sendable 约束
        bool isValid = typeChecker_->validateSendableConstraint(resolvedElementType, "AsyncSequence element");
        if (!isValid) {
            return false;
        }
    }
    
    // 分析序列的生成器闭包
    auto generator = asyncSeqExpr->getGenerator();
    if (generator) {
        // 进入新的异步上下文分析生成器
        contextManager_->enterAsyncContext();
        
        bool result = analyzeExpression(generator);
        
        contextManager_->exitAsyncContext();
        
        return result;
    }
    
    return true;
}

bool ConcurrencySemanticAnalyzer::analyzeAsyncLetExpr(std::shared_ptr<AsyncLetExprNode> asyncLetExpr) {
    if (!asyncLetExpr) {
        return false;
    }
    
    // async let 需要在异步上下文中
    if (!contextManager_->isInAsyncContext()) {
        reportError("'async let' requires async context");
        return false;
    }
    
    // 分析绑定的表达式
    auto bindingExpr = asyncLetExpr->getExpression();
    if (!bindingExpr) {
        reportError("Invalid expression in async let binding");
        return false;
    }
    
    // 检查表达式类型
    auto exprType = inferExpressionType(bindingExpr);
    if (!exprType) {
        reportError("Cannot resolve type for async let expression");
        return false;
    }
    
    // 验证表达式是否是异步的
    if (auto asyncFuncType = std::dynamic_pointer_cast<AsyncFunctionType>(exprType)) {
        // 如果是异步函数调用，验证调用
        if (auto callExpr = std::dynamic_pointer_cast<CallExprNode>(bindingExpr)) {
            bool isValid = analyzeAsyncCall(callExpr);
            if (!isValid) {
                return false;
            }
        }
    }
    
    // 验证绑定变量的类型是否符合 Sendable 约束
    auto variableName = asyncLetExpr->getVariableName();
    if (!variableName.empty()) {
        bool isValid = typeChecker_->validateSendableConstraint(exprType, "async let variable");
        if (!isValid) {
            return false;
        }
        
        // 注册变量类型
        variableTypes_[variableName] = exprType;
    }
    
    return analyzeExpression(bindingExpr);
}

bool ConcurrencySemanticAnalyzer::analyzeAsyncSequenceIteration(std::shared_ptr<ForAwaitStmt> forAwaitStmt) {
    if (!forAwaitStmt) {
        return false;
    }
    
    // for-await-in 循环需要在异步上下文中
    if (!contextManager_->isInAsyncContext()) {
        reportError("'for await' loop requires async context");
        return false;
    }
    
    // 分析被迭代的序列
    auto sequence = forAwaitStmt->getSequence();
    if (!sequence) {
        reportError("Invalid sequence in for-await loop");
        return false;
    }
    
    auto sequenceType = inferExpressionType(sequence);
    if (!sequenceType) {
        reportError("Cannot resolve sequence type in for-await loop");
        return false;
    }
    
    // 检查是否是 AsyncSequence 类型
    auto asyncSeqType = std::dynamic_pointer_cast<AsyncSequenceType>(sequenceType);
    if (!asyncSeqType) {
        reportError("Sequence in for-await loop must conform to AsyncSequence");
        return false;
    }
    
    // 验证 AsyncSequence 迭代
    bool isValid = typeChecker_->validateAsyncSequenceIteration(asyncSeqType, true);
    if (!isValid) {
        return false;
    }
    
    // 分析循环体
    auto body = forAwaitStmt->getBody();
    if (body) {
        return analyzeStatement(body);
    }
    
    return true;
}

bool ConcurrencySemanticAnalyzer::analyzeActorMemberAccess(std::shared_ptr<MemberAccessExprNode> memberAccess) {
    if (!memberAccess) {
        return false;
    }
    
    // 获取基础表达式的类型
    auto baseExpr = memberAccess->getBase();
    if (!baseExpr) {
        reportError("Invalid member access expression");
        return false;
    }
    
    auto baseType = inferExpressionType(baseExpr);
    if (!baseType) {
        reportError("Cannot resolve base type for member access");
        return false;
    }
    
    // 检查是否是 Actor 类型
    auto actorType = std::dynamic_pointer_cast<ActorType>(baseType);
    if (!actorType) {
        // 不是 Actor 类型，无需特殊检查
        return true;
    }
    
    // 检查 Actor 成员访问
    std::string memberName = memberAccess->getMemberName();
    auto currentActor = contextManager_->getCurrentActor();
    bool isInAsync = contextManager_->isInAsyncContext();
    
    return typeChecker_->validateActorAccess(actorType, memberName, currentActor, isInAsync);
}

bool ConcurrencySemanticAnalyzer::analyzeSendableConstraint(std::shared_ptr<VarDeclNode> varDecl) {
    if (!varDecl) {
        return false;
    }
    
    // 解析变量类型
    auto varType = resolveType(varDecl->getType());
    if (!varType) {
        reportError("Cannot resolve type for variable: " + varDecl->getName());
        return false;
    }
    
    // 检查是否需要 Sendable 约束
    bool needsSendable = false;
    
    // 如果变量在跨 Actor 上下文中使用，需要 Sendable
    if (contextManager_->isInAsyncContext()) {
        // 简化检查：在异步上下文中的变量可能需要 Sendable
        needsSendable = true;
    }
    
    // 如果变量被标记为需要 Sendable
    if (varDecl->requiresSendable()) {
        needsSendable = true;
    }
    
    if (needsSendable) {
        return typeChecker_->validateSendableConstraint(varType, "variable declaration");
    }
    
    // 注册变量类型
    variableTypes_[varDecl->getName()] = varType;
    
    return true;
}

std::vector<std::string> ConcurrencySemanticAnalyzer::getDiagnostics() const {
    std::vector<std::string> allDiagnostics;
    
    // 添加错误
    for (const auto& error : errors_) {
        allDiagnostics.push_back("Error: " + error);
    }
    
    // 添加警告
    for (const auto& warning : warnings_) {
        allDiagnostics.push_back("Warning: " + warning);
    }
    
    // 添加类型检查器的诊断
    auto typeCheckerDiagnostics = typeChecker_->getDiagnostics();
    allDiagnostics.insert(allDiagnostics.end(), 
                         typeCheckerDiagnostics.begin(), 
                         typeCheckerDiagnostics.end());
    
    return allDiagnostics;
}

bool ConcurrencySemanticAnalyzer::hasErrors() const {
    return !errors_.empty();
}

void ConcurrencySemanticAnalyzer::clearDiagnostics() {
    errors_.clear();
    warnings_.clear();
    typeChecker_->clearDiagnostics();
}

void ConcurrencySemanticAnalyzer::registerActorType(const std::string& name, std::shared_ptr<ActorType> actorType) {
    actorTypes_[name] = actorType;
}

void ConcurrencySemanticAnalyzer::registerAsyncFunction(const std::string& name, std::shared_ptr<AsyncFunctionType> funcType) {
    asyncFunctions_[name] = funcType;
}

// 私有辅助方法实现

std::shared_ptr<Type> ConcurrencySemanticAnalyzer::resolveType(const std::string& typeName) {
    // 简化的类型解析实现
    // 在实际实现中，这应该与主要的类型系统集成
    
    // 检查是否是 Actor 类型
    auto actorIt = actorTypes_.find(typeName);
    if (actorIt != actorTypes_.end()) {
        return actorIt->second;
    }
    
    // 检查基础类型
    if (typeName == "Int") {
        return std::make_shared<PrimitiveType>(TypeKind::Int, "Int");
    } else if (typeName == "String") {
        return std::make_shared<PrimitiveType>(TypeKind::String, "String");
    } else if (typeName == "Bool") {
        return std::make_shared<PrimitiveType>(TypeKind::Bool, "Bool");
    } else if (typeName == "Void") {
        return std::make_shared<PrimitiveType>(TypeKind::Void, "Void");
    }
    
    // 如果找不到类型，返回 nullptr
    return nullptr;
}

std::shared_ptr<ActorType> ConcurrencySemanticAnalyzer::resolveActorType(const std::string& actorName) {
    auto it = actorTypes_.find(actorName);
    return (it != actorTypes_.end()) ? it->second : nullptr;
}

bool ConcurrencySemanticAnalyzer::isInAsyncContext() const {
    return contextManager_->isInAsyncContext();
}

std::shared_ptr<ActorType> ConcurrencySemanticAnalyzer::getCurrentActor() const {
    return contextManager_->getCurrentActor();
}

bool ConcurrencySemanticAnalyzer::analyzeStatement(std::shared_ptr<StmtNode> stmt) {
    if (!stmt) {
        return true;
    }
    
    // 根据语句类型进行分析
    if (auto actorStmt = std::dynamic_pointer_cast<ActorStmt>(stmt)) {
        return analyzeActorDecl(actorStmt);
    } else if (auto forAwaitStmt = std::dynamic_pointer_cast<ForAwaitStmt>(stmt)) {
        return analyzeAsyncSequenceIteration(forAwaitStmt);
    }
    // 可以添加更多语句类型的处理
    
    return true;
}

bool ConcurrencySemanticAnalyzer::analyzeExpression(std::shared_ptr<ExprNode> expr) {
    if (!expr) {
        return true;
    }
    
    // 根据表达式类型进行分析
    if (auto callExpr = std::dynamic_pointer_cast<CallExprNode>(expr)) {
        return analyzeAsyncCall(callExpr);
    } else if (auto awaitExpr = std::dynamic_pointer_cast<AwaitExprNode>(expr)) {
        return analyzeAwaitExpr(awaitExpr);
    } else if (auto taskExpr = std::dynamic_pointer_cast<TaskExprNode>(expr)) {
        return analyzeTaskCreation(taskExpr);
    } else if (auto taskGroupExpr = std::dynamic_pointer_cast<TaskGroupExprNode>(expr)) {
        return analyzeTaskGroupOperation(taskGroupExpr);
    } else if (auto asyncSeqExpr = std::dynamic_pointer_cast<AsyncSequenceExprNode>(expr)) {
        return analyzeAsyncSequenceExpr(asyncSeqExpr);
    } else if (auto asyncLetExpr = std::dynamic_pointer_cast<AsyncLetExprNode>(expr)) {
        return analyzeAsyncLetExpr(asyncLetExpr);
    } else if (auto memberAccess = std::dynamic_pointer_cast<MemberAccessExprNode>(expr)) {
        return analyzeActorMemberAccess(memberAccess);
    }
    
    return true;
}

bool ConcurrencySemanticAnalyzer::analyzeDeclaration(std::shared_ptr<DeclNode> decl) {
    if (!decl) {
        return true;
    }
    
    // 根据声明类型进行分析
    if (auto funcDecl = std::dynamic_pointer_cast<FunctionDeclNode>(decl)) {
        return analyzeFunctionDecl(funcDecl);
    } else if (auto varDecl = std::dynamic_pointer_cast<VarDeclNode>(decl)) {
        return analyzeSendableConstraint(varDecl);
    }
    
    return true;
}

void ConcurrencySemanticAnalyzer::reportError(const std::string& message) {
    errors_.push_back(message);
}

void ConcurrencySemanticAnalyzer::reportWarning(const std::string& message) {
    warnings_.push_back(message);
}

std::shared_ptr<Type> ConcurrencySemanticAnalyzer::inferExpressionType(std::shared_ptr<ExprNode> expr) {
    if (!expr) {
        return nullptr;
    }
    
    // 简化的类型推断实现
    if (auto callExpr = std::dynamic_pointer_cast<CallExprNode>(expr)) {
        return inferCallExpressionType(callExpr);
    } else if (auto memberAccess = std::dynamic_pointer_cast<MemberAccessExprNode>(expr)) {
        return inferMemberAccessType(memberAccess);
    }
    
    // 默认返回 nullptr，表示无法推断类型
    return nullptr;
}

std::shared_ptr<Type> ConcurrencySemanticAnalyzer::inferCallExpressionType(std::shared_ptr<CallExprNode> callExpr) {
    if (!callExpr) {
        return nullptr;
    }
    
    // 简化实现：根据函数名查找类型
    std::string funcName = callExpr->getFunctionName();
    
    auto asyncFuncIt = asyncFunctions_.find(funcName);
    if (asyncFuncIt != asyncFunctions_.end()) {
        return asyncFuncIt->second;
    }
    
    // 如果找不到，返回 nullptr
    return nullptr;
}

std::shared_ptr<Type> ConcurrencySemanticAnalyzer::inferMemberAccessType(std::shared_ptr<MemberAccessExprNode> memberAccess) {
    if (!memberAccess) {
        return nullptr;
    }
    
    // 获取基础类型
    auto baseType = inferExpressionType(memberAccess->getBase());
    if (!baseType) {
        return nullptr;
    }
    
    // 如果是用户定义类型，查找成员类型
    if (auto userType = std::dynamic_pointer_cast<UserDefinedType>(baseType)) {
        auto members = userType->getMembers();
        auto memberIt = members.find(memberAccess->getMemberName());
        if (memberIt != members.end()) {
            return memberIt->second;
        }
    }
    
    return nullptr;
}

// ConcurrencyFeatureDetector 实现

ConcurrencyFeatureDetector::ConcurrencyFeatures 
ConcurrencyFeatureDetector::detectFeatures(std::shared_ptr<ASTNode> rootNode) {
    ConcurrencyFeatures features;
    
    if (rootNode) {
        visitNode(rootNode, features);
    }
    
    return features;
}

void ConcurrencyFeatureDetector::visitNode(std::shared_ptr<ASTNode> node, ConcurrencyFeatures& features) {
    if (!node) {
        return;
    }
    
    if (auto funcDecl = std::dynamic_pointer_cast<FunctionDeclNode>(node)) {
        visitFunctionDecl(funcDecl, features);
    } else if (auto actorStmt = std::dynamic_pointer_cast<ActorStmt>(node)) {
        visitActorStmt(actorStmt, features);
    } else if (auto forAwaitStmt = std::dynamic_pointer_cast<ForAwaitStmt>(node)) {
        visitForAwaitStmt(forAwaitStmt, features);
    } else if (auto expr = std::dynamic_pointer_cast<ExprNode>(node)) {
        visitExpression(expr, features);
    }
    
    // 递归访问子节点
    // 这里需要根据具体的 AST 结构实现
}

void ConcurrencyFeatureDetector::visitFunctionDecl(std::shared_ptr<FunctionDeclNode> funcDecl, ConcurrencyFeatures& features) {
    if (funcDecl && funcDecl->isAsync()) {
        features.hasAsyncFunctions = true;
        features.asyncFunctionNames.push_back(funcDecl->getName());
    }
}

void ConcurrencyFeatureDetector::visitActorStmt(std::shared_ptr<ActorStmt> actorStmt, ConcurrencyFeatures& features) {
    if (actorStmt) {
        features.hasActors = true;
        features.actorNames.push_back(actorStmt->getName());
    }
}

void ConcurrencyFeatureDetector::visitForAwaitStmt(std::shared_ptr<ForAwaitStmt> forAwaitStmt, ConcurrencyFeatures& features) {
    if (forAwaitStmt) {
        features.hasForAwaitLoops = true;
    }
}

void ConcurrencyFeatureDetector::visitExpression(std::shared_ptr<ExprNode> expr, ConcurrencyFeatures& features) {
    if (auto awaitExpr = std::dynamic_pointer_cast<AwaitExprNode>(expr)) {
        features.hasAwaitExpressions = true;
    } else if (auto taskGroupExpr = std::dynamic_pointer_cast<TaskGroupExprNode>(expr)) {
        features.hasTaskGroups = true;
    } else if (auto asyncSeqExpr = std::dynamic_pointer_cast<AsyncSequenceExprNode>(expr)) {
        features.hasAsyncSequences = true;
    } else if (auto asyncLetExpr = std::dynamic_pointer_cast<AsyncLetExprNode>(expr)) {
        features.hasAsyncLet = true;
    }
}

// ConcurrencyCodeGenHelper 实现

ConcurrencyCodeGenHelper::AsyncFunctionInfo 
ConcurrencyCodeGenHelper::generateAsyncFunctionInfo(std::shared_ptr<AsyncFunctionType> funcType) {
    AsyncFunctionInfo info;
    
    if (funcType) {
        info.functionName = funcType->getName();
        info.returnType = funcType->getReturnType()->getName();
        info.needsAsyncContext = true;
        info.needsActorIsolation = false; // 默认值，可以根据具体情况调整
        
        for (const auto& paramType : funcType->getParameterTypes()) {
            info.parameterTypes.push_back(paramType->getName());
        }
    }
    
    return info;
}

ConcurrencyCodeGenHelper::ActorInfo 
ConcurrencyCodeGenHelper::generateActorInfo(std::shared_ptr<ActorType> actorType) {
    ActorInfo info;
    
    if (actorType) {
        info.actorName = actorType->getName();
        info.isGlobalActor = actorType->isGlobalActor();
        
        // 分类成员为隔离和非隔离
        for (const auto& member : actorType->getMembers()) {
            if (actorType->isIsolatedMember(member.first)) {
                info.isolatedMembers.push_back(member.first);
            } else {
                info.nonisolatedMembers.push_back(member.first);
            }
        }
    }
    
    return info;
}

ConcurrencyCodeGenHelper::TaskGroupInfo 
ConcurrencyCodeGenHelper::generateTaskGroupInfo(std::shared_ptr<TaskGroupType> taskGroupType) {
    TaskGroupInfo info;
    
    if (taskGroupType) {
        info.groupType = taskGroupType->getName();
        info.childTaskReturnType = taskGroupType->getChildTaskReturnType()->getName();
    }
    
    return info;
}

std::string ConcurrencyCodeGenHelper::getAsyncRuntimeFunction(const std::string& operation) {
    if (operation == "create") {
        return "swift_async_function_create";
    } else if (operation == "await") {
        return "swift_async_function_await";
    } else if (operation == "resume") {
        return "swift_async_function_resume";
    }
    return "swift_async_function_" + operation;
}

std::string ConcurrencyCodeGenHelper::getActorRuntimeFunction(const std::string& operation) {
    if (operation == "create") {
        return "swift_actor_create";
    } else if (operation == "execute") {
        return "swift_actor_execute";
    } else if (operation == "destroy") {
        return "swift_actor_destroy";
    }
    return "swift_actor_" + operation;
}

std::string ConcurrencyCodeGenHelper::getTaskGroupRuntimeFunction(const std::string& operation) {
    if (operation == "create") {
        return "swift_task_group_create";
    } else if (operation == "add_task") {
        return "swift_task_group_add_task";
    } else if (operation == "wait_all") {
        return "swift_task_group_wait_all";
    } else if (operation == "destroy") {
        return "swift_task_group_destroy";
    }
    return "swift_task_group_" + operation;
}

} // namespace miniswift