#ifndef MINISWIFT_METHOD_H
#define MINISWIFT_METHOD_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <iostream>
#include "../Value.h"
#include "../Environment.h"
#include "../../parser/AST.h"
#include "../../parser/Parameter.h"
#include "Property.h"

namespace miniswift {

// 前向声明
class Interpreter;
class Environment;

// 方法类型枚举
enum class MethodType {
    INSTANCE,    // 实例方法
    STATIC,      // 静态方法
    CLASS        // 类方法
};

// 方法定义
struct MethodDefinition {
    Token name;
    MethodType methodType;
    std::vector<Parameter> parameters;
    Token returnType;
    std::unique_ptr<BlockStmt> body;
    bool isStatic;
    
    MethodDefinition(Token n, MethodType mt = MethodType::INSTANCE, bool static_method = false)
        : name(n), methodType(mt), isStatic(static_method) {}
    
    // Move constructor
    MethodDefinition(MethodDefinition&& other) noexcept
        : name(std::move(other.name)), methodType(other.methodType),
          parameters(std::move(other.parameters)), body(std::move(other.body)),
          isStatic(other.isStatic) {}
    
    // Move assignment operator
    MethodDefinition& operator=(MethodDefinition&& other) noexcept {
        if (this != &other) {
            name = std::move(other.name);
            methodType = other.methodType;
            parameters = std::move(other.parameters);
            body = std::move(other.body);
            isStatic = other.isStatic;
        }
        return *this;
    }
    
    // Delete copy constructor and copy assignment
    MethodDefinition(const MethodDefinition&) = delete;
    MethodDefinition& operator=(const MethodDefinition&) = delete;
};

// 方法运行时值
class MethodValue {
public:
    MethodValue(const MethodDefinition& def, std::shared_ptr<Environment> env)
        : definition_(def), environment_(env) {}
    
    // Copy constructor
    MethodValue(const MethodValue& other)
        : definition_(other.definition_), environment_(other.environment_) {}
    
    // 调用方法
    Value call(Interpreter& interpreter, const std::vector<Value>& arguments, const Value* selfValue = nullptr);
    
    // 获取方法定义
    const MethodDefinition& getDefinition() const { return definition_; }
    
    // 检查是否为静态方法
    bool isStatic() const { return definition_.isStatic; }
    
private:
    const MethodDefinition& definition_;
    std::shared_ptr<Environment> environment_;
};

// 方法管理器 - 管理类型的所有方法
class MethodManager {
public:
    // 添加方法定义
    void addMethod(MethodDefinition&& method);
    
    // 获取方法定义
    const MethodDefinition* getMethod(const std::string& name) const;
    
    // 获取所有方法
    const std::vector<MethodDefinition>& getAllMethods() const { return methods_; }
    
    // 创建方法实例
    std::unique_ptr<MethodValue> createMethodInstance(
        const std::string& name, 
        std::shared_ptr<Environment> env
    ) const;
    
private:
    std::vector<MethodDefinition> methods_;
};

// 实例方法容器 - 管理对象实例的方法
class InstanceMethodContainer {
public:
    explicit InstanceMethodContainer(const MethodManager& manager, std::shared_ptr<Environment> env);
    
    // Copy constructor
    InstanceMethodContainer(const InstanceMethodContainer& other);
    
    // 调用方法
    Value callMethod(Interpreter& interpreter, const std::string& name, 
                    const std::vector<Value>& arguments, const Value& selfValue);
    
    // 获取方法（返回 Callable 值）
    Value getMethod(const std::string& name, const Value& selfValue, Interpreter& interpreter);
    
    // 检查方法是否存在
    bool hasMethod(const std::string& name) const;
    
    // 获取方法定义
    const MethodDefinition* getMethodDefinition(const std::string& name) const;
    
private:
    std::unordered_map<std::string, std::unique_ptr<MethodValue>> methods_;
    std::shared_ptr<Environment> environment_;
};

// 前向声明
class Interpreter;

// 方法调用环境 - 为方法调用创建特殊环境，包含 self 绑定
class MethodCallEnvironment : public Environment {
public:
    MethodCallEnvironment(std::shared_ptr<Environment> enclosing, const Value& selfValue, Interpreter* interpreter = nullptr)
        : Environment(enclosing), selfValue_(selfValue), interpreter_(interpreter) {
        // 在环境中定义 self
        define("self", selfValue_);
        // 也定义 __implicit_self__ 用于隐式成员访问
        define("__implicit_self__", selfValue_);
    }
    
    // 重写 get 方法以支持属性访问
    Value get(const Token& name) override {
        // std::cout << "DEBUG: MethodCallEnvironment::get called for variable: " << name.lexeme << std::endl;
        try {
            // First try to get from the current environment
            // std::cout << "DEBUG: Trying to get from current environment: " << name.lexeme << std::endl;
            Value result = Environment::get(name);
            // std::cout << "DEBUG: Found variable in environment: " << name.lexeme << std::endl;
            return result;
        } catch (const std::runtime_error&) {
            // std::cout << "DEBUG: Variable not found in environment, trying self properties: " << name.lexeme << std::endl;
            // If not found, try to get from self's properties
            if (interpreter_ && selfValue_.isClass()) {
                const auto& classInstance = selfValue_.asClass();
                // std::cout << "DEBUG: Checking self properties for: " << name.lexeme << std::endl;
                // std::cout << "DEBUG: Self class name: " << classInstance->getClassName() << std::endl;
                if (classInstance->properties) {
                    // std::cout << "DEBUG: Properties container exists" << std::endl;
                    auto allProps = classInstance->properties->getAllPropertyNames();
                    // std::cout << "DEBUG: All properties: ";
                    // for (const auto& prop : allProps) {
                    //     std::cout << prop << " ";
                    // }
                    // std::cout << std::endl;
                    if (classInstance->properties->hasProperty(name.lexeme)) {
                        // std::cout << "DEBUG: Found property in self: " << name.lexeme << std::endl;
                        return classInstance->properties->getProperty(*interpreter_, name.lexeme);
                    }
                    // std::cout << "DEBUG: Property not found in self: " << name.lexeme << std::endl;
                } else {
                    // std::cout << "DEBUG: No properties container in self" << std::endl;
                }
            } else {
                // std::cout << "DEBUG: No self object or properties available" << std::endl;
            }
            // Re-throw the original exception if not found in self either
            throw std::runtime_error("Undefined variable '" + name.lexeme + "'");
        }
    }
    
    const Value& getSelf() const { return selfValue_; }
    
private:
    Value selfValue_;
    Interpreter* interpreter_;
};

} // namespace miniswift

#endif // MINISWIFT_METHOD_H