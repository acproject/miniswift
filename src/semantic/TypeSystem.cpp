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
    auto newFuncType = std::make_shared<FunctionType>(parameterTypes, returnType);
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

} // namespace miniswift