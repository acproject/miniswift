#include "Inheritance.h"
#include "../Interpreter.h"
#include "../Environment.h"
#include "../ErrorHandling.h"
#include <algorithm>
#include <stdexcept>

namespace miniswift {

// ============================================================================
// InheritanceManager Implementation
// ============================================================================

void InheritanceManager::registerClass(const std::string& className, const std::string& superClassName) {
    if (!superClassName.empty()) {
        // 检查循环继承
        if (isSubclass(superClassName, className)) {
            throw std::runtime_error("Circular inheritance detected: " + className + " -> " + superClassName);
        }
        inheritanceMap_[className] = superClassName;
    }
    
    // 初始化类的方法映射
    if (classMethods_.find(className) == classMethods_.end()) {
        classMethods_[className] = {};
    }
}

bool InheritanceManager::hasClass(const std::string& className) const {
    return classMethods_.find(className) != classMethods_.end();
}

std::string InheritanceManager::getSuperclass(const std::string& className) const {
    auto it = inheritanceMap_.find(className);
    return (it != inheritanceMap_.end()) ? it->second : "";
}

bool InheritanceManager::isSubclass(const std::string& subclass, const std::string& superclass) const {
    if (subclass == superclass) {
        return true;
    }
    
    auto it = inheritanceMap_.find(subclass);
    if (it == inheritanceMap_.end()) {
        return false;
    }
    
    return isSubclass(it->second, superclass);
}

std::vector<std::string> InheritanceManager::getInheritanceChain(const std::string& className) const {
    std::vector<std::string> chain;
    std::string current = className;
    
    while (!current.empty()) {
        chain.push_back(current);
        auto it = inheritanceMap_.find(current);
        current = (it != inheritanceMap_.end()) ? it->second : "";
    }
    
    return chain;
}

std::shared_ptr<FunctionStmt> InheritanceManager::findMethod(const std::string& className, 
                                                           const std::string& methodName) const {
    return findMethodRecursive(className, methodName);
}

void InheritanceManager::registerMethod(const std::string& className, 
                                      const std::string& methodName,
                                      std::shared_ptr<FunctionStmt> method) {
    classMethods_[className][methodName] = method;
}

bool InheritanceManager::isMethodOverridden(const std::string& className, 
                                          const std::string& methodName) const {
    // 检查当前类是否有该方法
    auto classIt = classMethods_.find(className);
    if (classIt == classMethods_.end()) {
        return false;
    }
    
    auto methodIt = classIt->second.find(methodName);
    if (methodIt == classIt->second.end()) {
        return false;
    }
    
    // 检查父类是否也有该方法
    std::string superclass = getSuperclass(className);
    if (superclass.empty()) {
        return false;
    }
    
    return findMethodRecursive(superclass, methodName) != nullptr;
}

std::shared_ptr<FunctionStmt> InheritanceManager::getOverriddenMethod(const std::string& className,
                                                                     const std::string& methodName) const {
    std::string superclass = getSuperclass(className);
    if (superclass.empty()) {
        return nullptr;
    }
    
    return findMethodRecursive(superclass, methodName);
}

std::shared_ptr<FunctionStmt> InheritanceManager::findMethodInClass(const std::string& className,
                                                                      const std::string& methodName) const {
    // 只在当前类中查找，不递归查找父类
    auto classIt = classMethods_.find(className);
    if (classIt != classMethods_.end()) {
        auto methodIt = classIt->second.find(methodName);
        if (methodIt != classIt->second.end()) {
            return methodIt->second;
        }
    }
    
    return nullptr;
}

std::shared_ptr<FunctionStmt> InheritanceManager::findMethodRecursive(const std::string& className,
                                                                     const std::string& methodName) const {
    // 在当前类中查找
    auto classIt = classMethods_.find(className);
    if (classIt != classMethods_.end()) {
        auto methodIt = classIt->second.find(methodName);
        if (methodIt != classIt->second.end()) {
            return methodIt->second;
        }
    }
    
    // 在父类中递归查找
    std::string superclass = getSuperclass(className);
    if (!superclass.empty()) {
        return findMethodRecursive(superclass, methodName);
    }
    
    return nullptr;
}

// ============================================================================
// SuperHandler Implementation
// ============================================================================

SuperHandler::SuperHandler(InheritanceManager& inheritanceManager, Interpreter& interpreter)
    : inheritanceManager_(inheritanceManager), interpreter_(interpreter) {}

Value SuperHandler::callSuperMethod(const std::string& currentClass,
                                   const std::string& methodName,
                                   const std::vector<Value>& arguments,
                                   std::shared_ptr<Environment> environment) {
    // 获取父类名称
    std::string superclass = inheritanceManager_.getSuperclass(currentClass);
    if (superclass.empty()) {
        throw std::runtime_error("Class " + currentClass + " has no superclass");
    }
    
    // 在父类中查找方法
    auto method = inheritanceManager_.findMethodRecursive(superclass, methodName);
    if (!method) {
        throw std::runtime_error("Method " + methodName + " not found in superclass " + superclass);
    }
    
    // 创建新的环境用于方法执行
    auto methodEnv = std::make_shared<Environment>(environment);
    
    // 绑定参数
    if (arguments.size() != method->parameters.size()) {
        throw std::runtime_error("Argument count mismatch for super method " + methodName);
    }
    
    for (size_t i = 0; i < arguments.size(); ++i) {
        methodEnv->define(method->parameters[i].name.lexeme, arguments[i]);
    }
    
    // 执行方法体
    try {
        interpreter_.executeWithEnvironment(*method->body, methodEnv);
        return Value(); // void return
    } catch (const ReturnException& returnEx) {
        return *returnEx.value;
    }
}

Value SuperHandler::getSuperProperty(const std::string& currentClass,
                                   const std::string& propertyName,
                                   std::shared_ptr<Environment> environment) {
    // 获取父类名称
    std::string superclass = inheritanceManager_.getSuperclass(currentClass);
    if (superclass.empty()) {
        throw std::runtime_error("Class " + currentClass + " has no superclass");
    }
    
    // 在父类中查找方法
    auto method = inheritanceManager_.findMethodRecursive(superclass, propertyName);
    if (method) {
        // 创建一个可调用的函数对象
        auto callable = std::make_shared<Function>(method.get(), environment);
        return Value(callable);
    }
    
    // 如果不是方法，可能是属性（暂时不支持）
    throw std::runtime_error("Property '" + propertyName + "' not found in superclass " + superclass);
}

void SuperHandler::setSuperProperty(const std::string& currentClass,
                                  const std::string& propertyName,
                                  const Value& value,
                                  std::shared_ptr<Environment> environment) {
    // 获取父类名称
    std::string superclass = inheritanceManager_.getSuperclass(currentClass);
    if (superclass.empty()) {
        throw std::runtime_error("Class " + currentClass + " has no superclass");
    }
    
    // 在环境中设置父类的属性
    // 这里需要根据具体的属性存储机制来实现
}

// ============================================================================
// OverrideValidator Implementation
// ============================================================================

OverrideValidator::OverrideValidator(InheritanceManager& inheritanceManager)
    : inheritanceManager_(inheritanceManager) {}

bool OverrideValidator::validateOverride(const std::string& className,
                                       const std::string& methodName,
                                       const std::vector<Parameter>& parameters,
                                       const Token& returnType) const {
    // 检查是否为final方法
    if (isFinalMethod(className, methodName)) {
        return false;
    }
    
    // 获取父类中的方法
    std::string superclass = inheritanceManager_.getSuperclass(className);
    if (superclass.empty()) {
        return true; // 没有父类，不存在重写
    }
    
    auto baseMethod = inheritanceManager_.findMethodRecursive(superclass, methodName);
    if (!baseMethod) {
        return true; // 父类中没有该方法，不存在重写
    }
    
    // 检查方法签名是否兼容
    return isSignatureCompatible(parameters, returnType, 
                               baseMethod->parameters, baseMethod->returnType);
}

bool OverrideValidator::isSignatureCompatible(const std::vector<Parameter>& overrideParams,
                                            const Token& overrideReturnType,
                                            const std::vector<Parameter>& baseParams,
                                            const Token& baseReturnType) const {
    // 检查参数数量
    if (overrideParams.size() != baseParams.size()) {
        return false;
    }
    
    // 检查参数类型
    for (size_t i = 0; i < overrideParams.size(); ++i) {
        if (overrideParams[i].type.lexeme != baseParams[i].type.lexeme) {
            return false;
        }
    }
    
    // 检查返回类型（协变返回类型支持可以在这里实现）
    if (overrideReturnType.lexeme != baseReturnType.lexeme) {
        return false;
    }
    
    return true;
}

bool OverrideValidator::isFinalMethod(const std::string& className,
                                    const std::string& methodName) const {
    auto classIt = finalMethods_.find(className);
    if (classIt == finalMethods_.end()) {
        return false;
    }
    
    return classIt->second.find(methodName) != classIt->second.end();
}

// ============================================================================
// ClassInstanceManager Implementation
// ============================================================================

ClassInstanceManager::ClassInstanceManager(InheritanceManager& inheritanceManager)
    : inheritanceManager_(inheritanceManager) {}

std::shared_ptr<ClassInstance> ClassInstanceManager::createInstance(const std::string& className,
                                                                   const std::vector<Value>& initArgs) {
    if (!inheritanceManager_.hasClass(className)) {
        throw std::runtime_error("Class " + className + " not found");
    }
    
    // 创建类实例
    auto instance = std::make_shared<ClassInstance>(className);
    
    // 初始化继承链中的所有属性
    auto inheritanceChain = inheritanceManager_.getInheritanceChain(className);
    for (const auto& ancestorClass : inheritanceChain) {
        // 这里需要根据具体的属性初始化机制来实现
        // 暂时跳过
    }
    
    return instance;
}

bool ClassInstanceManager::isInstanceOf(std::shared_ptr<ClassInstance> instance,
                                      const std::string& className) const {
    if (!instance) {
        return false;
    }
    
    return inheritanceManager_.isSubclass(instance->getClassName(), className);
}

bool ClassInstanceManager::canCastTo(std::shared_ptr<ClassInstance> instance,
                                   const std::string& targetClassName) const {
    if (!instance) {
        return false;
    }
    
    // 向上转型总是安全的
    if (inheritanceManager_.isSubclass(instance->getClassName(), targetClassName)) {
        return true;
    }
    
    // 向下转型需要运行时检查
    if (inheritanceManager_.isSubclass(targetClassName, instance->getClassName())) {
        return isInstanceOf(instance, targetClassName);
    }
    
    return false;
}

std::shared_ptr<ClassInstance> ClassInstanceManager::castTo(std::shared_ptr<ClassInstance> instance,
                                                          const std::string& targetClassName) const {
    if (!canCastTo(instance, targetClassName)) {
        throw std::runtime_error("Cannot cast instance to " + targetClassName);
    }
    
    // 对于向上转型，返回原实例（类型信息在运行时维护）
    // 对于向下转型，也返回原实例，但需要确保类型安全
    return instance;
}

} // namespace miniswift