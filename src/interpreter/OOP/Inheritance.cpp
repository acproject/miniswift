#include "Inheritance.h"
#include "../Interpreter.h"
#include "../Environment.h"
#include "../ErrorHandling.h"
#include "Method.h"
#include "Property.h"
#include <algorithm>
#include <stdexcept>
#include <iostream>

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
    
    // 从当前环境中获取 self 对象
    Value selfObject;
    try {
        selfObject = environment->get(Token{TokenType::Identifier, "self", 0});
    } catch (const std::runtime_error&) {
        throw std::runtime_error("Cannot call super method without self context");
    }
    
    // 创建方法调用环境，自动绑定 self
    auto methodEnv = std::make_shared<MethodCallEnvironment>(environment, selfObject, &interpreter_);
    
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
    
    // 首先尝试从self对象的属性容器中获取属性
    try {
        Value selfObject = environment->get(Token{TokenType::Identifier, "self", 0});
        std::cout << "DEBUG: Found self object, trying to access property: " << propertyName << std::endl;
        if (selfObject.isClass()) {
            auto& classInstance = selfObject.asClass();
            std::cout << "DEBUG: Self is class instance of: " << classInstance->getClassName() << std::endl;
            if (classInstance->properties) {
                std::cout << "DEBUG: Properties container exists" << std::endl;
                 
                 // 列出所有属性
                 auto allProps = classInstance->properties->getAllPropertyNames();
                 std::cout << "DEBUG: All properties in container: ";
                 for (const auto& prop : allProps) {
                     std::cout << prop << " ";
                 }
                 std::cout << std::endl;
                 
                 bool hasProperty = classInstance->properties->hasProperty(propertyName);
                 std::cout << "DEBUG: hasProperty(" << propertyName << ") = " << hasProperty << std::endl;
                if (hasProperty) {
                    // 检查这个属性是否定义在父类中
                    auto* classPropManager = interpreter_.getClassPropertyManager(superclass);
                    std::cout << "DEBUG: Checking superclass property manager for: " << superclass << std::endl;
                    if (classPropManager) {
                        auto* propDef = classPropManager->getProperty(propertyName);
                        std::cout << "DEBUG: Property definition in superclass: " << (propDef ? "found" : "not found") << std::endl;
                        if (propDef) {
                            // 属性确实定义在父类中，返回属性值
                            std::cout << "DEBUG: Returning property value from instance" << std::endl;
                            return classInstance->properties->getProperty(interpreter_, propertyName);
                        }
                    } else {
                        std::cout << "DEBUG: No property manager for superclass: " << superclass << std::endl;
                    }
                } else {
                    std::cout << "DEBUG: Property not found in instance properties" << std::endl;
                }
            } else {
                std::cout << "DEBUG: No properties container" << std::endl;
            }
        }
    } catch (const std::runtime_error& e) {
        std::cout << "DEBUG: Exception in getSuperProperty: " << e.what() << std::endl;
        // 如果获取self失败，继续尝试其他方式
    }
    
    // 在父类中查找方法
    auto method = inheritanceManager_.findMethodRecursive(superclass, propertyName);
    if (method) {
        // 创建一个包含当前环境（包括self）的函数对象
        // 这样当函数被调用时，self 对象会被正确传递
        auto callable = std::make_shared<Function>(method.get(), environment);
        return Value(callable);
    }
    
    // 如果既不是属性也不是方法
    throw std::runtime_error("Failed to access super member '" + propertyName + "': Property '" + propertyName + "' not found in superclass " + superclass);
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
    
    // 尝试从self对象的属性容器中设置属性
    try {
        Value selfObject = environment->get(Token{TokenType::Identifier, "self", 0});
        if (selfObject.isClass()) {
            auto& classInstance = selfObject.asClass();
            if (classInstance->properties && classInstance->properties->hasProperty(propertyName)) {
                // 检查这个属性是否定义在父类中
                auto* classPropManager = interpreter_.getClassPropertyManager(superclass);
                if (classPropManager && classPropManager->getProperty(propertyName)) {
                    // 属性确实定义在父类中，设置属性值
                    classInstance->properties->setProperty(interpreter_, propertyName, value);
                    return;
                }
            }
        }
    } catch (const std::runtime_error&) {
        // 如果获取self失败，抛出异常
    }
    
    throw std::runtime_error("Failed to set super property '" + propertyName + "': Property not found in superclass " + superclass);
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

ClassInstanceManager::ClassInstanceManager(InheritanceManager& inheritanceManager, Interpreter& interpreter)
    : inheritanceManager_(inheritanceManager), interpreter_(interpreter) {}

std::shared_ptr<ClassInstance> ClassInstanceManager::createInstance(const std::string& className,
                                                                   const std::vector<Value>& initArgs) {
    if (!inheritanceManager_.hasClass(className)) {
        throw std::runtime_error("Class " + className + " not found");
    }
    
    // 创建类实例
    auto instance = std::make_shared<ClassInstance>(className);
    
    // 创建支持继承的属性容器
    // 获取类的属性管理器
    auto* classPropManager = interpreter_.getClassPropertyManager(className);
    if (classPropManager) {
        // 使用支持继承的构造函数创建属性容器
        instance->properties = std::make_unique<InstancePropertyContainer>(
            *classPropManager, interpreter_.getCurrentEnvironment(), interpreter_, className);
        
        // 初始化所有属性的默认值（包括继承的属性）
        instance->properties->initializeDefaultsWithInheritance(interpreter_, className);
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