#ifndef MINISWIFT_CONCURRENCY_TYPE_SYSTEM_H
#define MINISWIFT_CONCURRENCY_TYPE_SYSTEM_H

#include "TypeSystem.h"
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <vector>

namespace miniswift {

// 并发相关的类型扩展

// Sendable 协议类型
class SendableProtocolType : public Type {
public:
    SendableProtocolType() : Type(TypeKind::Protocol, "Sendable") {}
    
    bool equals(const Type& other) const override {
        return dynamic_cast<const SendableProtocolType*>(&other) != nullptr;
    }
    
    std::shared_ptr<Type> clone() const override {
        return std::make_shared<SendableProtocolType>();
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        return std::make_shared<SendableProtocolType>();
    }
};

// Actor 类型
class ActorType : public UserDefinedType {
public:
    ActorType(const std::string& name) 
        : UserDefinedType(TypeKind::Class, name), isGlobalActor_(false) {
        // Actor 默认是引用类型且线程安全
    }
    
    bool isReferenceType() const override { return true; }
    bool isActorType() const { return true; }
    bool isGlobalActor() const { return isGlobalActor_; }
    void setGlobalActor(bool isGlobal) { isGlobalActor_ = isGlobal; }
    
    // Actor 隔离检查
    bool isIsolatedMember(const std::string& memberName) const {
        auto it = memberIsolation_.find(memberName);
        return it != memberIsolation_.end() ? it->second : true; // 默认隔离
    }
    
    void setMemberIsolation(const std::string& memberName, bool isolated) {
        memberIsolation_[memberName] = isolated;
    }
    
    std::shared_ptr<Type> clone() const override {
        auto cloned = std::make_shared<ActorType>(name_);
        cloned->isGlobalActor_ = isGlobalActor_;
        cloned->memberIsolation_ = memberIsolation_;
        for (const auto& member : members_) {
            cloned->addMember(member.first, member.second->clone());
        }
        return cloned;
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        return clone();
    }
    
private:
    bool isGlobalActor_;
    std::unordered_map<std::string, bool> memberIsolation_; // true = isolated, false = nonisolated
};

// 异步函数类型（扩展 FunctionType）
class AsyncFunctionType : public FunctionType {
public:
    AsyncFunctionType(const std::vector<std::shared_ptr<Type>>& parameterTypes,
                     std::shared_ptr<Type> returnType,
                     bool isThrows = false)
        : FunctionType(parameterTypes, returnType, isThrows, true) {}
    
    // 异步函数需要异步上下文
    bool requiresAsyncContext() const { return true; }
    
    std::shared_ptr<Type> clone() const override {
        std::vector<std::shared_ptr<Type>> clonedParams;
        for (const auto& param : parameterTypes) {
            clonedParams.push_back(param->clone());
        }
        return std::make_shared<AsyncFunctionType>(clonedParams, returnType->clone(), isThrows);
    }
};

// TaskGroup 类型
class TaskGroupType : public Type {
public:
    TaskGroupType(std::shared_ptr<Type> childTaskReturnType)
        : Type(TypeKind::Generic, "TaskGroup<" + childTaskReturnType->getName() + ">"),
          childTaskReturnType_(childTaskReturnType) {}
    
    std::shared_ptr<Type> getChildTaskReturnType() const { return childTaskReturnType_; }
    
    bool equals(const Type& other) const override {
        if (auto taskGroupType = dynamic_cast<const TaskGroupType*>(&other)) {
            return childTaskReturnType_->equals(*taskGroupType->childTaskReturnType_);
        }
        return false;
    }
    
    std::shared_ptr<Type> clone() const override {
        return std::make_shared<TaskGroupType>(childTaskReturnType_->clone());
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        return clone();
    }
    
private:
    std::shared_ptr<Type> childTaskReturnType_;
};

// AsyncSequence 类型
class AsyncSequenceType : public Type {
public:
    AsyncSequenceType(std::shared_ptr<Type> elementType)
        : Type(TypeKind::Protocol, "AsyncSequence"),
          elementType_(elementType) {}
    
    std::shared_ptr<Type> getElementType() const { return elementType_; }
    
    bool equals(const Type& other) const override {
        if (auto asyncSeqType = dynamic_cast<const AsyncSequenceType*>(&other)) {
            return elementType_->equals(*asyncSeqType->elementType_);
        }
        return false;
    }
    
    std::shared_ptr<Type> clone() const override {
        return std::make_shared<AsyncSequenceType>(elementType_->clone());
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        return clone();
    }
    
private:
    std::shared_ptr<Type> elementType_;
};

// 并发类型检查器
class ConcurrencyTypeChecker {
public:
    ConcurrencyTypeChecker() {
        initializeSendableTypes();
    }
    
    // Sendable 协议检查
    bool isSendable(const std::shared_ptr<Type>& type) const {
        return checkSendable(type);
    }
    
