#ifndef MINISWIFT_METHOD_INTERPRETER_H
#define MINISWIFT_METHOD_INTERPRETER_H

#include "../Interpreter.h"
#include "Method.h"
#include <unordered_map>
#include <string>
#include <memory>

namespace miniswift {

// 方法解释器 - 继承自 Interpreter，扩展方法相关功能
class MethodInterpreter : public Interpreter {
public:
    MethodInterpreter() = default;
    
    // 注册结构体方法
    void registerStructMethods(const std::string& structName, const std::vector<std::unique_ptr<FunctionStmt>>& methods);
    
    // 注册类方法
    void registerClassMethods(const std::string& className, const std::vector<std::unique_ptr<FunctionStmt>>& methods);
    
    // 获取结构体方法
    const MethodDefinition* getStructMethod(const std::string& structName, const std::string& methodName) const;
    
    // 获取类方法
    const MethodDefinition* getClassMethod(const std::string& className, const std::string& methodName) const;
    
    // 重写 getMemberValue 以支持方法调用
    Value getMemberValue(const Value& object, const std::string& memberName) override;
    
    // 调用结构体方法
    Value callStructMethod(const std::string& structName, const std::string& methodName, 
                          const std::vector<Value>& arguments, const Value& selfValue);
    
    // 调用类方法
    Value callClassMethod(const std::string& className, const std::string& methodName, 
                         const std::vector<Value>& arguments, const Value& selfValue);
    
    // 检查是否存在结构体方法
    bool hasStructMethod(const std::string& structName, const std::string& methodName) const;
    
    // 检查是否存在类方法
    bool hasClassMethod(const std::string& className, const std::string& methodName) const;
    
private:
    // 结构体方法管理器
    std::unordered_map<std::string, std::unique_ptr<MethodManager>> structMethods_;
    
    // 类方法管理器
    std::unordered_map<std::string, std::unique_ptr<MethodManager>> classMethods_;
    
    // 辅助方法：从 FunctionStmt 创建 MethodDefinition
    MethodDefinition createMethodDefinition(const FunctionStmt& funcStmt);
    
    // 辅助方法：获取方法管理器
    MethodManager* getStructMethodManager(const std::string& structName);
    MethodManager* getClassMethodManager(const std::string& className);
    const MethodManager* getStructMethodManager(const std::string& structName) const;
    const MethodManager* getClassMethodManager(const std::string& className) const;
};

} // namespace miniswift

#endif // MINISWIFT_METHOD_INTERPRETER_H