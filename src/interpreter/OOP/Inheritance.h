#ifndef MINISWIFT_INHERITANCE_H
#define MINISWIFT_INHERITANCE_H

#include "../../parser/StmtNodes.h"
#include "../Value.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace miniswift {

// Forward declarations
class Environment;
class Interpreter;

/**
 * 继承系统核心类
 * 负责管理类的继承关系、方法重写、super关键字等
 */
class InheritanceManager {
public:
  InheritanceManager() = default;
  ~InheritanceManager() = default;

  // 注册类的继承关系
  void registerClass(const std::string &className,
                     const std::string &superClassName = "");

  // 检查类是否存在
  bool hasClass(const std::string &className) const;

  // 获取类的超类名称
  std::string getSuperclass(const std::string &className) const;

  // 检查是否为子类关系
  bool isSubclass(const std::string &subclass,
                  const std::string &superclass) const;

  // 获取类的继承链（从子类到根类）
  std::vector<std::string>
  getInheritanceChain(const std::string &className) const;

  // 查找方法（支持继承查找）
  std::shared_ptr<FunctionStmt> findMethod(const std::string &className,
                                           const std::string &methodName) const;

  // 只在指定类中查找方法（不递归查找父类）
  std::shared_ptr<FunctionStmt> findMethodInClass(const std::string &className,
                                                   const std::string &methodName) const;

  // 注册类的方法
  void registerMethod(const std::string &className,
                      const std::string &methodName,
                      std::shared_ptr<FunctionStmt> method);

  // 检查方法是否被重写
  bool isMethodOverridden(const std::string &className,
                          const std::string &methodName) const;

  // 获取被重写的方法
  std::shared_ptr<FunctionStmt>
  getOverriddenMethod(const std::string &className,
                      const std::string &methodName) const;

private:
  // 类继承关系映射：子类 -> 父类
  std::unordered_map<std::string, std::string> inheritanceMap_;

  // 类方法映射：类名 -> {方法名 -> 方法}
  std::unordered_map<
      std::string,
      std::unordered_map<std::string, std::shared_ptr<FunctionStmt>>>
      classMethods_;

  public:
    // 递归查找方法
    std::shared_ptr<FunctionStmt>
    findMethodRecursive(const std::string &className,
                        const std::string &methodName) const;
};

/**
 * Super关键字处理器
 * 处理super.method()调用和super.property访问
 */
class SuperHandler {
public:
  SuperHandler(InheritanceManager &inheritanceManager,
               Interpreter &interpreter);

  // 处理super方法调用
  Value callSuperMethod(const std::string &currentClass,
                        const std::string &methodName,
                        const std::vector<Value> &arguments,
                        std::shared_ptr<Environment> environment);

  // 处理super属性访问
  Value getSuperProperty(const std::string &currentClass,
                         const std::string &propertyName,
                         std::shared_ptr<Environment> environment);

  // 处理super属性设置
  void setSuperProperty(const std::string &currentClass,
                        const std::string &propertyName, const Value &value,
                        std::shared_ptr<Environment> environment);

private:
  InheritanceManager &inheritanceManager_;
  Interpreter &interpreter_;
};

/**
 * 方法重写验证器
 * 验证方法重写的合法性
 */
class OverrideValidator {
public:
  OverrideValidator(InheritanceManager &inheritanceManager);

  // 验证方法重写是否合法
  bool validateOverride(const std::string &className,
                        const std::string &methodName,
                        const std::vector<Parameter> &parameters,
                        const Token &returnType) const;

  // 检查方法签名是否匹配
  bool isSignatureCompatible(const std::vector<Parameter> &overrideParams,
                             const Token &overrideReturnType,
                             const std::vector<Parameter> &baseParams,
                             const Token &baseReturnType) const;

  // 验证final方法不能被重写
  bool isFinalMethod(const std::string &className,
                     const std::string &methodName) const;

private:
  InheritanceManager &inheritanceManager_;

  // final方法集合：类名 -> {方法名}
  std::unordered_map<std::string, std::unordered_set<std::string>>
      finalMethods_;
};

/**
 * 类实例管理器
 * 管理类实例的创建和继承关系
 */
class ClassInstanceManager {
public:
  ClassInstanceManager(InheritanceManager &inheritanceManager);

  // 创建类实例
  std::shared_ptr<ClassInstance>
  createInstance(const std::string &className,
                 const std::vector<Value> &initArgs);

  // 检查实例是否为某个类的实例
  bool isInstanceOf(std::shared_ptr<ClassInstance> instance,
                    const std::string &className) const;

  // 类型转换检查
  bool canCastTo(std::shared_ptr<ClassInstance> instance,
                 const std::string &targetClassName) const;

  // 执行类型转换
  std::shared_ptr<ClassInstance>
  castTo(std::shared_ptr<ClassInstance> instance,
         const std::string &targetClassName) const;

private:
  InheritanceManager &inheritanceManager_;
};

} // namespace miniswift

#endif // MINISWIFT_INHERITANCE_H