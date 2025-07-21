#include "MethodInterpreter.h"
#include "../Environment.h"
#include "../../lexer/Token.h"
#include "../../parser/AST.h"
#include <stdexcept>
#include <iostream>

namespace miniswift {

// Define RuntimeError as an alias for std::runtime_error
using RuntimeError = std::runtime_error;

// 注册结构体方法
void MethodInterpreter::registerStructMethods(const std::string& structName, const std::vector<std::unique_ptr<FunctionStmt>>& methods) {
    auto manager = getStructMethodManager(structName);
    if (!manager) {
        structMethods_[structName] = std::make_unique<MethodManager>();
        manager = structMethods_[structName].get();
    }
    
    for (const auto& method : methods) {
        auto methodDef = createMethodDefinition(*method);
        manager->addMethod(std::move(methodDef));
    }
}

// 注册类方法
void MethodInterpreter::registerClassMethods(const std::string& className, const std::vector<std::unique_ptr<FunctionStmt>>& methods) {
    std::cout << "Registering " << methods.size() << " methods for class '" << className << "'" << std::endl;
    auto manager = getClassMethodManager(className);
    if (!manager) {
        classMethods_[className] = std::make_unique<MethodManager>();
        manager = classMethods_[className].get();
    }
    
    for (const auto& method : methods) {
        std::cout << "Registering method: " << method->name.lexeme << std::endl;
        auto methodDef = createMethodDefinition(*method);
        manager->addMethod(std::move(methodDef));
    }
}

// 获取结构体方法
const MethodDefinition* MethodInterpreter::getStructMethod(const std::string& structName, const std::string& methodName) const {
    const auto* manager = getStructMethodManager(structName);
    if (!manager) {
        return nullptr;
    }
    return manager->getMethod(methodName);
}

// 获取类方法
const MethodDefinition* MethodInterpreter::getClassMethod(const std::string& className, const std::string& methodName) const {
    std::cout << "getClassMethod called for class '" << className << "', method '" << methodName << "'" << std::endl;
    const auto* manager = getClassMethodManager(className);
    if (!manager) {
        std::cout << "No method manager found for class '" << className << "'" << std::endl;
        return nullptr;
    }
    const auto* result = manager->getMethod(methodName);
    std::cout << "getMethod returned: " << (result ? "found" : "not found") << std::endl;
    return result;
}

// 重写 getMemberValue 以支持方法调用
Value MethodInterpreter::getMemberValue(const Value& object, const std::string& memberName) {
    // 查找方法
    if (object.isStruct()) {
        const auto& structVal = object.asStruct();
        const auto* methodDef = getStructMethod(structVal.structName, memberName);
        if (methodDef) {
            // 创建绑定了 self 的方法
            auto methodManager = getStructMethodManager(structVal.structName);
            if (methodManager) {
                auto methodInstance = methodManager->createMethodInstance(memberName, globals);
                if (methodInstance) {
                    // 返回绑定了 self 的 Callable
                    std::vector<Token> params;
                    for (const auto& param : methodDef->parameters) {
                        params.push_back(param);
                    }
                    
                    auto closureEnv = std::make_shared<MethodCallEnvironment>(globals, object);
                    
                    // 创建 FunctionStmt 来包装方法
                    Token emptyReturnType{TokenType::Identifier, "", 0};
                    Token emptyType{TokenType::Identifier, "", 0};
                    
                    std::vector<Parameter> parameters;
                    for (const auto& param : params) {
                        parameters.emplace_back(param, emptyType);
                    }
                    
                    auto funcStmt = std::make_shared<FunctionStmt>(
                        methodDef->name,
                        parameters,
                        emptyReturnType,
                        std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(methodDef->body->clone().release()))
                    );
                    
                    auto callable = std::make_shared<Callable>(funcStmt.get(), closureEnv);
                    return Value(callable);
                }
            }
        }
    } else if (object.isClass()) {
        const auto& classVal = *object.asClass();
        std::cout << "Looking for method '" << memberName << "' in class '" << classVal.className << "'" << std::endl;
        const auto* methodDef = getClassMethod(classVal.className, memberName);
        if (methodDef) {
            std::cout << "Found method '" << memberName << "' in class '" << classVal.className << "'" << std::endl;
            // 创建绑定了 self 的方法
            auto methodManager = getClassMethodManager(classVal.className);
            if (methodManager) {
                auto methodInstance = methodManager->createMethodInstance(memberName, globals);
                if (methodInstance) {
                    // 返回绑定了 self 的 Callable
                    std::vector<Token> params;
                    for (const auto& param : methodDef->parameters) {
                        params.push_back(param);
                    }
                    
                    auto closureEnv = std::make_shared<MethodCallEnvironment>(globals, object);
                    
                    // 创建 FunctionStmt 来包装方法
                    Token emptyReturnType{TokenType::Identifier, "", 0};
                    Token emptyType{TokenType::Identifier, "", 0};
                    
                    std::vector<Parameter> parameters;
                    for (const auto& param : params) {
                        parameters.emplace_back(param, emptyType);
                    }
                    
                    auto funcStmt = std::make_shared<FunctionStmt>(
                        methodDef->name,
                        parameters,
                        emptyReturnType,
                        std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(methodDef->body->clone().release()))
                    );
                    
                    auto callable = std::make_shared<Callable>(funcStmt.get(), closureEnv);
                    return Value(callable);
                }
            }
        }
    }
    
    // 如果没有找到方法，尝试查找属性
    if (object.isStruct()) {
        const auto& structVal = object.asStruct();
        // 首先尝试属性系统
        if (structVal.properties) {
            if (structVal.properties->hasProperty(memberName)) {
                return structVal.properties->getProperty(*this, memberName);
            }
        }
        
        // 然后尝试 legacy 成员访问
        if (structVal.members) {
            std::cout << "Legacy members container exists" << std::endl;
            if (structVal.members->find(memberName) != structVal.members->end()) {
                std::cout << "Property found in legacy members" << std::endl;
                return structVal.members->at(memberName);
            } else {
                std::cout << "Property not found in legacy members" << std::endl;
            }
        } else {
            std::cout << "No legacy members container" << std::endl;
        }
    } else if (object.isClass()) {
        const auto& classVal = *object.asClass();
        
        // 首先尝试属性系统
        if (classVal.properties && classVal.properties->hasProperty(memberName)) {
            return classVal.properties->getProperty(*this, memberName);
        }
        
        // 然后尝试 legacy 成员访问
        if (classVal.members && classVal.members->find(memberName) != classVal.members->end()) {
            return classVal.members->at(memberName);
        }
    }
    
    // 如果既不是属性也不是方法，抛出异常
    throw RuntimeError("Undefined property or method '" + memberName + "'");
}

