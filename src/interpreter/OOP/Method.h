#ifndef MINISWIFT_METHOD_H
#define MINISWIFT_METHOD_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "../Value.h"
#include "../Environment.h"
#include "../../parser/AST.h"

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
    std::vector<Token> parameters;
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
    Value getMethod(const std::string& name, const Value& selfValue);
    
    // 检查方法是否存在
    bool hasMethod(const std::string& name) const;
    
    // 获取方法定义
    const MethodDefinition* getMethodDefinition(const std::string& name) const;
    
private:
    std::unordered_map<std::string, std::unique_ptr<MethodValue>> methods_;
    std::shared_ptr<Environment> environment_;
};

// 方法调用环境 - 为方法调用创建特殊环境，包含 self 绑定
class MethodCallEnvironment : public Environment {
public:
    MethodCallEnvironment(std::shared_ptr<Environment> enclosing, const Value& selfValue)
        : Environment(enclosing), selfValue_(selfValue) {
        // 在环境中定义 self
        define("self", selfValue_);
    }
    
    const Value& getSelf() const { return selfValue_; }
    
private:
    Value selfValue_;
};

} // namespace miniswift

#endif // MINISWIFT_METHOD_H