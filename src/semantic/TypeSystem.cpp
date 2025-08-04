#include "TypeSystem.h"
#include <algorithm>
#include <sstream>

namespace miniswift {

// Type基类实现 - 移除实现，使用头文件中的内联定义

bool Type::equals(const Type& other) const {
    return kind_ == other.kind_ && equalsImpl(other);
}

bool Type::isSubtypeOf(const Type& other) const {
    if (equals(other)) {
        return true;
    }
    return isSubtypeOfImpl(other);
}

bool Type::isConvertibleTo(const Type& other) const {
    if (isSubtypeOf(other)) {
        return true;
    }
    return isConvertibleToImpl(other);
}

// 默认实现
bool Type::equalsImpl(const Type& other) const {
    return true; // 基类只比较kind
}

bool Type::isSubtypeOfImpl(const Type& other) const {
    return false; // 默认不是子类型
}

bool Type::isConvertibleToImpl(const Type& other) const {
    return false; // 默认不可转换
}

std::shared_ptr<Type> Type::getOptionalType() const {
    // 默认实现：返回包装此类型的可选类型
    return nullptr; // 子类应该重写此方法
}

std::shared_ptr<Type> Type::getUnwrappedType() const {
    // 默认实现：非可选类型返回自身
    return nullptr; // 子类应该重写此方法
}

// PrimitiveType实现 - 移除实现，使用头文件中的内联定义

// PrimitiveType使用基类Type的toString()方法

bool PrimitiveType::isConvertibleToImpl(const Type& other) const {
    if (other.getKind() == TypeKind::Any) {
        return true; // 所有类型都可以转换为Any
    }
    
    // 数值类型之间的转换
    if ((getKind() == TypeKind::Int || getKind() == TypeKind::Float) &&
        (other.getKind() == TypeKind::Int || other.getKind() == TypeKind::Float)) {
        return true;
    }
    
    // Bool可以转换为String
    if (getKind() == TypeKind::Bool && other.getKind() == TypeKind::String) {
        return true;
    }
    
    return false;
}

std::shared_ptr<Type> PrimitiveType::cloneImpl() const {
    return std::make_shared<PrimitiveType>(getKind(), getName());
}

// ArrayType实现 - 移除实现，使用头文件中的内联定义

// DictionaryType实现 - 移除实现，使用头文件中的内联定义

// TupleType实现 - 移除实现，使用头文件中的内联定义

// OptionalType实现 - 移除实现，使用头文件中的内联定义

// FunctionType实现 - 移除实现，使用头文件中的内联定义

// UserDefinedType实现 - 移除实现，使用头文件中的内联定义

// GenericType实现 - 移除实现，使用头文件中的内联定义

// TypeSystem实现
TypeSystem::TypeSystem() {
    initializeBuiltinTypes();
}

void TypeSystem::initializeBuiltinTypes() {
    // 创建内置类型
    intType_ = std::make_shared<PrimitiveType>(TypeKind::Int, "Int");
    floatType_ = std::make_shared<PrimitiveType>(TypeKind::Float, "Float");
    boolType_ = std::make_shared<PrimitiveType>(TypeKind::Bool, "Bool");
    stringType_ = std::make_shared<PrimitiveType>(TypeKind::String, "String");
    voidType_ = std::make_shared<PrimitiveType>(TypeKind::Void, "Void");
    anyType_ = std::make_shared<PrimitiveType>(TypeKind::Any, "Any");
    errorType_ = std::make_shared<PrimitiveType>(TypeKind::Error, "Error");
    
    // 注册内置类型
    builtinTypes_["Int"] = intType_;
    builtinTypes_["Float"] = floatType_;
    builtinTypes_["Bool"] = boolType_;
    builtinTypes_["String"] = stringType_;
    builtinTypes_["Void"] = voidType_;
    builtinTypes_["Any"] = anyType_;
}

std::shared_ptr<Type> TypeSystem::getIntType() const {
    return intType_;
}

std::shared_ptr<Type> TypeSystem::getFloatType() const {
    return floatType_;
}

std::shared_ptr<Type> TypeSystem::getBoolType() const {
    return boolType_;
}

std::shared_ptr<Type> TypeSystem::getStringType() const {
    return stringType_;
}

std::shared_ptr<Type> TypeSystem::getVoidType() const {
    return voidType_;
}

std::shared_ptr<Type> TypeSystem::getAnyType() const {
    return anyType_;
}

std::shared_ptr<Type> TypeSystem::getErrorType() const {
    return errorType_;
}

std::shared_ptr<Type> TypeSystem::createDictionaryType(std::shared_ptr<Type> keyType, std::shared_ptr<Type> valueType) {
    return getDictionaryType(keyType, valueType);
}

std::shared_ptr<ArrayType> TypeSystem::getArrayType(std::shared_ptr<Type> elementType) {
    // 检查是否已存在相同的数组类型
    for (const auto& arrayType : arrayTypes_) {
        if (arrayType->getElementType()->equals(*elementType)) {
            return arrayType;
        }
    }
    
    // 创建新的数组类型
    auto newArrayType = std::make_shared<ArrayType>(elementType);
    arrayTypes_.push_back(newArrayType);
    return newArrayType;
}

std::shared_ptr<DictionaryType> TypeSystem::getDictionaryType(
    std::shared_ptr<Type> keyType, 
    std::shared_ptr<Type> valueType) {
    
    // 检查是否已存在相同的字典类型
    for (const auto& dictType : dictionaryTypes_) {
        if (dictType->getKeyType()->equals(*keyType) && dictType->getValueType()->equals(*valueType)) {
            return dictType;
        }
    }
    
    // 创建新的字典类型
    auto newDictType = std::make_shared<DictionaryType>(keyType, valueType);
    dictionaryTypes_.push_back(newDictType);
    return newDictType;
}

std::shared_ptr<TupleType> TypeSystem::getTupleType(
    const std::vector<std::shared_ptr<Type>>& elementTypes) {
    
    // 将类型转换为TupleElement
    std::vector<TupleType::TupleElement> elements;
    for (const auto& type : elementTypes) {
        elements.emplace_back(type);
    }
    
    // 检查是否已存在相同的元组类型
    for (const auto& tupleType : tupleTypes_) {
        if (tupleType->getElementCount() == elementTypes.size()) {
            bool matches = true;
            for (size_t i = 0; i < elementTypes.size(); ++i) {
                if (!tupleType->getElementType(i)->equals(*elementTypes[i])) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                return tupleType;
            }
        }
    }
    
    // 创建新的元组类型
    auto newTupleType = std::make_shared<TupleType>(elements);
    tupleTypes_.push_back(newTupleType);
    return newTupleType;
}

std::shared_ptr<OptionalType> TypeSystem::getOptionalType(std::shared_ptr<Type> wrappedType) {
    // 检查是否已存在相同的可选类型
    for (const auto& optionalType : optionalTypes_) {
        if (optionalType->getWrappedType()->equals(*wrappedType)) {
            return optionalType;
        }
    }
    
    // 创建新的可选类型
    auto newOptionalType = std::make_shared<OptionalType>(wrappedType);
    optionalTypes_.push_back(newOptionalType);
    return newOptionalType;
}

std::shared_ptr<FunctionType> TypeSystem::getFunctionType(
    const std::vector<std::shared_ptr<Type>>& parameterTypes,
    std::shared_ptr<Type> returnType) {
    
    // 检查是否已存在相同的函数类型
    for (const auto& funcType : functionTypes_) {
        if (funcType->getReturnType()->equals(*returnType) && 
            funcType->getParameterTypes().size() == parameterTypes.size()) {
            
            bool matches = true;
            for (size_t i = 0; i < parameterTypes.size(); ++i) {
                if (!funcType->getParameterTypes()[i]->equals(*parameterTypes[i])) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                return funcType;
            }
        }
    }
    
    // 创建新的函数类型
    auto newFuncType = std::make_shared<FunctionType>(parameterTypes, returnType, false, false);
    functionTypes_.push_back(newFuncType);
    return newFuncType;
}

std::shared_ptr<Type> TypeSystem::getTypeByName(const std::string& name) const {
    // 查找内置类型
    auto builtinIt = builtinTypes_.find(name);
    if (builtinIt != builtinTypes_.end()) {
        return builtinIt->second;
    }
    
    // 查找用户定义类型
    auto userIt = userDefinedTypes_.find(name);
    if (userIt != userDefinedTypes_.end()) {
        return userIt->second;
    }
    
    return nullptr;
}

void TypeSystem::registerUserDefinedType(std::shared_ptr<UserDefinedType> type) {
    userDefinedTypes_[type->getName()] = type;
}

bool TypeSystem::areTypesEqual(const std::shared_ptr<Type>& type1, const std::shared_ptr<Type>& type2) const {
    if (!type1 || !type2) {
        return type1 == type2;
    }
    return type1->equals(*type2);
}

bool TypeSystem::isSubtype(const std::shared_ptr<Type>& subtype, const std::shared_ptr<Type>& supertype) const {
    if (!subtype || !supertype) {
        return false;
    }
    return subtype->isSubtypeOf(*supertype);
}

bool TypeSystem::isConvertible(const std::shared_ptr<Type>& from, const std::shared_ptr<Type>& to) const {
    if (!from || !to) {
        return false;
    }
    return from->isConvertibleTo(*to);
}

std::shared_ptr<Type> TypeSystem::findCommonType(
    const std::shared_ptr<Type>& type1, 
    const std::shared_ptr<Type>& type2) const {
    
    if (!type1 || !type2) {
        return nullptr;
    }
    
    // 如果类型相同
    if (areTypesEqual(type1, type2)) {
        return type1;
    }
    
    // 如果一个是另一个的子类型
    if (isSubtype(type1, type2)) {
        return type2;
    }
    if (isSubtype(type2, type1)) {
        return type1;
    }
    
    // 数值类型的公共类型
    if (isNumericType(type1) && isNumericType(type2)) {
        if (type1->getKind() == TypeKind::Float || type2->getKind() == TypeKind::Float) {
            return getFloatType();
        } else {
            return getIntType();
        }
    }
    
    // 如果都可以转换为Any
    if (isConvertible(type1, getAnyType()) && isConvertible(type2, getAnyType())) {
        return getAnyType();
    }
    
    return nullptr;
}

bool TypeSystem::isNumericType(std::shared_ptr<Type> type) const {
    if (!type) return false;
    return type->getKind() == TypeKind::Int || type->getKind() == TypeKind::Float;
}

bool TypeSystem::areTypesComparable(std::shared_ptr<Type> type1, std::shared_ptr<Type> type2) const {
    if (!type1 || !type2) {
        return false;
    }
    
    // 相同类型可比较
    if (areTypesEqual(type1, type2)) {
        return true;
    }
    
    // 数值类型之间可比较
    if (isNumericType(type1) && isNumericType(type2)) {
        return true;
    }
    
    // 字符串类型可比较
    if (type1->getKind() == TypeKind::String && type2->getKind() == TypeKind::String) {
        return true;
    }
    
    // 可选类型的比较
    if (type1->getKind() == TypeKind::Optional || type2->getKind() == TypeKind::Optional) {
        // 简化处理：可选类型与其包装类型可比较
        return true;
    }
    
    return false;
}

std::shared_ptr<Type> TypeSystem::inferType(const std::vector<std::shared_ptr<Type>>& candidateTypes) const {
    if (candidateTypes.empty()) {
        return nullptr;
    }
    
    if (candidateTypes.size() == 1) {
        return candidateTypes[0];
    }
    
    // 找到所有类型的公共类型
    std::shared_ptr<Type> commonType = candidateTypes[0];
    for (size_t i = 1; i < candidateTypes.size(); ++i) {
        commonType = findCommonType(commonType, candidateTypes[i]);
        if (!commonType) {
            return nullptr;
        }
    }
    
    return commonType;
}

std::shared_ptr<GenericType> TypeSystem::createGenericType(const std::string& name) {
    auto genericType = std::make_shared<GenericType>(name);
    genericTypes_[name] = genericType;
    return genericType;
}

std::shared_ptr<Type> TypeSystem::instantiateGenericType(
    std::shared_ptr<GenericType> genericType,
    const std::vector<std::shared_ptr<Type>>& typeArguments) {
    
    // 简化实现：直接返回第一个类型参数
    // 实际实现需要更复杂的泛型实例化逻辑
    if (!typeArguments.empty()) {
        return typeArguments[0];
    }
    
    return genericType;
}

bool TypeSystem::checkGenericConstraints(
    std::shared_ptr<GenericType> genericType,
    std::shared_ptr<Type> concreteType) const {
    
    // 获取泛型约束并检查
    std::vector<std::string> constraints; // 这里应该从genericType获取约束
    return satisfiesConstraints(concreteType, constraints);
}

bool TypeSystem::satisfiesConstraints(
    const std::shared_ptr<Type>& type,
    const std::vector<std::string>& constraints) const {
    
    // 简化实现：如果没有约束，则满足
    if (constraints.empty()) {
        return true;
    }
    
    // 这里应该实现具体的约束检查逻辑
    // 例如检查类型是否实现了特定的协议或接口
    for (const auto& constraint : constraints) {
        // 简化处理：假设所有约束都满足
        // 实际实现需要根据约束类型进行具体检查
    }
    
    return true;
}

std::vector<std::string> TypeSystem::getBuiltinTypeNames() const {
    std::vector<std::string> names;
    for (const auto& pair : builtinTypes_) {
        names.push_back(pair.first);
    }
    return names;
}

std::vector<std::string> TypeSystem::getUserDefinedTypeNames() const {
    std::vector<std::string> names;
    for (const auto& pair : userDefinedTypes_) {
        names.push_back(pair.first);
    }
    return names;
}

size_t TypeSystem::getTypeCount() const {
    return builtinTypes_.size() + userDefinedTypes_.size() + 
           arrayTypes_.size() + dictionaryTypes_.size() + 
           tupleTypes_.size() + optionalTypes_.size() + 
           functionTypes_.size() + genericTypes_.size();
}

void TypeSystem::dumpTypes() const {
    std::cout << "Type System Dump:\n";
    
    std::cout << "Builtin Types:\n";
    for (const auto& pair : builtinTypes_) {
        std::cout << "  " << pair.first << " -> " << pair.second->toString() << "\n";
    }
    
    std::cout << "User Defined Types:\n";
    for (const auto& pair : userDefinedTypes_) {
        std::cout << "  " << pair.first << " -> " << pair.second->toString() << "\n";
    }
    
    std::cout << "Array Types: " << arrayTypes_.size() << "\n";
    std::cout << "Dictionary Types: " << dictionaryTypes_.size() << "\n";
    std::cout << "Tuple Types: " << tupleTypes_.size() << "\n";
    std::cout << "Optional Types: " << optionalTypes_.size() << "\n";
    std::cout << "Function Types: " << functionTypes_.size() << "\n";
    std::cout << "Generic Types: " << genericTypes_.size() << "\n";
}

// ============================================================================
// 并发类型系统实现
// ============================================================================

// ActorType 实现
ActorType::ActorType(const std::string& name, bool isGlobalActor)
    : Type(TypeKind::Class, name), isGlobalActor_(isGlobalActor) {
}

void ActorType::addProperty(const std::string& name, std::shared_ptr<Type> type, ActorIsolation isolation) {
    properties_[name] = type;
    propertyIsolations_[name] = isolation;
}

void ActorType::addMethod(const std::string& name, std::shared_ptr<FunctionType> type, ActorIsolation isolation) {
    methods_[name] = type;
    methodIsolations_[name] = isolation;
}

std::shared_ptr<Type> ActorType::getPropertyType(const std::string& name) const {
    auto it = properties_.find(name);
    return (it != properties_.end()) ? it->second : nullptr;
}

std::shared_ptr<FunctionType> ActorType::getMethodType(const std::string& name) const {
    auto it = methods_.find(name);
    return (it != methods_.end()) ? it->second : nullptr;
}

ActorIsolation ActorType::getPropertyIsolation(const std::string& name) const {
    auto it = propertyIsolations_.find(name);
    return (it != propertyIsolations_.end()) ? it->second : ActorIsolation::NONE;
}

ActorIsolation ActorType::getMethodIsolation(const std::string& name) const {
    auto it = methodIsolations_.find(name);
    return (it != methodIsolations_.end()) ? it->second : ActorIsolation::NONE;
}

bool ActorType::hasProperty(const std::string& name) const {
    return properties_.find(name) != properties_.end();
}

bool ActorType::hasMethod(const std::string& name) const {
    return methods_.find(name) != methods_.end();
}

std::vector<std::string> ActorType::getPropertyNames() const {
    std::vector<std::string> names;
    for (const auto& pair : properties_) {
        names.push_back(pair.first);
    }
    return names;
}

std::vector<std::string> ActorType::getMethodNames() const {
    std::vector<std::string> names;
    for (const auto& pair : methods_) {
        names.push_back(pair.first);
    }
    return names;
}

std::string ActorType::toString() const {
    return "actor " + name_;
}

bool ActorType::equals(const Type& other) const {
    if (other.getKind() != TypeKind::Class) return false;
    const ActorType* actorType = dynamic_cast<const ActorType*>(&other);
    return actorType && actorType->name_ == name_ && actorType->isGlobalActor_ == isGlobalActor_;
}

std::shared_ptr<Type> ActorType::cloneImpl() const {
    auto clone = std::make_shared<ActorType>(name_, isGlobalActor_);
    clone->properties_ = properties_;
    clone->methods_ = methods_;
    clone->propertyIsolations_ = propertyIsolations_;
    clone->methodIsolations_ = methodIsolations_;
    return clone;
}

// TaskType 实现
TaskType::TaskType(std::shared_ptr<Type> resultType, TaskPriority priority)
    : Type(TypeKind::Class, "Task"), resultType_(resultType), priority_(priority) {
}

std::string TaskType::toString() const {
    return "Task<" + resultType_->toString() + ">";
}

bool TaskType::equals(const Type& other) const {
    if (other.getKind() != TypeKind::Class) return false;
    const TaskType* taskType = dynamic_cast<const TaskType*>(&other);
    return taskType && taskType->resultType_->equals(*resultType_) && taskType->priority_ == priority_;
}

std::shared_ptr<Type> TaskType::cloneImpl() const {
    return std::make_shared<TaskType>(resultType_->clone(), priority_);
}

// TaskGroupType 实现
TaskGroupType::TaskGroupType(std::shared_ptr<Type> childResultType)
    : Type(TypeKind::Class, "TaskGroup"), childResultType_(childResultType) {
}

std::string TaskGroupType::toString() const {
    return "TaskGroup<" + childResultType_->toString() + ">";
}

bool TaskGroupType::equals(const Type& other) const {
    if (other.getKind() != TypeKind::Class) return false;
    const TaskGroupType* taskGroupType = dynamic_cast<const TaskGroupType*>(&other);
    return taskGroupType && taskGroupType->childResultType_->equals(*childResultType_);
}

std::shared_ptr<Type> TaskGroupType::cloneImpl() const {
    return std::make_shared<TaskGroupType>(childResultType_->clone());
}

// AsyncSequenceType 实现
AsyncSequenceType::AsyncSequenceType(std::shared_ptr<Type> elementType)
    : Type(TypeKind::Protocol, "AsyncSequence"), elementType_(elementType) {
}

std::string AsyncSequenceType::toString() const {
    return "AsyncSequence<" + elementType_->toString() + ">";
}

bool AsyncSequenceType::equals(const Type& other) const {
    if (other.getKind() != TypeKind::Protocol) return false;
    const AsyncSequenceType* asyncSeqType = dynamic_cast<const AsyncSequenceType*>(&other);
    return asyncSeqType && asyncSeqType->elementType_->equals(*elementType_);
}

std::shared_ptr<Type> AsyncSequenceType::cloneImpl() const {
    return std::make_shared<AsyncSequenceType>(elementType_->clone());
}

// AsyncIteratorType 实现
AsyncIteratorType::AsyncIteratorType(std::shared_ptr<Type> elementType)
    : Type(TypeKind::Protocol, "AsyncIterator"), elementType_(elementType) {
}

std::string AsyncIteratorType::toString() const {
    return "AsyncIterator<" + elementType_->toString() + ">";
}

bool AsyncIteratorType::equals(const Type& other) const {
    if (other.getKind() != TypeKind::Protocol) return false;
    const AsyncIteratorType* asyncIterType = dynamic_cast<const AsyncIteratorType*>(&other);
    return asyncIterType && asyncIterType->elementType_->equals(*elementType_);
}

std::shared_ptr<Type> AsyncIteratorType::cloneImpl() const {
    return std::make_shared<AsyncIteratorType>(elementType_->clone());
}

// SendableProtocolType 实现
SendableProtocolType::SendableProtocolType()
    : Type(TypeKind::Protocol, "Sendable") {
}

std::string SendableProtocolType::toString() const {
    return "Sendable";
}

bool SendableProtocolType::equals(const Type& other) const {
    return dynamic_cast<const SendableProtocolType*>(&other) != nullptr;
}

bool SendableProtocolType::conformsToSendable(const std::shared_ptr<Type>& type) {
    // 基本类型都是Sendable的
    TypeKind kind = type->getKind();
    if (kind == TypeKind::Int || kind == TypeKind::Float || kind == TypeKind::Bool || 
        kind == TypeKind::String || kind == TypeKind::Double) {
        return true;
    }
    
    // 检查是否是SendableProtocolType
    if (dynamic_cast<const SendableProtocolType*>(type.get())) {
        return true;
    }
    
    // 其他类型需要显式声明符合Sendable协议
    return false;
}

std::shared_ptr<Type> SendableProtocolType::cloneImpl() const {
    return std::make_shared<SendableProtocolType>();
}

// ContinuationType 实现
ContinuationType::ContinuationType(std::shared_ptr<Type> resultType, bool throwing)
    : Type(TypeKind::Class, "Continuation"), resultType_(resultType), throwing_(throwing) {
}

std::string ContinuationType::toString() const {
    std::string base = throwing_ ? "CheckedContinuation" : "UnsafeContinuation";
    return base + "<" + resultType_->toString() + ">";
}

bool ContinuationType::equals(const Type& other) const {
    if (other.getKind() != TypeKind::Class) return false;
    const ContinuationType* contType = dynamic_cast<const ContinuationType*>(&other);
    return contType && contType->resultType_->equals(*resultType_) && contType->throwing_ == throwing_;
}

std::shared_ptr<Type> ContinuationType::cloneImpl() const {
    return std::make_shared<ContinuationType>(resultType_->clone(), throwing_);
}

// AsyncStreamType 实现
AsyncStreamType::AsyncStreamType(std::shared_ptr<Type> elementType, bool throwing)
    : Type(TypeKind::Class, "AsyncStream"), elementType_(elementType), throwing_(throwing) {
}

std::string AsyncStreamType::toString() const {
    std::string base = throwing_ ? "AsyncThrowingStream" : "AsyncStream";
    return base + "<" + elementType_->toString() + ">";
}

bool AsyncStreamType::equals(const Type& other) const {
    if (other.getKind() != getKind()) return false;
    const AsyncStreamType* streamType = dynamic_cast<const AsyncStreamType*>(&other);
    return streamType && streamType->elementType_->equals(*elementType_) && streamType->throwing_ == throwing_;
}

std::shared_ptr<Type> AsyncStreamType::cloneImpl() const {
    return std::make_shared<AsyncStreamType>(elementType_->clone(), throwing_);
}

// ConcurrencyTypeChecker 实现
ConcurrencyTypeChecker::ConcurrencyTypeChecker(std::shared_ptr<TypeSystem> typeSystem)
    : typeSystem_(typeSystem) {
    initializeBuiltinSendableTypes();
}

bool ConcurrencyTypeChecker::validateAsyncCall(const std::shared_ptr<FunctionType>& funcType, 
                                              const std::vector<std::shared_ptr<Type>>& argTypes,
                                              bool inAsyncContext) {
    if (!funcType->isAsync && !inAsyncContext) {
        return true; // 同步调用在任何上下文都可以
    }
    
    if (funcType->isAsync && !inAsyncContext) {
        addDiagnostic("Async function call requires async context");
        return false;
    }
    
    // 检查参数类型是否符合Sendable约束
    for (size_t i = 0; i < argTypes.size() && i < funcType->parameterTypes.size(); ++i) {
        if (!validateSendableConstraint(argTypes[i], "function parameter")) {
            return false;
        }
    }
    
    return true;
}

bool ConcurrencyTypeChecker::validateActorAccess(const std::shared_ptr<ActorType>& actorType,
                                                const std::string& memberName,
                                                const std::string& currentActorContext,
                                                bool inAsyncContext) {
    if (!actorType) {
        addDiagnostic("Invalid actor type");
        return false;
    }
    
    ActorIsolation memberIsolation = ActorIsolation::NONE;
    
    if (actorType->hasProperty(memberName)) {
        memberIsolation = actorType->getPropertyIsolation(memberName);
    } else if (actorType->hasMethod(memberName)) {
        memberIsolation = actorType->getMethodIsolation(memberName);
    } else {
        addDiagnostic("Member '" + memberName + "' not found in actor '" + actorType->getName() + "'");
        return false;
    }
    
    return validateActorIsolationAccess(actorType, memberName, memberIsolation);
}

bool ConcurrencyTypeChecker::validateSendableConstraint(const std::shared_ptr<Type>& type,
                                                       const std::string& context) {
    if (!type) {
        addDiagnostic("Invalid type in " + context);
        return false;
    }
    
    if (isSendableType(type)) {
        return true;
    }
    
    addDiagnostic("Type '" + type->toString() + "' does not conform to Sendable protocol in " + context);
    return false;
}

bool ConcurrencyTypeChecker::validateTaskGroupOperation(const std::shared_ptr<TaskGroupType>& taskGroupType,
                                                       const std::string& operation,
                                                       const std::vector<std::shared_ptr<Type>>& operandTypes) {
    if (!taskGroupType) {
        addDiagnostic("Invalid TaskGroup type");
        return false;
    }
    
    if (operation == "addTask") {
        // 检查任务返回类型是否与TaskGroup的子结果类型兼容
        if (!operandTypes.empty()) {
            auto taskResultType = operandTypes[0];
            if (!typeSystem_->areTypesEqual(taskResultType, taskGroupType->getChildResultType())) {
                addDiagnostic("Task result type does not match TaskGroup child result type");
                return false;
            }
        }
    }
    
    return true;
}

bool ConcurrencyTypeChecker::validateAsyncSequenceIteration(const std::shared_ptr<AsyncSequenceType>& seqType,
                                                           bool inAsyncContext) {
    if (!seqType) {
        addDiagnostic("Invalid AsyncSequence type");
        return false;
    }
    
    if (!inAsyncContext) {
        addDiagnostic("AsyncSequence iteration requires async context");
        return false;
    }
    
    return validateSendableConstraint(seqType->getElementType(), "AsyncSequence element");
}

// 类型创建方法
std::shared_ptr<ActorType> ConcurrencyTypeChecker::createActorType(const std::string& name, bool isGlobalActor) {
    auto actorType = std::make_shared<ActorType>(name, isGlobalActor);
    actorTypes_[name] = actorType;
    return actorType;
}

std::shared_ptr<TaskType> ConcurrencyTypeChecker::createTaskType(std::shared_ptr<Type> resultType, TaskPriority priority) {
    return std::make_shared<TaskType>(resultType, priority);
}

std::shared_ptr<TaskGroupType> ConcurrencyTypeChecker::createTaskGroupType(std::shared_ptr<Type> childResultType) {
    return std::make_shared<TaskGroupType>(childResultType);
}

std::shared_ptr<AsyncSequenceType> ConcurrencyTypeChecker::createAsyncSequenceType(std::shared_ptr<Type> elementType) {
    return std::make_shared<AsyncSequenceType>(elementType);
}

std::shared_ptr<AsyncIteratorType> ConcurrencyTypeChecker::createAsyncIteratorType(std::shared_ptr<Type> elementType) {
    return std::make_shared<AsyncIteratorType>(elementType);
}

std::shared_ptr<ContinuationType> ConcurrencyTypeChecker::createContinuationType(std::shared_ptr<Type> resultType, bool throwing) {
    return std::make_shared<ContinuationType>(resultType, throwing);
}

std::shared_ptr<AsyncStreamType> ConcurrencyTypeChecker::createAsyncStreamType(std::shared_ptr<Type> elementType, bool throwing) {
    return std::make_shared<AsyncStreamType>(elementType, throwing);
}

// 类型查询方法
bool ConcurrencyTypeChecker::isActorType(const std::shared_ptr<Type>& type) const {
    return dynamic_cast<const ActorType*>(type.get()) != nullptr;
}

bool ConcurrencyTypeChecker::isTaskType(const std::shared_ptr<Type>& type) const {
    return dynamic_cast<const TaskType*>(type.get()) != nullptr;
}

bool ConcurrencyTypeChecker::isAsyncSequenceType(const std::shared_ptr<Type>& type) const {
    return dynamic_cast<const AsyncSequenceType*>(type.get()) != nullptr;
}

bool ConcurrencyTypeChecker::isSendableType(const std::shared_ptr<Type>& type) const {
    return SendableProtocolType::conformsToSendable(type) || isBuiltinSendableType(type);
}

ConcurrencySafetyLevel ConcurrencyTypeChecker::getSafetyLevel(const std::shared_ptr<Type>& type) const {
    if (isSendableType(type)) {
        return ConcurrencySafetyLevel::SENDABLE;
    }
    
    if (isActorType(type)) {
        return ConcurrencySafetyLevel::ACTOR_ISOLATED;
    }
    
    return ConcurrencySafetyLevel::UNSAFE;
}

ActorIsolation ConcurrencyTypeChecker::getActorIsolation(const std::shared_ptr<Type>& type) const {
    const ActorType* actorType = dynamic_cast<const ActorType*>(type.get());
    if (actorType) {
        return actorType->isGlobalActor() ? ActorIsolation::GLOBAL_ACTOR : ActorIsolation::ACTOR_ISOLATED;
    }
    
    return ActorIsolation::NONE;
}

bool ConcurrencyTypeChecker::canCrossActorBoundary(const std::shared_ptr<Type>& type) const {
    return isSendableType(type);
}

bool ConcurrencyTypeChecker::requiresAsyncContext(const std::shared_ptr<Type>& type) const {
    return isTaskType(type) || isAsyncSequenceType(type);
}

bool ConcurrencyTypeChecker::isMainActorIsolated(const std::shared_ptr<Type>& type) const {
    const ActorType* actorType = dynamic_cast<const ActorType*>(type.get());
    return actorType && actorType->getName() == "MainActor";
}

// 错误处理
std::vector<ConcurrencyTypeError> ConcurrencyTypeChecker::getDiagnostics() const {
    return diagnostics_;
}

void ConcurrencyTypeChecker::addDiagnostic(const std::string& message, int line, int column, const std::string& errorCode) {
    diagnostics_.emplace_back(message, line, column, errorCode);
}

void ConcurrencyTypeChecker::clearDiagnostics() {
    diagnostics_.clear();
}

bool ConcurrencyTypeChecker::hasErrors() const {
    return !diagnostics_.empty();
}

// 私有辅助方法
void ConcurrencyTypeChecker::initializeBuiltinSendableTypes() {
    builtinSendableTypes_.insert("Int");
    builtinSendableTypes_.insert("Float");
    builtinSendableTypes_.insert("Double");
    builtinSendableTypes_.insert("Bool");
    builtinSendableTypes_.insert("String");
    builtinSendableTypes_.insert("Void");
}

bool ConcurrencyTypeChecker::isBuiltinSendableType(const std::shared_ptr<Type>& type) const {
    return builtinSendableTypes_.find(type->getName()) != builtinSendableTypes_.end();
}

bool ConcurrencyTypeChecker::checkSendableConformance(const std::shared_ptr<Type>& type) const {
    return SendableProtocolType::conformsToSendable(type);
}

bool ConcurrencyTypeChecker::validateActorIsolationAccess(const std::shared_ptr<ActorType>& actorType,
                                                         const std::string& memberName,
                                                         ActorIsolation currentIsolation) const {
    // 简化的隔离检查逻辑
    if (currentIsolation == ActorIsolation::NONISOLATED) {
        return true; // nonisolated成员可以从任何地方访问
    }
    
    // 其他情况需要更复杂的检查
    return true;
}

std::string ConcurrencyTypeChecker::getActorIsolationString(ActorIsolation isolation) const {
    switch (isolation) {
        case ActorIsolation::NONE: return "none";
        case ActorIsolation::ACTOR_ISOLATED: return "actor-isolated";
        case ActorIsolation::MAIN_ACTOR: return "@MainActor";
        case ActorIsolation::GLOBAL_ACTOR: return "@GlobalActor";
        case ActorIsolation::NONISOLATED: return "nonisolated";
        default: return "unknown";
    }
}

std::string ConcurrencyTypeChecker::getTaskPriorityString(TaskPriority priority) const {
    switch (priority) {
        case TaskPriority::Background: return "background";
        case TaskPriority::Utility: return "utility";
        case TaskPriority::Default: return "default";
        case TaskPriority::UserInitiated: return "userInitiated";
        case TaskPriority::UserInteractive: return "userInteractive";
        default: return "unknown";
    }
}

// AsyncContextManager 实现
AsyncContextManager::AsyncContextManager() {
}

void AsyncContextManager::enterAsyncContext(const std::string& contextName) {
    contextStack_.emplace_back("async", contextName);
}

void AsyncContextManager::exitAsyncContext() {
    if (!contextStack_.empty() && contextStack_.back().type == "async") {
        contextStack_.pop_back();
    }
}

bool AsyncContextManager::isInAsyncContext() const {
    return findFrameByType("async") != nullptr;
}

void AsyncContextManager::enterActorContext(const std::string& actorName) {
    contextStack_.emplace_back("actor", actorName);
}

void AsyncContextManager::exitActorContext() {
    if (!contextStack_.empty() && contextStack_.back().type == "actor") {
        contextStack_.pop_back();
    }
}

std::string AsyncContextManager::getCurrentActorContext() const {
    const ContextFrame* frame = findFrameByType("actor");
    return frame ? frame->name : "";
}

bool AsyncContextManager::isInActorContext() const {
    return findFrameByType("actor") != nullptr;
}

void AsyncContextManager::enterTaskGroup(const std::string& groupId) {
    contextStack_.emplace_back("taskgroup", groupId);
}

void AsyncContextManager::exitTaskGroup() {
    if (!contextStack_.empty() && contextStack_.back().type == "taskgroup") {
        contextStack_.pop_back();
    }
}

bool AsyncContextManager::isInTaskGroup() const {
    return findFrameByType("taskgroup") != nullptr;
}

std::string AsyncContextManager::getCurrentTaskGroup() const {
    const ContextFrame* frame = findFrameByType("taskgroup");
    return frame ? frame->name : "";
}

void AsyncContextManager::addTaskToGroup(const std::string& taskId) {
    ContextFrame* frame = findFrameByType("taskgroup");
    if (frame) {
        frame->tasks.push_back(taskId);
    }
}

void AsyncContextManager::removeTaskFromGroup(const std::string& taskId) {
    ContextFrame* frame = findFrameByType("taskgroup");
    if (frame) {
        auto it = std::find(frame->tasks.begin(), frame->tasks.end(), taskId);
        if (it != frame->tasks.end()) {
            frame->tasks.erase(it);
        }
    }
}

std::vector<std::string> AsyncContextManager::getTasksInCurrentGroup() const {
    const ContextFrame* frame = findFrameByType("taskgroup");
    return frame ? frame->tasks : std::vector<std::string>();
}

int AsyncContextManager::getAsyncContextDepth() const {
    int depth = 0;
    for (const auto& frame : contextStack_) {
        if (frame.type == "async") {
            depth++;
        }
    }
    return depth;
}

std::vector<std::string> AsyncContextManager::getContextStack() const {
    std::vector<std::string> stack;
    for (const auto& frame : contextStack_) {
        stack.push_back(frame.type + ":" + frame.name);
    }
    return stack;
}

void AsyncContextManager::reset() {
    contextStack_.clear();
}

AsyncContextManager::ContextFrame* AsyncContextManager::getCurrentFrame() {
    return contextStack_.empty() ? nullptr : &contextStack_.back();
}

const AsyncContextManager::ContextFrame* AsyncContextManager::getCurrentFrame() const {
    return contextStack_.empty() ? nullptr : &contextStack_.back();
}

AsyncContextManager::ContextFrame* AsyncContextManager::findFrameByType(const std::string& type) {
    for (auto it = contextStack_.rbegin(); it != contextStack_.rend(); ++it) {
        if (it->type == type) {
            return &(*it);
        }
    }
    return nullptr;
}

const AsyncContextManager::ContextFrame* AsyncContextManager::findFrameByType(const std::string& type) const {
    for (auto it = contextStack_.rbegin(); it != contextStack_.rend(); ++it) {
        if (it->type == type) {
            return &(*it);
        }
    }
    return nullptr;
}

// ConcurrencyTypeFactory 实现
std::shared_ptr<ActorType> ConcurrencyTypeFactory::createMainActorType() {
    return std::make_shared<ActorType>("MainActor", true);
}

std::shared_ptr<SendableProtocolType> ConcurrencyTypeFactory::createSendableProtocol() {
    return std::make_shared<SendableProtocolType>();
}

std::shared_ptr<TaskType> ConcurrencyTypeFactory::createDetachedTask(std::shared_ptr<Type> resultType, TaskPriority priority) {
    return std::make_shared<TaskType>(resultType, priority);
}

std::shared_ptr<AsyncSequenceType> ConcurrencyTypeFactory::createAsyncSequence(std::shared_ptr<Type> elementType) {
    return std::make_shared<AsyncSequenceType>(elementType);
}

std::shared_ptr<Type> ConcurrencyTypeFactory::createBuiltinAsyncType(const std::string& typeName) {
    if (typeName == "Task") {
        // 创建一个泛型Task类型，这里简化为Void结果类型
        return std::make_shared<TaskType>(std::make_shared<PrimitiveType>(TypeKind::Void, "Void"));
    } else if (typeName == "AsyncSequence") {
        // 创建一个泛型AsyncSequence类型，这里简化为Any元素类型
        return std::make_shared<AsyncSequenceType>(std::make_shared<PrimitiveType>(TypeKind::Any, "Any"));
    }
    
    return nullptr;
}

std::shared_ptr<FunctionType> ConcurrencyTypeFactory::createAsyncFunctionType(
    const std::vector<std::shared_ptr<Type>>& paramTypes,
    std::shared_ptr<Type> returnType,
    bool throwing) {
    
    auto funcType = std::make_shared<FunctionType>(paramTypes, returnType, throwing, true);
    return funcType;
}

} // namespace miniswift