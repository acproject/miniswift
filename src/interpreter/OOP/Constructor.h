#ifndef MINISWIFT_CONSTRUCTOR_H
#define MINISWIFT_CONSTRUCTOR_H

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
class InstancePropertyContainer;
class InstanceMethodContainer;

// 构造器定义
struct ConstructorDefinition {
    InitType initType;
    std::vector<Parameter> parameters;
    std::unique_ptr<BlockStmt> body;
    bool isRequired;
    bool isFailable;
    
    ConstructorDefinition(InitType type, std::vector<Parameter> params, 
                         std::unique_ptr<BlockStmt> body, bool required = false)
        : initType(type), parameters(std::move(params)), body(std::move(body)), 
          isRequired(required), isFailable(type == InitType::FAILABLE) {}
    
    // Move constructor
    ConstructorDefinition(ConstructorDefinition&& other) noexcept
        : initType(other.initType), parameters(std::move(other.parameters)),
          body(std::move(other.body)), isRequired(other.isRequired), 
          isFailable(other.isFailable) {}
    
    // Move assignment operator
    ConstructorDefinition& operator=(ConstructorDefinition&& other) noexcept {
        if (this != &other) {
            initType = other.initType;
            parameters = std::move(other.parameters);
            body = std::move(other.body);
            isRequired = other.isRequired;
            isFailable = other.isFailable;
        }
        return *this;
    }
    
    // Delete copy constructor and copy assignment
    ConstructorDefinition(const ConstructorDefinition&) = delete;
    ConstructorDefinition& operator=(const ConstructorDefinition&) = delete;
};

// 析构器定义
struct DestructorDefinition {
    std::unique_ptr<BlockStmt> body;
    
    explicit DestructorDefinition(std::unique_ptr<BlockStmt> body)
        : body(std::move(body)) {}
    
    // Move constructor
    DestructorDefinition(DestructorDefinition&& other) noexcept
        : body(std::move(other.body)) {}
    
    // Move assignment operator
    DestructorDefinition& operator=(DestructorDefinition&& other) noexcept {
        if (this != &other) {
            body = std::move(other.body);
        }
        return *this;
    }
    
    // Delete copy constructor and copy assignment
    DestructorDefinition(const DestructorDefinition&) = delete;
    DestructorDefinition& operator=(const DestructorDefinition&) = delete;
};

// 构造器运行时值
class ConstructorValue {
public:
    ConstructorValue(const ConstructorDefinition& def, std::shared_ptr<Environment> env)
        : definition_(def), environment_(env) {}
    
    // Copy constructor
    ConstructorValue(const ConstructorValue& other)
        : definition_(other.definition_), environment_(other.environment_) {}
    
    // 调用构造器
    Value call(Interpreter& interpreter, const std::vector<Value>& arguments, 
              InstancePropertyContainer* properties, InstanceMethodContainer* methods);
    
    // 获取构造器定义
    const ConstructorDefinition& getDefinition() const { return definition_; }
    
    // 检查是否为可失败构造器
    bool isFailable() const { return definition_.isFailable; }
    
    // 检查是否为便利构造器
    bool isConvenience() const { return definition_.initType == InitType::CONVENIENCE; }
    
private:
    const ConstructorDefinition& definition_;
    std::shared_ptr<Environment> environment_;
};

// 析构器运行时值
class DestructorValue {
public:
    DestructorValue(const DestructorDefinition& def, std::shared_ptr<Environment> env)
        : definition_(def), environment_(env) {}
    
    // Copy constructor
    DestructorValue(const DestructorValue& other)
        : definition_(other.definition_), environment_(other.environment_) {}
    
    // 调用析构器
    void call(Interpreter& interpreter, const Value& selfValue);
    
    // 获取析构器定义
    const DestructorDefinition& getDefinition() const { return definition_; }
    
private:
    const DestructorDefinition& definition_;
    std::shared_ptr<Environment> environment_;
};

// 构造器管理器 - 管理类型的所有构造器
class ConstructorManager {
public:
    // 添加构造器定义
    void addConstructor(ConstructorDefinition&& constructor);
    
    // 获取指定构造器
    const ConstructorDefinition* getConstructor(const std::vector<std::string>& paramTypes) const;
    
    // 获取默认构造器（无参数）
    const ConstructorDefinition* getDefaultConstructor() const;
    
    // 获取所有构造器
    const std::vector<ConstructorDefinition>& getAllConstructors() const { return constructors_; }
    
    // 创建构造器实例
    std::unique_ptr<ConstructorValue> createConstructorInstance(
        const std::vector<std::string>& paramTypes,
        std::shared_ptr<Environment> env
    ) const;
    
    // 检查是否有构造器
    bool hasConstructors() const { return !constructors_.empty(); }
    
private:
    std::vector<ConstructorDefinition> constructors_;
    
    // 匹配参数类型
    bool matchesParameterTypes(const ConstructorDefinition& constructor, 
                              const std::vector<std::string>& paramTypes) const;
};

// 析构器管理器 - 管理类型的析构器
class DestructorManager {
public:
    // 设置析构器定义
    void setDestructor(DestructorDefinition&& destructor);
    
    // 获取析构器
    const DestructorDefinition* getDestructor() const;
    
    // 创建析构器实例
    std::unique_ptr<DestructorValue> createDestructorInstance(
        std::shared_ptr<Environment> env
    ) const;
    
    // 检查是否有析构器
    bool hasDestructor() const { return destructor_ != nullptr; }
    
private:
    std::unique_ptr<DestructorDefinition> destructor_;
};

// 构造器调用环境 - 为构造器调用创建特殊环境
class ConstructorCallEnvironment : public Environment {
public:
    ConstructorCallEnvironment(std::shared_ptr<Environment> enclosing, 
                              InstancePropertyContainer* properties,
                              InstanceMethodContainer* methods)
        : Environment(enclosing), properties_(properties), methods_(methods) {
        (void)methods_; // 抑制未使用字段警告
    }
    
    virtual ~ConstructorCallEnvironment() = default;
    
    // 重写变量访问以支持属性访问
    void assign(const Token& name, const Value& value) override;
    Value get(const Token& name) override;
    
private:
    InstancePropertyContainer* properties_;
    InstanceMethodContainer* methods_; // 为将来的方法调用功能预留
};

// 析构器调用环境 - 为析构器调用创建特殊环境
class DestructorCallEnvironment : public Environment {
public:
    DestructorCallEnvironment(std::shared_ptr<Environment> enclosing, const Value& selfValue)
        : Environment(enclosing), selfValue_(selfValue) {
        // 在环境中定义 self
        define("self", selfValue_);
    }
    
    virtual ~DestructorCallEnvironment() = default;
    
    const Value& getSelf() const { return selfValue_; }
    
private:
    Value selfValue_;
};

// 构造器链调用 - 支持构造器链（self.init 和 super.init）
class ConstructorChain {
public:
    // 调用同类的其他构造器 (self.init)
    static Value callSelfInit(Interpreter& interpreter, 
                             const ConstructorManager& manager,
                             const std::vector<Value>& arguments,
                             InstancePropertyContainer* properties,
                             InstanceMethodContainer* methods,
                             std::shared_ptr<Environment> env);
    
    // 调用父类构造器 (super.init) - 为将来的继承功能预留
    static Value callSuperInit(Interpreter& interpreter,
                              const std::vector<Value>& arguments,
                              const Value& selfValue);
    
private:
    // 获取参数类型字符串
    static std::vector<std::string> getArgumentTypes(const std::vector<Value>& arguments);
};

} // namespace miniswift

#endif // MINISWIFT_CONSTRUCTOR_H