    // Actor 隔离检查
    bool checkActorIsolation(const std::shared_ptr<Type>& actorType, 
                           const std::string& memberName,
                           bool isAsyncContext) const {
        if (auto actor = std::dynamic_pointer_cast<ActorType>(actorType)) {
            bool isIsolated = actor->isIsolatedMember(memberName);
            // 如果成员是隔离的，只能在异步上下文中访问（除非在同一个 actor 内部）
            return !isIsolated || isAsyncContext;
        }
        return true;
    }
    
    // 异步上下文检查
    bool isInAsyncContext(const std::shared_ptr<Type>& functionType) const {
        if (auto funcType = std::dynamic_pointer_cast<FunctionType>(functionType)) {
            return funcType->getIsAsync();
        }
        return false;
    }
    
    // 检查类型是否可以跨并发域传递
    bool canCrossActorBoundary(const std::shared_ptr<Type>& type) const {
        return isSendable(type);
    }
    
    // 注册自定义 Sendable 类型
    void registerSendableType(const std::string& typeName) {
        sendableTypes_.insert(typeName);
    }
    
    // 注册非 Sendable 类型
    void registerNonSendableType(const std::string& typeName) {
        nonSendableTypes_.insert(typeName);
    }
    
    // 扩展的验证方法
    bool validateAsyncCall(const std::shared_ptr<Type>& functionType,
                          const std::vector<std::shared_ptr<Type>>& argumentTypes,
                          bool isInAsyncContext) const;
    
    bool validateActorAccess(const std::shared_ptr<ActorType>& actorType,
                           const std::string& memberName,
                           const std::shared_ptr<ActorType>& currentActor,
                           bool isInAsyncContext) const;
    
    bool validateSendableConstraint(const std::shared_ptr<Type>& type,
                                   const std::string& context) const;
    
    bool validateTaskGroupOperation(const std::shared_ptr<TaskGroupType>& taskGroupType,
                                   const std::shared_ptr<Type>& taskReturnType) const;
    
    bool validateAsyncSequenceIteration(const std::shared_ptr<AsyncSequenceType>& asyncSeqType,
                                       bool isInAsyncContext) const;
    
    // 诊断信息
    std::vector<std::string> getDiagnostics() const;
    void addDiagnostic(const std::string& message);
    void clearDiagnostics();
    
private:
    std::unordered_set<std::string> sendableTypes_;
    std::unordered_set<std::string> nonSendableTypes_;
    mutable std::vector<std::string> diagnostics_;
    
    void initializeSendableTypes() {
        // 基础类型默认是 Sendable 的
        sendableTypes_.insert("Int");
        sendableTypes_.insert("Int8");
        sendableTypes_.insert("Int16");
        sendableTypes_.insert("Int32");
        sendableTypes_.insert("Int64");
        sendableTypes_.insert("UInt");
        sendableTypes_.insert("UInt8");
        sendableTypes_.insert("UInt16");
        sendableTypes_.insert("UInt32");
        sendableTypes_.insert("UInt64");
        sendableTypes_.insert("Float");
        sendableTypes_.insert("Double");
        sendableTypes_.insert("Bool");
        sendableTypes_.insert("String");
        sendableTypes_.insert("Character");
        
        // 一些类型默认不是 Sendable 的
        nonSendableTypes_.insert("NSObject");
        nonSendableTypes_.insert("UIView");
    }
    
    bool checkSendable(const std::shared_ptr<Type>& type) const {
        if (!type) return false;
        
        const std::string& typeName = type->getName();
        
        // 检查显式标记的类型
        if (nonSendableTypes_.count(typeName)) {
            return false;
        }
        
        if (sendableTypes_.count(typeName)) {
            return true;
        }
        
        // 根据类型种类判断
        switch (type->getKind()) {
            case TypeKind::Void:
            case TypeKind::Bool:
            case TypeKind::Int:
            case TypeKind::Int8:
            case TypeKind::Int16:
            case TypeKind::Int32:
            case TypeKind::Int64:
            case TypeKind::UInt:
            case TypeKind::UInt8:
            case TypeKind::UInt16:
            case TypeKind::UInt32:
            case TypeKind::UInt64:
            case TypeKind::Float:
            case TypeKind::Double:
            case TypeKind::String:
            case TypeKind::Character:
                return true;
                
            case TypeKind::Array:
                if (auto arrayType = std::dynamic_pointer_cast<ArrayType>(type)) {
                    return checkSendable(arrayType->getElementType());
                }
                return false;
                
            case TypeKind::Dictionary:
                if (auto dictType = std::dynamic_pointer_cast<DictionaryType>(type)) {
                    return checkSendable(dictType->getKeyType()) && 
                           checkSendable(dictType->getValueType());
                }
                return false;
                
            case TypeKind::Tuple:
                if (auto tupleType = std::dynamic_pointer_cast<TupleType>(type)) {
                    for (const auto& element : tupleType->getElements()) {
                        if (!checkSendable(element.type)) {
                            return false;
                        }
                    }
                    return true;
                }
                return false;
                
            case TypeKind::Optional:
                if (auto optionalType = std::dynamic_pointer_cast<OptionalType>(type)) {
                    return checkSendable(optionalType->getWrappedType());
                }
                return false;
                
            case TypeKind::Function:
                // 函数类型默认是 Sendable 的，如果它们的参数和返回值都是 Sendable 的
                if (auto funcType = std::dynamic_pointer_cast<FunctionType>(type)) {
                    for (const auto& paramType : funcType->getParameterTypes()) {
                        if (!checkSendable(paramType)) {
                            return false;
                        }
                    }
                    return checkSendable(funcType->getReturnType());
                }
                return false;
                
            case TypeKind::Struct:
                // 结构体如果所有成员都是 Sendable 的，则它也是 Sendable 的
                if (auto userType = std::dynamic_pointer_cast<UserDefinedType>(type)) {
                    for (const auto& member : userType->getMembers()) {
                        if (!checkSendable(member.second)) {
                            return false;
                        }
                    }
                    return true;
                }
                return false;
                
            case TypeKind::Class:
                // 类默认不是 Sendable 的，除非显式标记
                // Actor 类型是特殊的，它们有自己的并发安全保证
                if (std::dynamic_pointer_cast<ActorType>(type)) {
                    return true; // Actor 类型本身是 Sendable 的
                }
                return false;
                
            case TypeKind::Enum:
                // 枚举如果所有关联值都是 Sendable 的，则它也是 Sendable 的
                return true; // 简化实现，假设枚举是 Sendable 的
                
            default:
                return false;
        }
    }
};

// 异步上下文管理器
class AsyncContextManager {
public:
    struct AsyncContext {
        std::shared_ptr<ActorType> currentActor;
        bool isInAsyncFunction;
        bool isInTaskGroup;
        std::vector<std::string> availableAsyncVariables;
        
