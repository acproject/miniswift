#ifndef MINISWIFT_TYPE_SYSTEM_H
#define MINISWIFT_TYPE_SYSTEM_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <iostream>
#include "../interpreter/Concurrency/ConcurrencyRuntime.h"

namespace miniswift {

// 前向声明
class Type;
class TypeChecker;
class TypeInference;

// 类型种类枚举
enum class TypeKind {
    // 基础类型
    Void,
    Bool,
    Int,
    Int8,
    Int16,
    Int32,
    Int64,
    UInt,
    UInt8,
    UInt16,
    UInt32,
    UInt64,
    Float,
    Double,
    String,
    Character,
    
    // 复合类型
    Array,
    Dictionary,
    Tuple,
    Optional,
    Function,
    
    // 用户定义类型
    Struct,
    Class,
    Enum,
    Protocol,
    
    // 泛型和高级类型
    Generic,
    Associated,
    Existential,
    Opaque,
    
    // 特殊类型
    Any,
    AnyObject,
    Never,
    Unknown,
    Error
};

// 类型基类
class Type {
public:
    Type(TypeKind kind, const std::string& name) : kind_(kind), name_(name) {}
    virtual ~Type() = default;
    
    TypeKind getKind() const { return kind_; }
    const std::string& getName() const { return name_; }
    
    // 类型比较
    virtual bool equals(const Type& other) const;
    virtual bool isSubtypeOf(const Type& other) const;
    virtual bool isConvertibleTo(const Type& other) const;
    
    // 类型属性
    virtual bool isValueType() const { return true; }
    virtual bool isReferenceType() const { return false; }
    virtual bool isOptional() const { return false; }
    virtual bool isGeneric() const { return false; }
    virtual bool isConcrete() const { return true; }
    virtual bool isActorType() const { return false; }
    virtual bool isActorIsolated() const { return false; }
    
    // 类型操作
    virtual std::shared_ptr<Type> getOptionalType() const;
    virtual std::shared_ptr<Type> getUnwrappedType() const;
    virtual std::string toString() const { return name_; }
    
    // 克隆
    virtual std::shared_ptr<Type> clone() const { return cloneImpl(); }
    
protected:
    // 内部实现方法
    virtual bool equalsImpl(const Type& other) const;
    virtual bool isSubtypeOfImpl(const Type& other) const;
    virtual bool isConvertibleToImpl(const Type& other) const;
    virtual std::shared_ptr<Type> cloneImpl() const = 0;
    
    TypeKind kind_;
    std::string name_;
};

// 基础类型
class PrimitiveType : public Type {
public:
    PrimitiveType(TypeKind kind, const std::string& name) : Type(kind, name) {}
    
    std::shared_ptr<Type> clone() const override {
        return cloneImpl();
    }
    
protected:
    bool isConvertibleToImpl(const Type& other) const override;
    std::shared_ptr<Type> cloneImpl() const override;
};

// 数组类型
class ArrayType : public Type {
public:
    ArrayType(std::shared_ptr<Type> elementType)
        : Type(TypeKind::Array, "Array<" + elementType->getName() + ">"),
          elementType_(elementType) {}
    
    std::shared_ptr<Type> getElementType() const { return elementType_; }
    
    bool equals(const Type& other) const override {
        if (auto arrayType = dynamic_cast<const ArrayType*>(&other)) {
            return elementType_->equals(*arrayType->elementType_);
        }
        return false;
    }
    
    std::shared_ptr<Type> clone() const override {
        return cloneImpl();
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        return std::make_shared<ArrayType>(elementType_->clone());
    }
    
private:
    std::shared_ptr<Type> elementType_;
};

// 字典类型
class DictionaryType : public Type {
public:
    DictionaryType(std::shared_ptr<Type> keyType, std::shared_ptr<Type> valueType)
        : Type(TypeKind::Dictionary, "Dictionary<" + keyType->getName() + ", " + valueType->getName() + ">"),
          keyType_(keyType), valueType_(valueType) {}
    
    std::shared_ptr<Type> getKeyType() const { return keyType_; }
    std::shared_ptr<Type> getValueType() const { return valueType_; }
    
    bool equals(const Type& other) const override {
        if (auto dictType = dynamic_cast<const DictionaryType*>(&other)) {
            return keyType_->equals(*dictType->keyType_) && 
                   valueType_->equals(*dictType->valueType_);
        }
        return false;
    }
    
