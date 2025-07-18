#ifndef MINISWIFT_PROPERTY_H
#define MINISWIFT_PROPERTY_H

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

// 属性类型枚举
enum class PropertyType {
    STORED,      // 存储属性
    COMPUTED,    // 计算属性
    LAZY         // 延迟属性
};

// Forward declarations from AST.h
enum class AccessorType;
struct PropertyAccessor;

// 属性定义（扩展的 StructMember）
struct PropertyDefinition {
    Token name;
    Token type;
    PropertyType propertyType;
    bool isVar;                   // true for var, false for let
    bool isStatic;               // 是否为静态属性
    
    // 存储属性相关
    std::unique_ptr<Expr> defaultValue;
    
    // 计算属性和观察器相关
    std::vector<PropertyAccessor> accessors;
    
    // 延迟属性相关
    bool isLazy;
    
    PropertyDefinition(Token n, Token t, PropertyType pt = PropertyType::STORED, bool var = true)
        : name(n), type(t), propertyType(pt), isVar(var), isStatic(false), isLazy(false) {}
    
    // Move constructor
    PropertyDefinition(PropertyDefinition&& other) noexcept
        : name(std::move(other.name)), type(std::move(other.type)), propertyType(other.propertyType),
          isVar(other.isVar), isStatic(other.isStatic), defaultValue(std::move(other.defaultValue)),
          accessors(std::move(other.accessors)), isLazy(other.isLazy) {}
    
    // Move assignment operator
    PropertyDefinition& operator=(PropertyDefinition&& other) noexcept {
        if (this != &other) {
            name = std::move(other.name);
            type = std::move(other.type);
            propertyType = other.propertyType;
            isVar = other.isVar;
            isStatic = other.isStatic;
            defaultValue = std::move(other.defaultValue);
            accessors = std::move(other.accessors);
            isLazy = other.isLazy;
        }
        return *this;
    }
    
    // Delete copy constructor and copy assignment
    PropertyDefinition(const PropertyDefinition&) = delete;
    PropertyDefinition& operator=(const PropertyDefinition&) = delete;
};

// Forward declaration
class InstancePropertyContainer;
class PropertyAccessorEnvironment;

// 属性运行时值
class PropertyValue {
public:
    PropertyValue(const PropertyDefinition& def, std::shared_ptr<Environment> env)
        : definition_(def), environment_(env), isInitialized_(false), container_(nullptr) {}
    
    // Copy constructor
    PropertyValue(const PropertyValue& other)
        : definition_(other.definition_), environment_(other.environment_), 
          storedValue_(other.storedValue_), isInitialized_(other.isInitialized_), container_(other.container_) {}
    
    // 设置属性容器引用
    void setContainer(InstancePropertyContainer* container) { container_ = container; }
    
    // 获取属性值
    Value getValue(Interpreter& interpreter);
    
    // 设置属性值
    void setValue(Interpreter& interpreter, const Value& newValue);
    
    // 检查是否已初始化（用于延迟属性）
    bool isInitialized() const { return isInitialized_; }
    
    // 获取属性定义
    const PropertyDefinition& getDefinition() const { return definition_; }
    
private:
    const PropertyDefinition& definition_;
    std::shared_ptr<Environment> environment_;
    Value storedValue_;           // 存储属性的值
    bool isInitialized_;         // 是否已初始化
    InstancePropertyContainer* container_;  // 指向所属容器的指针
    
    // 执行访问器
    Value executeAccessor(Interpreter& interpreter, AccessorType type, const Value* newValue = nullptr);
    
    // 查找指定类型的访问器
    const PropertyAccessor* findAccessor(AccessorType type) const;
};

// 属性管理器 - 管理类型的所有属性
class PropertyManager {
public:
    // 添加属性定义
    void addProperty(PropertyDefinition&& property);
    
    // 获取属性定义
    const PropertyDefinition* getProperty(const std::string& name) const;
    
    // 获取所有属性
    const std::vector<PropertyDefinition>& getAllProperties() const { return properties_; }
    
    // 创建属性实例
    std::unique_ptr<PropertyValue> createPropertyInstance(
        const std::string& name, 
        std::shared_ptr<Environment> env
    ) const;
    
private:
    std::vector<PropertyDefinition> properties_;
};

// 实例属性容器 - 管理对象实例的属性值
class InstancePropertyContainer {
    friend class PropertyValue;  // 允许 PropertyValue 访问私有成员
    
public:
    explicit InstancePropertyContainer(const PropertyManager& manager, std::shared_ptr<Environment> env);
    
    // Copy constructor
    InstancePropertyContainer(const InstancePropertyContainer& other);
    
    // 获取属性值
    Value getProperty(Interpreter& interpreter, const std::string& name);
    
    // 设置属性值
    void setProperty(Interpreter& interpreter, const std::string& name, const Value& value);
    
    // 检查属性是否存在
    bool hasProperty(const std::string& name) const;
    
    // 获取属性定义
    const PropertyDefinition* getPropertyDefinition(const std::string& name) const;
    
    // 初始化所有存储属性的默认值
    void initializeDefaults(Interpreter& interpreter);
    
private:
    std::unordered_map<std::string, std::unique_ptr<PropertyValue>> properties_;
    std::shared_ptr<Environment> environment_;
};

// 特殊的访问器环境，能够将对属性变量的赋值和访问转发到属性系统
class PropertyAccessorEnvironment : public Environment {
public:
    PropertyAccessorEnvironment(std::shared_ptr<Environment> enclosing, 
                               InstancePropertyContainer* container,
                               Interpreter* interpreter)
        : Environment(enclosing), container_(container), interpreter_(interpreter) {}
    
    void assign(const Token& name, const Value& value) override;
    Value get(const Token& name) override;
    
private:
    InstancePropertyContainer* container_;
    Interpreter* interpreter_;
};

} // namespace miniswift

#endif // MINISWIFT_PROPERTY_H