        AsyncContext() : isInAsyncFunction(false), isInTaskGroup(false) {}
    };
    
    AsyncContextManager() {
        contextStack_.push_back(AsyncContext());
    }
    
    // 进入异步上下文
    void enterAsyncContext(std::shared_ptr<ActorType> actor = nullptr) {
        AsyncContext newContext = getCurrentContext();
        newContext.isInAsyncFunction = true;
        newContext.currentActor = actor;
        contextStack_.push_back(newContext);
    }
    
    // 退出异步上下文
    void exitAsyncContext() {
        if (contextStack_.size() > 1) {
            contextStack_.pop_back();
        }
    }
    
    // 进入 TaskGroup 上下文
    void enterTaskGroupContext() {
        AsyncContext newContext = getCurrentContext();
        newContext.isInTaskGroup = true;
        contextStack_.push_back(newContext);
    }
    
    // 退出 TaskGroup 上下文
    void exitTaskGroupContext() {
        if (contextStack_.size() > 1) {
            contextStack_.pop_back();
        }
    }
    
    // 获取当前上下文
    const AsyncContext& getCurrentContext() const {
        return contextStack_.back();
    }
    
    // 检查是否在异步上下文中
    bool isInAsyncContext() const {
        return getCurrentContext().isInAsyncFunction;
    }
    
    // 检查是否在 TaskGroup 中
    bool isInTaskGroup() const {
        return getCurrentContext().isInTaskGroup;
    }
    
    // 获取当前 Actor
    std::shared_ptr<ActorType> getCurrentActor() const {
        return getCurrentContext().currentActor;
    }
    
    // 添加异步变量
    void addAsyncVariable(const std::string& varName) {
        if (!contextStack_.empty()) {
            contextStack_.back().availableAsyncVariables.push_back(varName);
        }
    }
    
    // 检查异步变量是否可用
    bool isAsyncVariableAvailable(const std::string& varName) const {
        const auto& vars = getCurrentContext().availableAsyncVariables;
        return std::find(vars.begin(), vars.end(), varName) != vars.end();
    }
    
    // 扩展的验证和管理方法
    void validateAsyncOperation(const std::string& operation) const;
    void validateActorOperation(const std::string& operation,
                               const std::shared_ptr<ActorType>& targetActor) const;
    bool canAccessMember(const std::shared_ptr<ActorType>& actorType,
                        const std::string& memberName) const;
    
    // 结构化并发支持
    void enterStructuredConcurrency(const std::string& constructType);
    void exitStructuredConcurrency();
    std::string getCurrentStructuredConcurrencyType() const;
    bool isInStructuredConcurrency() const;
    
    // TaskGroup 管理
    void enterTaskGroup(const std::string& groupId);
    void exitTaskGroup();
    void addTaskToGroup(const std::string& taskId);
    void removeTaskFromGroup(const std::string& taskId);
    size_t getActiveTaskCount() const;
    
private:
    struct TaskGroupInfo {
        std::string groupId;
        std::unordered_set<std::string> activeTasks;
    };
    
    std::vector<AsyncContext> contextStack_;
    std::vector<std::string> structuredConcurrencyStack_;
    std::vector<TaskGroupInfo> activeTaskGroups_;
};

} // namespace miniswift

#endif // MINISWIFT_CONCURRENCY_TYPE_SYSTEM_H