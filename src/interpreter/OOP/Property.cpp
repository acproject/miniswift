#include "Property.h"
#include "../Environment.h"
#include "../Interpreter.h"
#include <stdexcept>
#include <iostream>

namespace miniswift {

// Define RuntimeError as an alias for std::runtime_error
using RuntimeError = std::runtime_error;

// PropertyValue 实现

Value PropertyValue::getValue(Interpreter &interpreter) {
  // 处理不同类型的属性
  switch (definition_.propertyType) {
  case PropertyType::STORED:
    // 存储属性直接返回存储的值
    return storedValue_;

  case PropertyType::COMPUTED:
    // 计算属性调用 getter
    return executeAccessor(interpreter, AccessorType::GET);

  case PropertyType::LAZY:
    // 延迟属性：如果未初始化，先计算并存储
    if (!isInitialized_) {
      // 执行默认值表达式
      if (definition_.defaultValue) {
        storedValue_ = interpreter.evaluate(*definition_.defaultValue);
        isInitialized_ = true;
      }
    }
    return storedValue_;
  }

  // 默认返回空值
  return Value();
}

void PropertyValue::setValue(Interpreter &interpreter, const Value &newValue) {
  // 检查是否是只读属性
  if (!definition_.isVar && isInitialized_) {
    throw RuntimeError("Cannot assign to value: '" + definition_.name.lexeme +
                       "' is a 'let' constant");
  }

  // 处理不同类型的属性
  switch (definition_.propertyType) {
  case PropertyType::STORED: {
    // 存储属性：检查是否有观察器
    if (const auto *willSet = findAccessor(AccessorType::WILL_SET)) {
      (void)willSet; // 抑制未使用变量警告
      // 执行 willSet 观察器
      executeAccessor(interpreter, AccessorType::WILL_SET, &newValue);
    }

    // 存储新值
    Value oldValue = storedValue_;
    storedValue_ = newValue;
    isInitialized_ = true;

    // 执行 didSet 观察器
    if (const auto *didSet = findAccessor(AccessorType::DID_SET)) {
      (void)didSet; // 抑制未使用变量警告
      executeAccessor(interpreter, AccessorType::DID_SET, &oldValue);
    }
    break;
  }

  case PropertyType::COMPUTED:
    // 计算属性：调用 setter
    executeAccessor(interpreter, AccessorType::SET, &newValue);
    break;

  case PropertyType::LAZY:
    // 延迟属性：直接设置值并标记为已初始化
    storedValue_ = newValue;
    isInitialized_ = true;
    break;
  }
}

Value PropertyValue::executeAccessor(Interpreter &interpreter,
                                     AccessorType type, const Value *newValue) {
  const auto *accessor = findAccessor(type);
  if (!accessor) {
    if (type == AccessorType::SET) {
      throw RuntimeError("Cannot assign to value: '" + definition_.name.lexeme +
                         "' is a read-only property");
    }
    return Value(); // 没有找到访问器，返回空值
  }

  // 创建特殊的访问器环境，能够处理属性赋值
  auto accessorEnv = std::make_shared<PropertyAccessorEnvironment>(
      environment_, container_, &interpreter);

  // 设置访问器的参数
  if (newValue) {
    std::string paramName = "newValue"; // 默认参数名

    // 如果有自定义参数名，使用自定义参数名
    if (!accessor->parameterName.empty()) {
      paramName = accessor->parameterName;
    }

    accessorEnv->define(paramName, *newValue);
  }

  // 不再预先定义属性变量，让 PropertyAccessorEnvironment::get
  // 直接从属性系统获取 这样可以确保访问器中总是能获取到最新的属性值

  // 执行访问器体并捕获返回值
  try {
    interpreter.executeWithEnvironment(*accessor->body, accessorEnv);

    // 对于 getter，如果没有显式返回，检查环境中的 "return" 变量
    if (type == AccessorType::GET) {
      if (accessorEnv->exists("return")) {
        return accessorEnv->get(Token{TokenType::Identifier, "return", 0});
      }
    }

    return Value();
  } catch (const std::runtime_error &e) {
    // 检查是否是 ReturnException
    std::string errorMsg = e.what();
    if (errorMsg == "return") {
      // ReturnException 已经被 executeWithEnvironment 处理，
      // 返回值已存储在环境的 "return" 变量中
      if (accessorEnv->exists("return")) {
        return accessorEnv->get(Token{TokenType::Identifier, "return", 0});
      }
    }
    throw; // 其他异常直接抛出
  }
}

const PropertyAccessor *PropertyValue::findAccessor(AccessorType type) const {
  for (const auto &accessor : definition_.accessors) {
    if (accessor.type == type) {
      return &accessor;
    }
  }
  return nullptr;
}

// PropertyManager 实现

void PropertyManager::addProperty(PropertyDefinition &&property) {
  properties_.push_back(std::move(property));
}

const PropertyDefinition *
PropertyManager::getProperty(const std::string &name) const {
  for (const auto &prop : properties_) {
    if (prop.name.lexeme == name) {
      return &prop;
    }
  }
  return nullptr;
}

std::unique_ptr<PropertyValue> PropertyManager::createPropertyInstance(
    const std::string &name, std::shared_ptr<Environment> env) const {
  const auto *propDef = getProperty(name);
  if (!propDef) {
    return nullptr;
  }

  return std::make_unique<PropertyValue>(*propDef, env);
}

// InstancePropertyContainer 实现

InstancePropertyContainer::InstancePropertyContainer(
    const PropertyManager &manager, std::shared_ptr<Environment> env)
    : environment_(env) {
  // 为所有属性创建实例
  for (const auto &propDef : manager.getAllProperties()) {
    auto propValue = std::make_unique<PropertyValue>(propDef, env);
    propValue->setContainer(this); // 设置容器引用
    properties_[propDef.name.lexeme] = std::move(propValue);
  }
}

// Copy constructor
InstancePropertyContainer::InstancePropertyContainer(
    const InstancePropertyContainer &other)
    : environment_(other.environment_) {
  // Copy all properties with their complete state
  for (const auto &[name, propValue] : other.properties_) {
    auto newPropValue = std::make_unique<PropertyValue>(*propValue);
    newPropValue->setContainer(this); // 设置新容器引用
    properties_[name] = std::move(newPropValue);
  }
}

Value InstancePropertyContainer::getProperty(Interpreter &interpreter,
                                             const std::string &name) {
  auto it = properties_.find(name);
  if (it == properties_.end()) {
    throw RuntimeError("Undefined property '" + name + "'");
  }

  return it->second->getValue(interpreter);
}

void InstancePropertyContainer::setProperty(Interpreter &interpreter,
                                            const std::string &name,
                                            const Value &value) {
  auto it = properties_.find(name);
  if (it == properties_.end()) {
    throw RuntimeError("Undefined property '" + name + "'");
  }

  it->second->setValue(interpreter, value);
}

bool InstancePropertyContainer::hasProperty(const std::string &name) const {
  return properties_.find(name) != properties_.end();
}

const PropertyDefinition *InstancePropertyContainer::getPropertyDefinition(
    const std::string &name) const {
  auto it = properties_.find(name);
  if (it != properties_.end()) {
    return &it->second->getDefinition();
  }
  return nullptr;
}

void InstancePropertyContainer::initializeDefaults(Interpreter &interpreter) {
  for (auto &[name, propValue] : properties_) {
    const auto &def = propValue->getDefinition();

    // 只初始化存储属性，延迟属性在访问时初始化
    if (def.propertyType == PropertyType::STORED && def.defaultValue &&
        !propValue->isInitialized()) {
      Value defaultValue = interpreter.evaluate(*def.defaultValue);
      propValue->setValue(interpreter, defaultValue);
    }
  }
}

// PropertyAccessorEnvironment 实现

void PropertyAccessorEnvironment::assign(const Token &name,
                                         const Value &value) {
  // 检查是否是属性赋值
  if (container_ && container_->hasProperty(name.lexeme)) {
    // 转发到属性系统
    container_->setProperty(*interpreter_, name.lexeme, value);

    // 同时更新环境中的变量，以便后续访问能看到新值
    if (exists(name.lexeme)) {
      Environment::assign(name, value);
    }
  } else {
    // 回退到普通环境赋值
    Environment::assign(name, value);
  }
}

Value PropertyAccessorEnvironment::get(const Token &name) {
  // 检查是否是属性访问
  if (container_ && container_->hasProperty(name.lexeme)) {
    // 只对存储属性进行转发，避免递归调用计算属性
    const auto *propDef = container_->getPropertyDefinition(name.lexeme);
    if (propDef && propDef->propertyType == PropertyType::STORED) {
      // 从属性系统获取存储属性的最新值
      return container_->getProperty(*interpreter_, name.lexeme);
    }
  }

  // 回退到普通环境访问
  return Environment::get(name);
}

} // namespace miniswift