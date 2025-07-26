#include "ConcurrencyTypeSystem.h"
#include <algorithm>
#include <iostream>

namespace miniswift {

// SendableProtocolType 实现
// 基本实现已在头文件中完成

// ActorType 实现
// 基本实现已在头文件中完成

// AsyncFunctionType 实现
// 基本实现已在头文件中完成

// TaskGroupType 实现
// 基本实现已在头文件中完成

// AsyncSequenceType 实现
// 基本实现已在头文件中完成

// ConcurrencyTypeChecker 扩展实现
bool ConcurrencyTypeChecker::validateAsyncCall(const std::shared_ptr<Type>& functionType,
                                              const std::vector<std::shared_ptr<Type>>& argumentTypes,
                                              bool isInAsyncContext) const {
    auto funcType = std::dynamic_pointer_cast<FunctionType>(functionType);
    if (!funcType) {
        return false;
    }
    
    // 如果函数是异步的，调用者必须在异步上下文中
    if (funcType->getIsAsync() && !isInAsyncContext) {
        std::cerr << "Error: Async function can only be called from async context" << std::endl;
        return false;
    }
    
    // 检查参数类型匹配
    const auto& paramTypes = funcType->getParameterTypes();
    if (argumentTypes.size() != paramTypes.size()) {
        std::cerr << "Error: Argument count mismatch" << std::endl;
        return false;
    }
    
    for (size_t i = 0; i < argumentTypes.size(); ++i) {
        if (!argumentTypes[i]->equals(*paramTypes[i])) {
            std::cerr << "Error: Argument type mismatch at position " << i << std::endl;
            return false;
        }
    }
    
    return true;
}

bool ConcurrencyTypeChecker::validateActorAccess(const std::shared_ptr<ActorType>& actorType,
                                               const std::string& memberName,
                                               const std::shared_ptr<ActorType>& currentActor,
                                               bool isInAsyncContext) const {
    if (!actorType) {
        return true; // 非 Actor 类型，无需检查
    }
    
    bool isIsolated = actorType->isIsolatedMember(memberName);
    
    // 如果成员不是隔离的，可以直接访问
    if (!isIsolated) {
        return true;
    }
    
    // 如果在同一个 Actor 内部，可以直接访问隔离成员
    if (currentActor && currentActor->getName() == actorType->getName()) {
        return true;
    }
    
    // 跨 Actor 访问隔离成员必须在异步上下文中
    if (!isInAsyncContext) {
        std::cerr << "Error: Cross-actor access to isolated member '" << memberName 
                  << "' requires async context" << std::endl;
        return false;
    }
    
    return true;
}

bool ConcurrencyTypeChecker::validateSendableConstraint(const std::shared_ptr<Type>& type,
                                                       const std::string& context) const {
    if (!isSendable(type)) {
        std::cerr << "Error: Type '" << type->getName() 
                  << "' is not Sendable in context: " << context << std::endl;
        return false;
    }
    return true;
}

bool ConcurrencyTypeChecker::validateTaskGroupOperation(const std::shared_ptr<TaskGroupType>& taskGroupType,
                                                       const std::shared_ptr<Type>& taskReturnType) const {
    if (!taskGroupType) {
        std::cerr << "Error: Invalid TaskGroup type" << std::endl;
        return false;
    }
    
    // 检查任务返回类型是否与 TaskGroup 的类型参数匹配
    if (!taskReturnType->equals(*taskGroupType->getChildTaskReturnType())) {
        std::cerr << "Error: Task return type does not match TaskGroup type parameter" << std::endl;
        return false;
    }
    
    // 检查任务返回类型是否是 Sendable 的
    if (!isSendable(taskReturnType)) {
        std::cerr << "Error: Task return type must be Sendable" << std::endl;
        return false;
    }
    
    return true;
}

bool ConcurrencyTypeChecker::validateAsyncSequenceIteration(const std::shared_ptr<AsyncSequenceType>& asyncSeqType,
                                                          bool isInAsyncContext) const {
    if (!asyncSeqType) {
        std::cerr << "Error: Invalid AsyncSequence type" << std::endl;
        return false;
    }
    
    // AsyncSequence 只能在异步上下文中迭代
    if (!isInAsyncContext) {
        std::cerr << "Error: AsyncSequence can only be iterated in async context" << std::endl;
        return false;
    }
    
    // 检查元素类型是否是 Sendable 的（如果需要跨并发域传递）
    auto elementType = asyncSeqType->getElementType();
    if (!isSendable(elementType)) {
        std::cerr << "Warning: AsyncSequence element type is not Sendable" << std::endl;
        // 这里只是警告，不阻止编译
    }
    
    return true;
}

std::vector<std::string> ConcurrencyTypeChecker::getDiagnostics() const {
    return diagnostics_;
}

void ConcurrencyTypeChecker::addDiagnostic(const std::string& message) {
    diagnostics_.push_back(message);
}

void ConcurrencyTypeChecker::clearDiagnostics() {
    diagnostics_.clear();
}

// AsyncContextManager 扩展实现
void AsyncContextManager::validateAsyncOperation(const std::string& operation) const {
    if (!isInAsyncContext()) {
        throw std::runtime_error("Operation '" + operation + "' requires async context");
    }
}

void AsyncContextManager::validateActorOperation(const std::string& operation,
                                               const std::shared_ptr<ActorType>& targetActor) const {
    auto currentActor = getCurrentActor();
    
    // 如果目标 Actor 与当前 Actor 不同，需要异步上下文
    if (targetActor && currentActor && 
        targetActor->getName() != currentActor->getName() && 
        !isInAsyncContext()) {
        throw std::runtime_error("Cross-actor operation '" + operation + "' requires async context");
    }
}

bool AsyncContextManager::canAccessMember(const std::shared_ptr<ActorType>& actorType,
                                        const std::string& memberName) const {
    if (!actorType) {
        return true; // 非 Actor 类型
    }
    
    bool isIsolated = actorType->isIsolatedMember(memberName);
    if (!isIsolated) {
        return true; // 非隔离成员
    }
    
    auto currentActor = getCurrentActor();
    
    // 在同一个 Actor 内部可以访问隔离成员
    if (currentActor && currentActor->getName() == actorType->getName()) {
        return true;
    }
    
    // 跨 Actor 访问需要异步上下文
    return isInAsyncContext();
}

void AsyncContextManager::enterStructuredConcurrency(const std::string& constructType) {
    AsyncContext newContext = getCurrentContext();
    newContext.isInTaskGroup = (constructType == "TaskGroup" || constructType == "withTaskGroup");
    
    // 记录结构化并发构造的类型
    structuredConcurrencyStack_.push_back(constructType);
    contextStack_.push_back(newContext);
}

void AsyncContextManager::exitStructuredConcurrency() {
    if (contextStack_.size() > 1) {
        contextStack_.pop_back();
    }
    if (!structuredConcurrencyStack_.empty()) {
        structuredConcurrencyStack_.pop_back();
    }
}

std::string AsyncContextManager::getCurrentStructuredConcurrencyType() const {
    return structuredConcurrencyStack_.empty() ? "" : structuredConcurrencyStack_.back();
}

bool AsyncContextManager::isInStructuredConcurrency() const {
    return !structuredConcurrencyStack_.empty();
}

void AsyncContextManager::addTaskToGroup(const std::string& taskId) {
    if (isInTaskGroup()) {
        activeTaskGroups_.back().activeTasks.insert(taskId);
    }
}

void AsyncContextManager::removeTaskFromGroup(const std::string& taskId) {
    if (isInTaskGroup() && !activeTaskGroups_.empty()) {
        activeTaskGroups_.back().activeTasks.erase(taskId);
    }
}

size_t AsyncContextManager::getActiveTaskCount() const {
    if (isInTaskGroup() && !activeTaskGroups_.empty()) {
        return activeTaskGroups_.back().activeTasks.size();
    }
    return 0;
}

void AsyncContextManager::enterTaskGroup(const std::string& groupId) {
    TaskGroupInfo groupInfo;
    groupInfo.groupId = groupId;
    activeTaskGroups_.push_back(groupInfo);
    
    enterStructuredConcurrency("TaskGroup");
}

void AsyncContextManager::exitTaskGroup() {
    if (!activeTaskGroups_.empty()) {
        activeTaskGroups_.pop_back();
    }
    exitStructuredConcurrency();
}

} // namespace miniswift