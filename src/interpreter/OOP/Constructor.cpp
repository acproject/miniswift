#include "Constructor.h"
#include "../Interpreter.h"
#include "Property.h"
#include "Method.h"
#include <iostream>
#include <stdexcept>

namespace miniswift {

// ConstructorValue 实现
Value ConstructorValue::call(Interpreter& interpreter, const std::vector<Value>& arguments,
                           InstancePropertyContainer* properties, InstanceMethodContainer* methods) {
    // 检查参数数量
    if (arguments.size() != definition_.parameters.size()) {
        throw std::runtime_error("Constructor argument count mismatch");
    }
    
    // 创建构造器调用环境
    auto constructorEnv = std::make_shared<ConstructorCallEnvironment>(
        environment_, properties, methods);
    
    // 绑定参数
    for (size_t i = 0; i < arguments.size(); ++i) {
        constructorEnv->define(definition_.parameters[i].name.lexeme, arguments[i]);
    }
    
    // 保存当前环境
    auto previousEnv = interpreter.getCurrentEnvironment();
    interpreter.setCurrentEnvironment(constructorEnv);
    
    try {
        // 执行构造器体
        for (const auto& stmt : definition_.body->statements) {
            stmt->accept(interpreter);
        }
        
        // 恢复环境
        interpreter.setCurrentEnvironment(previousEnv);
        
        // 对于可失败构造器，检查是否应该返回 nil
        if (definition_.isFailable) {
            // 这里可以添加失败条件检查逻辑
            // 暂时总是成功
        }
        
        // 返回成功标志（实际的对象创建在外部处理）
        return Value(true);
        
    } catch (const std::exception& e) {
        // 恢复环境
        interpreter.setCurrentEnvironment(previousEnv);
        
        // 对于可失败构造器，返回 nil
        if (definition_.isFailable) {
            return Value(); // nil
        }
        
        // 对于普通构造器，重新抛出异常
        throw;
    }
}

// DestructorValue 实现
void DestructorValue::call(Interpreter& interpreter, const Value& selfValue) {
    // 创建析构器调用环境
    auto destructorEnv = std::make_shared<DestructorCallEnvironment>(
        environment_, selfValue);
    
    // 保存当前环境
    auto previousEnv = interpreter.getCurrentEnvironment();
    interpreter.setCurrentEnvironment(destructorEnv);
    
    try {
        // 执行析构器体
        for (const auto& stmt : definition_.body->statements) {
            stmt->accept(interpreter);
        }
        
        // 恢复环境
        interpreter.setCurrentEnvironment(previousEnv);
        
    } catch (const std::exception& e) {
        // 恢复环境
        interpreter.setCurrentEnvironment(previousEnv);
        
        // 析构器中的异常应该被记录但不应该传播
        std::cerr << "Warning: Exception in destructor: " << e.what() << std::endl;
    }
}

// ConstructorManager 实现
void ConstructorManager::addConstructor(ConstructorDefinition&& constructor) {
    constructors_.push_back(std::move(constructor));
}

const ConstructorDefinition* ConstructorManager::getConstructor(
    const std::vector<std::string>& paramTypes) const {
    
    for (const auto& constructor : constructors_) {
        if (matchesParameterTypes(constructor, paramTypes)) {
            return &constructor;
        }
    }
    return nullptr;
}

const ConstructorDefinition* ConstructorManager::getDefaultConstructor() const {
    return getConstructor({}); // 无参数构造器
}

std::unique_ptr<ConstructorValue> ConstructorManager::createConstructorInstance(
    const std::vector<std::string>& paramTypes,
    std::shared_ptr<Environment> env) const {
    
    const auto* constructor = getConstructor(paramTypes);
    if (!constructor) {
        return nullptr;
    }
    
    return std::make_unique<ConstructorValue>(*constructor, env);
}

bool ConstructorManager::matchesParameterTypes(
    const ConstructorDefinition& constructor,
    const std::vector<std::string>& paramTypes) const {
    
    if (constructor.parameters.size() != paramTypes.size()) {
        return false;
    }
    
    for (size_t i = 0; i < paramTypes.size(); ++i) {
        // 简化的类型匹配 - 在完整实现中需要更复杂的类型系统
        if (!constructor.parameters[i].type.lexeme.empty() && 
            constructor.parameters[i].type.lexeme != paramTypes[i]) {
            return false;
        }
    }
    
    return true;
}

// DestructorManager 实现
void DestructorManager::setDestructor(DestructorDefinition&& destructor) {
    destructor_ = std::make_unique<DestructorDefinition>(std::move(destructor));
}

const DestructorDefinition* DestructorManager::getDestructor() const {
    return destructor_.get();
}

std::unique_ptr<DestructorValue> DestructorManager::createDestructorInstance(
    std::shared_ptr<Environment> env) const {
    
    if (!destructor_) {
        return nullptr;
    }
    
    return std::make_unique<DestructorValue>(*destructor_, env);
}

// ConstructorCallEnvironment 实现
void ConstructorCallEnvironment::assign(const Token& name, const Value& value) {
    // 首先尝试设置属性
    if (properties_ && properties_->hasProperty(name.lexeme)) {
        // 这里需要 Interpreter 实例，暂时使用基类实现
        // 在实际使用中，需要传递 Interpreter 引用
        Environment::assign(name, value);
        return;
    }
    
    // 否则使用标准环境赋值
    Environment::assign(name, value);
}

Value ConstructorCallEnvironment::get(const Token& name) {
    // 首先尝试从属性获取
    if (properties_ && properties_->hasProperty(name.lexeme)) {
        // 这里需要 Interpreter 实例，暂时使用基类实现
        // 在实际使用中，需要传递 Interpreter 引用
        return Environment::get(name);
    }
    
    // 否则使用标准环境获取
    return Environment::get(name);
}

// ConstructorChain 实现
Value ConstructorChain::callSelfInit(Interpreter& interpreter,
                                   const ConstructorManager& manager,
                                   const std::vector<Value>& arguments,
                                   InstancePropertyContainer* properties,
                                   InstanceMethodContainer* methods,
                                   std::shared_ptr<Environment> env) {
    
    // 获取参数类型
    auto paramTypes = getArgumentTypes(arguments);
    
    // 查找匹配的构造器
    auto constructor = manager.createConstructorInstance(paramTypes, env);
    if (!constructor) {
        throw std::runtime_error("No matching constructor found for self.init call");
    }
    
    // 调用构造器
    return constructor->call(interpreter, arguments, properties, methods);
}

Value ConstructorChain::callSuperInit(Interpreter& interpreter,
                                    const std::vector<Value>& arguments,
                                    const Value& selfValue) {
    // 抑制未使用参数警告
    (void)interpreter;
    (void)arguments;
    (void)selfValue;
    
    // 父类构造器调用 - 为将来的继承功能预留
    // 目前返回成功
    return Value(true);
}

std::vector<std::string> ConstructorChain::getArgumentTypes(const std::vector<Value>& arguments) {
    std::vector<std::string> types;
    
    for (const auto& arg : arguments) {
        // 简化的类型推断
        switch (arg.type) {
            case ValueType::Int:
                types.push_back("Int");
                break;
            case ValueType::Double:
                types.push_back("Double");
                break;
            case ValueType::String:
                types.push_back("String");
                break;
            case ValueType::Bool:
                types.push_back("Bool");
                break;
            case ValueType::Array:
                types.push_back("Array");
                break;
            case ValueType::Dictionary:
                types.push_back("Dictionary");
                break;
            case ValueType::Struct:
                types.push_back("Struct");
                break;
            case ValueType::Class:
                types.push_back("Class");
                break;
            default:
                types.push_back("Any");
                break;
        }
    }
    
    return types;
}

} // namespace miniswift