    std::shared_ptr<Type> clone() const override {
        return cloneImpl();
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        return std::make_shared<DictionaryType>(keyType_->clone(), valueType_->clone());
    }
    
private:
    std::shared_ptr<Type> keyType_;
    std::shared_ptr<Type> valueType_;
};

// 元组类型
class TupleType : public Type {
public:
    struct TupleElement {
        std::shared_ptr<Type> type;
        std::string label;
        
        TupleElement(std::shared_ptr<Type> t, const std::string& l = "")
            : type(t), label(l) {}
    };
    
    TupleType(const std::vector<TupleElement>& elements)
        : Type(TypeKind::Tuple, buildTupleName(elements)), elements_(elements) {}
    
    const std::vector<TupleElement>& getElements() const { return elements_; }
    size_t getElementCount() const { return elements_.size(); }
    std::shared_ptr<Type> getElementType(size_t index) const {
        return index < elements_.size() ? elements_[index].type : nullptr;
    }
    
    bool equals(const Type& other) const override {
        if (auto tupleType = dynamic_cast<const TupleType*>(&other)) {
            if (elements_.size() != tupleType->elements_.size()) return false;
            for (size_t i = 0; i < elements_.size(); ++i) {
                if (!elements_[i].type->equals(*tupleType->elements_[i].type)) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    
    std::shared_ptr<Type> clone() const override {
        return cloneImpl();
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        std::vector<TupleElement> clonedElements;
        for (const auto& elem : elements_) {
            clonedElements.emplace_back(elem.type->clone(), elem.label);
        }
        return std::make_shared<TupleType>(clonedElements);
    }
    
private:
    std::vector<TupleElement> elements_;
    
    static std::string buildTupleName(const std::vector<TupleElement>& elements) {
        std::string name = "(";
        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) name += ", ";
            if (!elements[i].label.empty()) {
                name += elements[i].label + ": ";
            }
            name += elements[i].type->getName();
        }
        name += ")";
        return name;
    }
};

// 可选类型
class OptionalType : public Type {
public:
    OptionalType(std::shared_ptr<Type> wrappedType)
        : Type(TypeKind::Optional, wrappedType->getName() + "?"),
          wrappedType_(wrappedType) {}
    
    std::shared_ptr<Type> getWrappedType() const { return wrappedType_; }
    
    bool isOptional() const override { return true; }
    
    std::shared_ptr<Type> getUnwrappedType() const override {
        return wrappedType_;
    }
    
    bool equals(const Type& other) const override {
        if (auto optionalType = dynamic_cast<const OptionalType*>(&other)) {
            return wrappedType_->equals(*optionalType->wrappedType_);
        }
        return false;
    }
    
    std::shared_ptr<Type> clone() const override {
        return cloneImpl();
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        return std::make_shared<OptionalType>(wrappedType_->clone());
    }
    
private:
    std::shared_ptr<Type> wrappedType_;
};

// 函数类型
class FunctionType : public Type {
public:
    FunctionType(const std::vector<std::shared_ptr<Type>>& parameterTypes,
                 std::shared_ptr<Type> returnType,
                 bool isThrows = false,
                 bool isAsync = false)
        : Type(TypeKind::Function, buildFunctionName(parameterTypes, returnType, isThrows, isAsync)),
          parameterTypes(parameterTypes), returnType(returnType),
          isThrows(isThrows), isAsync(isAsync) {}
    
    const std::vector<std::shared_ptr<Type>>& getParameterTypes() const { return parameterTypes; }
    std::shared_ptr<Type> getReturnType() const { return returnType; }
    bool getIsThrows() const { return isThrows; }
    bool getIsAsync() const { return isAsync; }
    