// 调用结构体方法
Value MethodInterpreter::callStructMethod(const std::string& structName, const std::string& methodName, 
                                        const std::vector<Value>& arguments, const Value& selfValue) {
    auto manager = getStructMethodManager(structName);
    if (!manager) {
        throw RuntimeError("Undefined struct '" + structName + "'");
    }
    
    auto methodInstance = manager->createMethodInstance(methodName, globals);
    if (!methodInstance) {
        throw RuntimeError("Undefined method '" + methodName + "' in struct '" + structName + "'");
    }
    
    return methodInstance->call(*this, arguments, &selfValue);
}

// 调用类方法
Value MethodInterpreter::callClassMethod(const std::string& className, const std::string& methodName, 
                                       const std::vector<Value>& arguments, const Value& selfValue) {
    auto manager = getClassMethodManager(className);
    if (!manager) {
        throw RuntimeError("Undefined class '" + className + "'");
    }
    
    auto methodInstance = manager->createMethodInstance(methodName, globals);
    if (!methodInstance) {
        throw RuntimeError("Undefined method '" + methodName + "' in class '" + className + "'");
    }
    
    return methodInstance->call(*this, arguments, &selfValue);
}

// 检查是否存在结构体方法
bool MethodInterpreter::hasStructMethod(const std::string& structName, const std::string& methodName) const {
    return getStructMethod(structName, methodName) != nullptr;
}

// 检查是否存在类方法
bool MethodInterpreter::hasClassMethod(const std::string& className, const std::string& methodName) const {
    return getClassMethod(className, methodName) != nullptr;
}

// 辅助方法：从 FunctionStmt 创建 MethodDefinition
MethodDefinition MethodInterpreter::createMethodDefinition(const FunctionStmt& funcStmt) {
    MethodDefinition methodDef(funcStmt.name, MethodType::INSTANCE, false);
    
    // 复制参数
    for (const auto& param : funcStmt.parameters) {
        methodDef.parameters.push_back(param.name);
    }
    
    // 复制方法体
    methodDef.body = std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(funcStmt.body->clone().release()));
    
    return methodDef;
}

// 辅助方法：获取方法管理器
MethodManager* MethodInterpreter::getStructMethodManager(const std::string& structName) {
    auto it = structMethods_.find(structName);
    return (it != structMethods_.end()) ? it->second.get() : nullptr;
}

MethodManager* MethodInterpreter::getClassMethodManager(const std::string& className) {
    auto it = classMethods_.find(className);
    return (it != classMethods_.end()) ? it->second.get() : nullptr;
}

const MethodManager* MethodInterpreter::getStructMethodManager(const std::string& structName) const {
    auto it = structMethods_.find(structName);
    return (it != structMethods_.end()) ? it->second.get() : nullptr;
}

const MethodManager* MethodInterpreter::getClassMethodManager(const std::string& className) const {
    auto it = classMethods_.find(className);
    return (it != classMethods_.end()) ? it->second.get() : nullptr;
}

// 重写 visit 方法以注册类方法
void MethodInterpreter::visit(const ClassStmt& stmt) {
    // 首先调用父类的 visit 方法来处理基本的类注册
    Interpreter::visit(stmt);
    
    // 然后注册类方法
    registerClassMethods(stmt.name.lexeme, stmt.methods);
}

// 重写 Range 表达式的 visit 方法
void MethodInterpreter::visit(const Range& expr) {
    // 直接调用父类的实现
    Interpreter::visit(expr);
}

// 重写 TupleLiteral 表达式的 visit 方法
void MethodInterpreter::visit(const TupleLiteral& expr) {
    // 直接调用父类的实现
    Interpreter::visit(expr);
}

} // namespace miniswift