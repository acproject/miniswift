#include "Method.h"
#include "../Interpreter.h"
#include "../Environment.h"
#include "../../lexer/Token.h"
#include "../../parser/AST.h"
#include <stdexcept>
#include <iostream>

namespace miniswift {

// MethodValue 实现

Value MethodValue::call(Interpreter& interpreter, const std::vector<Value>& arguments, const Value* selfValue) {
    // 检查参数数量 - 对于实例方法，如果有参数，第一个参数是self，所以实际用户参数数量应该是parameters.size() - 1
    // 但是如果没有参数，那么用户参数数量就是0
    size_t expectedArgs;
    if (definition_.isStatic) {
        expectedArgs = definition_.parameters.size();
    } else {
        // 对于实例方法，如果有参数且第一个是self，则用户参数数量是parameters.size() - 1
        // 如果没有参数，则用户参数数量是0
        expectedArgs = definition_.parameters.empty() ? 0 : definition_.parameters.size() - 1;
    }
    
    if (arguments.size() != expectedArgs) {
        throw std::runtime_error("Expected " + std::to_string(expectedArgs) + 
                          " arguments but got " + std::to_string(arguments.size()));
    }
    
    // 创建方法调用环境
    std::shared_ptr<Environment> methodEnv;
    
    if (definition_.isStatic) {
        // 静态方法不需要 self
        methodEnv = std::make_shared<Environment>(environment_);
    } else {
        // 实例方法需要 self
        if (!selfValue) {
            throw std::runtime_error("Instance method called without self value");
        }
        methodEnv = std::make_shared<MethodCallEnvironment>(environment_, *selfValue);
    }
    
    // 绑定参数
    if (!definition_.isStatic) {
        // 对于实例方法，self已经通过MethodCallEnvironment处理
        // 直接绑定所有用户参数到对应的参数名
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i < definition_.parameters.size()) {
                methodEnv->define(definition_.parameters[i].lexeme, arguments[i]);
            }
        }
    } else {
        // 对于静态方法，直接绑定所有参数
        for (size_t i = 0; i < definition_.parameters.size(); ++i) {
            methodEnv->define(definition_.parameters[i].lexeme, arguments[i]);
        }
    }
    
    // 执行方法体
    try {
        interpreter.executeWithEnvironment(*definition_.body, methodEnv);
        return Value(); // 没有返回值
    } catch (const std::runtime_error& e) {
        // 检查是否是 ReturnException
        std::string errorMsg = e.what();
        if (errorMsg == "return") {
            // ReturnException 已经被处理，返回值存储在环境中
            if (methodEnv->exists("return")) {
                return methodEnv->get(Token{TokenType::Identifier, "return", 0});
            }
        }
        throw; // 其他异常直接抛出
    }
}

// MethodManager 实现

void MethodManager::addMethod(MethodDefinition&& method) {
    methods_.push_back(std::move(method));
}

const MethodDefinition* MethodManager::getMethod(const std::string& name) const {
    for (const auto& method : methods_) {
        if (method.name.lexeme == name) {
            return &method;
        }
    }
    return nullptr;
}

std::unique_ptr<MethodValue> MethodManager::createMethodInstance(
    const std::string& name, 
    std::shared_ptr<Environment> env
) const {
    const auto* methodDef = getMethod(name);
    if (!methodDef) {
        return nullptr;
    }
    
    return std::make_unique<MethodValue>(*methodDef, env);
}

// InstanceMethodContainer 实现

InstanceMethodContainer::InstanceMethodContainer(const MethodManager& manager, std::shared_ptr<Environment> env)
    : environment_(env) {
    // 为所有方法创建实例
    for (const auto& methodDef : manager.getAllMethods()) {
        auto methodValue = std::make_unique<MethodValue>(methodDef, env);
        methods_[methodDef.name.lexeme] = std::move(methodValue);
    }
}

// Copy constructor
InstanceMethodContainer::InstanceMethodContainer(const InstanceMethodContainer& other)
    : environment_(other.environment_) {
    // Copy all methods
    for (const auto& [name, methodValue] : other.methods_) {
        auto newMethodValue = std::make_unique<MethodValue>(*methodValue);
        methods_[name] = std::move(newMethodValue);
    }
}

Value InstanceMethodContainer::callMethod(Interpreter& interpreter, const std::string& name, 
                                        const std::vector<Value>& arguments, const Value& selfValue) {
    auto it = methods_.find(name);
    if (it == methods_.end()) {
        throw std::runtime_error("Undefined method '" + name + "'");
    }
    
    return it->second->call(interpreter, arguments, &selfValue);
}

Value InstanceMethodContainer::getMethod(const std::string& name, const Value& selfValue) {
    auto it = methods_.find(name);
    if (it == methods_.end()) {
        throw std::runtime_error("Undefined method '" + name + "'");
    }
    
    // 创建一个绑定了 self 的 Callable
    const auto& methodDef = it->second->getDefinition();
    
    // 创建一个闭包环境，包含 self
    auto closureEnv = std::make_shared<MethodCallEnvironment>(environment_, selfValue);
    
    // 创建一个 FunctionStmt 来包装方法
      Token emptyReturnType{TokenType::Identifier, "", 0}; // 空返回类型
      Token emptyType{TokenType::Identifier, "", 0}; // 空类型
      
      // 将 Token 参数转换为 Parameter
      std::vector<Parameter> parameters;
      for (const auto& param : methodDef.parameters) {
          parameters.emplace_back(param, emptyType);
      }
      
      auto funcStmt = new FunctionStmt(
          methodDef.name,
          parameters,
          emptyReturnType,
          std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(methodDef.body->clone().release()))
      );
    
    // 返回 Callable 值
    auto callable = std::make_shared<Callable>(funcStmt, closureEnv);
    return Value(callable);
}

bool InstanceMethodContainer::hasMethod(const std::string& name) const {
    return methods_.find(name) != methods_.end();
}

const MethodDefinition* InstanceMethodContainer::getMethodDefinition(const std::string& name) const {
    auto it = methods_.find(name);
    if (it == methods_.end()) {
        return nullptr;
    }
    return &it->second->getDefinition();
}

} // namespace miniswift