    bool equals(const Type& other) const override {
        if (auto funcType = dynamic_cast<const FunctionType*>(&other)) {
            if (parameterTypes.size() != funcType->parameterTypes.size()) return false;
            if (!returnType->equals(*funcType->returnType)) return false;
            if (isThrows != funcType->isThrows || isAsync != funcType->isAsync) return false;
            
            for (size_t i = 0; i < parameterTypes.size(); ++i) {
                if (!parameterTypes[i]->equals(*funcType->parameterTypes[i])) {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    
    std::shared_ptr<Type> clone() const override {
        return cloneImpl();
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        std::vector<std::shared_ptr<Type>> clonedParams;
        for (const auto& param : parameterTypes) {
            clonedParams.push_back(param->clone());
        }
        return std::make_shared<FunctionType>(clonedParams, returnType->clone(), isThrows, isAsync);
    }
    
public:
    std::vector<std::shared_ptr<Type>> parameterTypes;
    std::shared_ptr<Type> returnType;
    bool isThrows;
    bool isAsync;
    
private:
    static std::string buildFunctionName(const std::vector<std::shared_ptr<Type>>& parameterTypes,
                                        std::shared_ptr<Type> returnType,
                                        bool isThrows, bool isAsync) {
        std::string name = "(";
        for (size_t i = 0; i < parameterTypes.size(); ++i) {
            if (i > 0) name += ", ";
            name += parameterTypes[i]->getName();
        }
        name += ")";
        if (isAsync) name += " async";
        if (isThrows) name += " throws";
        name += " -> " + returnType->getName();
        return name;
    }
};

// 用户定义类型
class UserDefinedType : public Type {
public:
    UserDefinedType(TypeKind kind, const std::string& name)
        : Type(kind, name) {}
    
    // 成员管理
    void addMember(const std::string& name, std::shared_ptr<Type> type) {
        members_[name] = type;
    }
    
    std::shared_ptr<Type> getMemberType(const std::string& name) const {
        auto it = members_.find(name);
        return it != members_.end() ? it->second : nullptr;
    }
    
    const std::unordered_map<std::string, std::shared_ptr<Type>>& getMembers() const {
        return members_;
    }
    
    bool isReferenceType() const override {
        return kind_ == TypeKind::Class;
    }
    
    std::shared_ptr<Type> clone() const override {
        return cloneImpl();
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        auto cloned = std::make_shared<UserDefinedType>(kind_, name_);
        for (const auto& member : members_) {
            cloned->addMember(member.first, member.second->clone());
        }
        return cloned;
    }
    
protected:
    std::unordered_map<std::string, std::shared_ptr<Type>> members_;
};

// 泛型类型
class GenericType : public Type {
public:
    GenericType(const std::string& name, const std::vector<std::string>& constraints = {})
        : Type(TypeKind::Generic, name), constraints_(constraints) {}
    
    const std::vector<std::string>& getConstraints() const { return constraints_; }
    
    bool isGeneric() const override { return true; }
    bool isConcrete() const override { return false; }
    
    std::shared_ptr<Type> clone() const override {
        return cloneImpl();
    }
    
protected:
    std::shared_ptr<Type> cloneImpl() const override {
        return std::make_shared<GenericType>(name_, constraints_);
    }
    
private:
    std::vector<std::string> constraints_;
};

// 类型系统主类
class TypeSystem {
public:
    TypeSystem();
    virtual ~TypeSystem() = default;
    
    // 内置类型获取
    std::shared_ptr<Type> getVoidType() const;
    std::shared_ptr<Type> getBoolType() const;
    std::shared_ptr<Type> getIntType() const;
    std::shared_ptr<Type> getFloatType() const;
    std::shared_ptr<Type> getDoubleType() const;
    std::shared_ptr<Type> getStringType() const;
    std::shared_ptr<Type> getAnyType() const;
    std::shared_ptr<Type> getErrorType() const;
    std::shared_ptr<Type> getNeverType() const;
    
    // 复合类型创建和获取
    std::shared_ptr<Type> createArrayType(std::shared_ptr<Type> elementType);
    std::shared_ptr<ArrayType> getArrayType(std::shared_ptr<Type> elementType);
    std::shared_ptr<Type> createDictionaryType(std::shared_ptr<Type> keyType, std::shared_ptr<Type> valueType);
    std::shared_ptr<DictionaryType> getDictionaryType(std::shared_ptr<Type> keyType, std::shared_ptr<Type> valueType);
    std::shared_ptr<Type> createTupleType(const std::vector<std::shared_ptr<Type>>& elementTypes);
    std::shared_ptr<TupleType> getTupleType(const std::vector<std::shared_ptr<Type>>& elementTypes);
    std::shared_ptr<Type> createOptionalType(std::shared_ptr<Type> wrappedType);
    std::shared_ptr<OptionalType> getOptionalType(std::shared_ptr<Type> wrappedType);
    std::shared_ptr<Type> createFunctionType(const std::vector<std::shared_ptr<Type>>& parameterTypes,
                                            std::shared_ptr<Type> returnType,
                                            bool isThrows = false, bool isAsync = false);
    std::shared_ptr<FunctionType> getFunctionType(const std::vector<std::shared_ptr<Type>>& parameterTypes,
                                                  std::shared_ptr<Type> returnType);
    
    // 用户定义类型
    std::shared_ptr<Type> createStructType(const std::string& name);
    std::shared_ptr<Type> createClassType(const std::string& name);
    std::shared_ptr<Type> createEnumType(const std::string& name);
    std::shared_ptr<Type> createProtocolType(const std::string& name);
    
    // 泛型类型
    std::shared_ptr<Type> createGenericType(const std::string& name, const std::vector<std::string>& constraints = {});
    std::shared_ptr<GenericType> createGenericType(const std::string& name);
    
    // 类型查找和注册
    std::shared_ptr<Type> lookupType(const std::string& name) const;
    std::shared_ptr<Type> getTypeByName(const std::string& name) const;
    void registerType(const std::string& name, std::shared_ptr<Type> type);
    void registerUserDefinedType(std::shared_ptr<UserDefinedType> type);
    
    // 类型检查
    bool areTypesEqual(const std::shared_ptr<Type>& type1, const std::shared_ptr<Type>& type2) const;
    bool isSubtype(const std::shared_ptr<Type>& subtype, const std::shared_ptr<Type>& supertype) const;
    bool isConvertible(const std::shared_ptr<Type>& from, const std::shared_ptr<Type>& to) const;
    
    // 类型推断
    std::shared_ptr<Type> inferCommonType(const std::vector<std::shared_ptr<Type>>& types) const;
    std::shared_ptr<Type> inferBinaryOperationType(const std::shared_ptr<Type>& left,
                                                   const std::shared_ptr<Type>& right,
                                                   const std::string& operator_) const;
    std::shared_ptr<Type> findCommonType(const std::shared_ptr<Type>& type1, const std::shared_ptr<Type>& type2) const;
    std::shared_ptr<Type> inferType(const std::vector<std::shared_ptr<Type>>& candidateTypes) const;
    
    // 泛型实例化
    std::shared_ptr<Type> instantiateGenericType(const std::shared_ptr<Type>& genericType,
                                                 const std::unordered_map<std::string, std::shared_ptr<Type>>& substitutions) const;
    std::shared_ptr<Type> instantiateGenericType(std::shared_ptr<GenericType> genericType,
                                                 const std::vector<std::shared_ptr<Type>>& typeArguments);
    
    // 类型约束检查
    bool satisfiesConstraints(const std::shared_ptr<Type>& type, const std::vector<std::string>& constraints) const;
    bool checkGenericConstraints(std::shared_ptr<GenericType> genericType, std::shared_ptr<Type> concreteType) const;
    
    // 辅助方法
    bool isNumericType(std::shared_ptr<Type> type) const;
    bool areTypesComparable(std::shared_ptr<Type> type1, std::shared_ptr<Type> type2) const;
    std::vector<std::string> getBuiltinTypeNames() const;
    std::vector<std::string> getUserDefinedTypeNames() const;
    size_t getTypeCount() const;
    void dumpTypes() const;
    
private:
    // 内置类型实例
    std::shared_ptr<Type> intType_;
    std::shared_ptr<Type> floatType_;
    std::shared_ptr<Type> boolType_;
    std::shared_ptr<Type> stringType_;
    std::shared_ptr<Type> voidType_;
    std::shared_ptr<Type> anyType_;
    std::shared_ptr<Type> errorType_;
    
    // 复合类型缓存
    std::vector<std::shared_ptr<ArrayType>> arrayTypes_;
    std::vector<std::shared_ptr<DictionaryType>> dictionaryTypes_;
    std::vector<std::shared_ptr<TupleType>> tupleTypes_;
    std::vector<std::shared_ptr<OptionalType>> optionalTypes_;
    std::vector<std::shared_ptr<FunctionType>> functionTypes_;
    
    // 用户定义类型和泛型类型
    std::unordered_map<std::string, std::shared_ptr<UserDefinedType>> userDefinedTypes_;
    std::unordered_map<std::string, std::shared_ptr<GenericType>> genericTypes_;
    
    // 内置类型存储
    std::unordered_map<std::string, std::shared_ptr<Type>> builtinTypes_;
    std::unordered_map<std::string, std::shared_ptr<Type>> userTypes_;
    
    // 类型缓存（用于复合类型）
    std::unordered_map<std::string, std::shared_ptr<Type>> typeCache_;
    
    // 初始化内置类型
    void initializeBuiltinTypes();
    
    // 辅助方法
    std::string generateTypeCacheKey(const std::string& baseType, const std::vector<std::string>& parameters) const;
};

// ============================================================================
// 并发类型系统扩展
// ============================================================================

// 并发类型种类
enum class ConcurrencyTypeKind {
    ACTOR,
    TASK,
    TASK_GROUP,
    ASYNC_SEQUENCE,
    ASYNC_ITERATOR,
    SENDABLE_PROTOCOL,
    MAIN_ACTOR,
    GLOBAL_ACTOR,
    CONTINUATION,
    ASYNC_STREAM,
    ASYNC_THROWING_STREAM
};

// 并发安全级别
enum class ConcurrencySafetyLevel {
    SENDABLE,           // 类型符合Sendable协议
    ACTOR_ISOLATED,     // Actor隔离类型
    MAIN_ACTOR,         // MainActor隔离
    GLOBAL_ACTOR,       // 全局Actor隔离
    UNSAFE              // 不安全的并发访问
};

// TaskPriority 定义在 ConcurrencyRuntime.h 中

// Actor隔离级别
enum class ActorIsolation {
    NONE,               // 无隔离
    ACTOR_ISOLATED,     // Actor隔离
    MAIN_ACTOR,         // MainActor隔离
    GLOBAL_ACTOR,       // 全局Actor隔离
    NONISOLATED         // 显式非隔离
};

// 并发类型检查错误
struct ConcurrencyTypeError {
    std::string message;
    int line;
    int column;
    std::string errorCode;
    
    ConcurrencyTypeError(const std::string& msg, int l = 0, int c = 0, const std::string& code = "")
        : message(msg), line(l), column(c), errorCode(code) {}
};

// Actor类型
class ActorType : public Type {
public:
    ActorType(const std::string& name, bool isGlobalActor = false);
    
    bool isGlobalActor() const { return isGlobalActor_; }
    void setGlobalActor(bool global) { isGlobalActor_ = global; }
    
    // 成员管理
    void addProperty(const std::string& name, std::shared_ptr<Type> type, ActorIsolation isolation = ActorIsolation::ACTOR_ISOLATED);
    void addMethod(const std::string& name, std::shared_ptr<FunctionType> type, ActorIsolation isolation = ActorIsolation::ACTOR_ISOLATED);
    
    std::shared_ptr<Type> getPropertyType(const std::string& name) const;
    std::shared_ptr<FunctionType> getMethodType(const std::string& name) const;
    
    ActorIsolation getPropertyIsolation(const std::string& name) const;
    ActorIsolation getMethodIsolation(const std::string& name) const;
    
    bool hasProperty(const std::string& name) const;
    bool hasMethod(const std::string& name) const;
    
    std::vector<std::string> getPropertyNames() const;
    std::vector<std::string> getMethodNames() const;
    
    std::string toString() const override;
    bool equals(const Type& other) const override;
    bool isActorType() const override { return true; }
    bool isActorIsolated() const override { return true; }

protected:
    std::shared_ptr<Type> cloneImpl() const override;
    
private:
    bool isGlobalActor_;
    std::unordered_map<std::string, std::shared_ptr<Type>> properties_;
    std::unordered_map<std::string, std::shared_ptr<FunctionType>> methods_;
    std::unordered_map<std::string, ActorIsolation> propertyIsolations_;
    std::unordered_map<std::string, ActorIsolation> methodIsolations_;
};

// Task类型
class TaskType : public Type {
public:
    TaskType(std::shared_ptr<Type> resultType, TaskPriority priority = TaskPriority::Default);
    
    std::shared_ptr<Type> getResultType() const { return resultType_; }
    TaskPriority getPriority() const { return priority_; }
    
    void setPriority(TaskPriority priority) { priority_ = priority; }
    
    std::string toString() const override;
    bool equals(const Type& other) const override;
    
protected:
    std::shared_ptr<Type> cloneImpl() const override;
    
private:
    std::shared_ptr<Type> resultType_;
    TaskPriority priority_;
};

// TaskGroup类型
class TaskGroupType : public Type {
public:
    TaskGroupType(std::shared_ptr<Type> childResultType);
    
    std::shared_ptr<Type> getChildResultType() const { return childResultType_; }
    
    std::string toString() const override;
    bool equals(const Type& other) const override;
    
protected:
    std::shared_ptr<Type> cloneImpl() const override;
    
private:
    std::shared_ptr<Type> childResultType_;
};

// AsyncSequence类型
class AsyncSequenceType : public Type {
public:
    AsyncSequenceType(std::shared_ptr<Type> elementType);
    
    std::shared_ptr<Type> getElementType() const { return elementType_; }
    
    std::string toString() const override;
    bool equals(const Type& other) const override;
    
protected:
    std::shared_ptr<Type> cloneImpl() const override;
    
private:
    std::shared_ptr<Type> elementType_;
};

// AsyncIterator类型
class AsyncIteratorType : public Type {
public:
    AsyncIteratorType(std::shared_ptr<Type> elementType);
    
    std::shared_ptr<Type> getElementType() const { return elementType_; }
    
    std::string toString() const override;
    bool equals(const Type& other) const override;
    
protected:
    std::shared_ptr<Type> cloneImpl() const override;
    
private:
    std::shared_ptr<Type> elementType_;
};

// Sendable协议类型
class SendableProtocolType : public Type {
public:
    SendableProtocolType();
    
    std::string toString() const override;
    bool equals(const Type& other) const override;
    
    // 检查类型是否符合Sendable协议
    static bool conformsToSendable(const std::shared_ptr<Type>& type);
    
protected:
    std::shared_ptr<Type> cloneImpl() const override;
};

// Continuation类型
class ContinuationType : public Type {
public:
    ContinuationType(std::shared_ptr<Type> resultType, bool throwing = false);
    
    std::shared_ptr<Type> getResultType() const { return resultType_; }
    bool isThrowing() const { return throwing_; }
    
    std::string toString() const override;
    bool equals(const Type& other) const override;
    
protected:
    std::shared_ptr<Type> cloneImpl() const override;
    
private:
    std::shared_ptr<Type> resultType_;
    bool throwing_;
};

// AsyncStream类型
class AsyncStreamType : public Type {
public:
    AsyncStreamType(std::shared_ptr<Type> elementType, bool throwing = false);
    
    std::shared_ptr<Type> getElementType() const { return elementType_; }
    bool isThrowing() const { return throwing_; }
    
    std::string toString() const override;
    bool equals(const Type& other) const override;
    
protected:
    std::shared_ptr<Type> cloneImpl() const override;
    
private:
    std::shared_ptr<Type> elementType_;
    bool throwing_;
};

// 并发类型检查器
class ConcurrencyTypeChecker {
public:
    ConcurrencyTypeChecker(std::shared_ptr<TypeSystem> typeSystem);
    
    // 主要检查接口
    bool validateAsyncCall(const std::shared_ptr<FunctionType>& funcType, 
                          const std::vector<std::shared_ptr<Type>>& argTypes,
                          bool inAsyncContext);
    
    bool validateActorAccess(const std::shared_ptr<ActorType>& actorType,
                           const std::string& memberName,
                           const std::string& currentActorContext,
                           bool inAsyncContext);
    
    bool validateSendableConstraint(const std::shared_ptr<Type>& type,
                                  const std::string& context);
    
    bool validateTaskGroupOperation(const std::shared_ptr<TaskGroupType>& taskGroupType,
                                  const std::string& operation,
                                  const std::vector<std::shared_ptr<Type>>& operandTypes);
    
    bool validateAsyncSequenceIteration(const std::shared_ptr<AsyncSequenceType>& seqType,
                                       bool inAsyncContext);
    
    // 类型创建和管理
    std::shared_ptr<ActorType> createActorType(const std::string& name, bool isGlobalActor = false);
    std::shared_ptr<TaskType> createTaskType(std::shared_ptr<Type> resultType, TaskPriority priority = TaskPriority::Default);
    std::shared_ptr<TaskGroupType> createTaskGroupType(std::shared_ptr<Type> childResultType);
    std::shared_ptr<AsyncSequenceType> createAsyncSequenceType(std::shared_ptr<Type> elementType);
    std::shared_ptr<AsyncIteratorType> createAsyncIteratorType(std::shared_ptr<Type> elementType);
    std::shared_ptr<ContinuationType> createContinuationType(std::shared_ptr<Type> resultType, bool throwing = false);
    std::shared_ptr<AsyncStreamType> createAsyncStreamType(std::shared_ptr<Type> elementType, bool throwing = false);
    
    // 类型查询
    bool isActorType(const std::shared_ptr<Type>& type) const;
    bool isTaskType(const std::shared_ptr<Type>& type) const;
    bool isAsyncSequenceType(const std::shared_ptr<Type>& type) const;
    bool isSendableType(const std::shared_ptr<Type>& type) const;
    
    // 安全级别检查
    ConcurrencySafetyLevel getSafetyLevel(const std::shared_ptr<Type>& type) const;
    ActorIsolation getActorIsolation(const std::shared_ptr<Type>& type) const;
    
    // 类型转换和兼容性
    bool canCrossActorBoundary(const std::shared_ptr<Type>& type) const;
    bool requiresAsyncContext(const std::shared_ptr<Type>& type) const;
    bool isMainActorIsolated(const std::shared_ptr<Type>& type) const;
    
    // 错误处理
    std::vector<ConcurrencyTypeError> getDiagnostics() const;
    void addDiagnostic(const std::string& message, int line = 0, int column = 0, const std::string& errorCode = "");
    void clearDiagnostics();
    bool hasErrors() const;
    
private:
    std::shared_ptr<TypeSystem> typeSystem_;
    std::vector<ConcurrencyTypeError> diagnostics_;
    
    // 内置Sendable类型集合
    std::unordered_set<std::string> builtinSendableTypes_;
    
    // 注册的Actor类型
    std::unordered_map<std::string, std::shared_ptr<ActorType>> actorTypes_;
    
    // 私有辅助方法
    void initializeBuiltinSendableTypes();
    bool isBuiltinSendableType(const std::shared_ptr<Type>& type) const;
    bool checkSendableConformance(const std::shared_ptr<Type>& type) const;
    bool validateActorIsolationAccess(const std::shared_ptr<ActorType>& actorType,
                                     const std::string& memberName,
                                     ActorIsolation currentIsolation) const;
    std::string getActorIsolationString(ActorIsolation isolation) const;
    std::string getTaskPriorityString(TaskPriority priority) const;
};

// 并发上下文管理器
class AsyncContextManager {
public:
    AsyncContextManager();
    
    // 上下文管理
    void enterAsyncContext(const std::string& contextName = "");
    void exitAsyncContext();
    bool isInAsyncContext() const;
    
    void enterActorContext(const std::string& actorName);
    void exitActorContext();
    std::string getCurrentActorContext() const;
    bool isInActorContext() const;
    
    void enterTaskGroup(const std::string& groupId);
    void exitTaskGroup();
    bool isInTaskGroup() const;
    std::string getCurrentTaskGroup() const;
    
    // 任务管理
    void addTaskToGroup(const std::string& taskId);
    void removeTaskFromGroup(const std::string& taskId);
    std::vector<std::string> getTasksInCurrentGroup() const;
    
    // 上下文查询
    int getAsyncContextDepth() const;
    std::vector<std::string> getContextStack() const;
    
    void reset();
    
private:
    struct ContextFrame {
        std::string type;  // "async", "actor", "taskgroup"
        std::string name;
        std::vector<std::string> tasks;  // 仅用于taskgroup
        
        ContextFrame(const std::string& t, const std::string& n) : type(t), name(n) {}
    };
    
    std::vector<ContextFrame> contextStack_;
    
    // 辅助方法
    ContextFrame* getCurrentFrame();
    const ContextFrame* getCurrentFrame() const;
    ContextFrame* findFrameByType(const std::string& type);
    const ContextFrame* findFrameByType(const std::string& type) const;
};

// 并发类型系统工厂
class ConcurrencyTypeFactory {
public:
    static std::shared_ptr<ActorType> createMainActorType();
    static std::shared_ptr<SendableProtocolType> createSendableProtocol();
    static std::shared_ptr<TaskType> createDetachedTask(std::shared_ptr<Type> resultType, TaskPriority priority = TaskPriority::Default);
    static std::shared_ptr<AsyncSequenceType> createAsyncSequence(std::shared_ptr<Type> elementType);
    
    // 内置并发类型创建
    static std::shared_ptr<Type> createBuiltinAsyncType(const std::string& typeName);
    static std::shared_ptr<FunctionType> createAsyncFunctionType(
        const std::vector<std::shared_ptr<Type>>& paramTypes,
        std::shared_ptr<Type> returnType,
        bool throwing = false
    );
};

} // namespace miniswift

#endif // MINISWIFT_TYPE_SYSTEM_H