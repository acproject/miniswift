#ifndef MINISWIFT_SYMBOL_TABLE_H
#define MINISWIFT_SYMBOL_TABLE_H

#include "TypeSystem.h"
#include "../parser/AccessControl.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>

namespace miniswift {

// 符号类型枚举
enum class SymbolKind {
    Variable,
    Function,
    Type,
    Class,
    Struct,
    Enum,
    Protocol,
    Module,
    Generic,
    Macro
};

// Using AccessLevel from AccessControl.h

// 符号信息基类
struct Symbol {
    std::string name;
    SymbolKind kind;
    std::shared_ptr<Type> type;
    AccessLevel accessLevel;
    bool isConst;
    bool isMutable;
    int declarationLine;
    int declarationColumn;
    std::string sourceFile;
    
    Symbol(const std::string& n, SymbolKind k, std::shared_ptr<Type> t, 
           AccessLevel access = AccessLevel::INTERNAL, bool isC = false)
        : name(n), kind(k), type(t), accessLevel(access), isConst(isC), 
          isMutable(!isC), declarationLine(-1), declarationColumn(-1) {}
    
    virtual ~Symbol() = default;
};

// 变量符号
struct VariableSymbol : Symbol {
    bool isInitialized;
    bool isUsed;
    
    VariableSymbol(const std::string& name, std::shared_ptr<Type> type, 
                   bool isConst = false, AccessLevel access = AccessLevel::INTERNAL)
        : Symbol(name, SymbolKind::Variable, type, access, isConst),
          isInitialized(false), isUsed(false) {}
};

// 函数符号
struct FunctionSymbol : Symbol {
    std::vector<std::shared_ptr<Type>> parameterTypes;
    std::vector<std::string> parameterNames;
    std::vector<std::string> parameterLabels;
    std::shared_ptr<Type> returnType;
    bool isThrows;
    bool isAsync;
    bool isGeneric;
    std::vector<std::string> genericParameters;
    
    FunctionSymbol(const std::string& name, std::shared_ptr<FunctionType> funcType,
                   AccessLevel access = AccessLevel::INTERNAL)
        : Symbol(name, SymbolKind::Function, funcType, access),
          isThrows(false), isAsync(false), isGeneric(false) {
        if (auto ft = std::dynamic_pointer_cast<FunctionType>(funcType)) {
            parameterTypes = ft->parameterTypes;
            returnType = ft->returnType;
        }
    }
};

// 类型符号
struct TypeSymbol : Symbol {
    std::vector<std::string> memberNames;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> members;
    std::vector<std::string> protocols;
    std::string superclass;
    bool isGeneric;
    std::vector<std::string> genericParameters;
    
    TypeSymbol(const std::string& name, SymbolKind kind, std::shared_ptr<Type> type,
               AccessLevel access = AccessLevel::INTERNAL)
        : Symbol(name, kind, type, access), isGeneric(false) {}
};

// 作用域类
class Scope {
public:
    Scope(Scope* parent = nullptr) : parentScope(parent) {}
    
    // 符号管理
    void define(const std::string& name, std::shared_ptr<Symbol> symbol);
    std::shared_ptr<Symbol> lookup(const std::string& name) const;
    std::shared_ptr<Symbol> lookupLocal(const std::string& name) const;
    bool exists(const std::string& name) const;
    bool existsLocal(const std::string& name) const;
    
    // 作用域管理
    Scope* getParent() const { return parentScope; }
    void setParent(Scope* parent) { parentScope = parent; }
    
    // 获取所有符号
    const std::unordered_map<std::string, std::shared_ptr<Symbol>>& getSymbols() const {
        return symbols;
    }
    
    // 作用域类型
    enum class ScopeType {
        Global,
        Function,
        Class,
        Struct,
        Block,
        Loop,
        Conditional
    };
    
    void setScopeType(ScopeType type) { scopeType = type; }
    ScopeType getScopeType() const { return scopeType; }
    
private:
    Scope* parentScope;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols;
    ScopeType scopeType = ScopeType::Block;
};

// 符号表主类
class SymbolTable {
public:
    SymbolTable();
    virtual ~SymbolTable() = default;
    
    // 作用域管理
    void enterScope(Scope::ScopeType type = Scope::ScopeType::Block);
    void exitScope();
    Scope* getCurrentScope() const { return currentScope; }
    Scope* getGlobalScope() const { return globalScope.get(); }
    
    // 符号定义
    void defineVariable(const std::string& name, std::shared_ptr<Type> type, 
                       bool isConst = false, AccessLevel access = AccessLevel::INTERNAL);
    void defineFunction(const std::string& name, std::shared_ptr<FunctionType> type,
                       AccessLevel access = AccessLevel::INTERNAL);
    void defineType(const std::string& name, SymbolKind kind, std::shared_ptr<Type> type,
                   AccessLevel access = AccessLevel::INTERNAL);
    void defineGeneric(const std::string& name, const std::vector<std::string>& constraints);
    
    // 符号查找
    std::shared_ptr<Symbol> lookup(const std::string& name) const;
    std::shared_ptr<Symbol> lookupLocal(const std::string& name) const;
    std::shared_ptr<VariableSymbol> lookupVariable(const std::string& name) const;
    std::shared_ptr<FunctionSymbol> lookupFunction(const std::string& name) const;
    std::shared_ptr<TypeSymbol> lookupType(const std::string& name) const;
    
    // 符号存在性检查
    bool exists(const std::string& name) const;
    bool existsLocal(const std::string& name) const;
    bool existsInCurrentScope(const std::string& name) const;
    
    // 类型检查辅助
    bool isVariableDeclared(const std::string& name) const;
    bool isFunctionDeclared(const std::string& name) const;
    bool isTypeDeclared(const std::string& name) const;
    
    // 函数重载支持
    std::vector<std::shared_ptr<FunctionSymbol>> lookupOverloadedFunctions(const std::string& name) const;
    std::shared_ptr<FunctionSymbol> findBestMatch(const std::string& name, 
                                                  const std::vector<std::shared_ptr<Type>>& argumentTypes) const;
    
    // 泛型支持
    void enterGenericScope(const std::vector<std::string>& genericParameters);
    void exitGenericScope();
    bool isGenericParameter(const std::string& name) const;
    
    // 模块支持
    void importModule(const std::string& moduleName);
    void exportSymbol(const std::string& name);
    std::vector<std::string> getExportedSymbols() const;
    
    // 调试和诊断
    void printSymbolTable() const;
    void printCurrentScope() const;
    std::vector<std::string> getUndeclaredVariables() const;
    std::vector<std::string> getUnusedVariables() const;
    
    // 符号使用标记
    void markVariableAsUsed(const std::string& name);
    void markVariableAsInitialized(const std::string& name);
    
private:
    std::unique_ptr<Scope> globalScope;
    Scope* currentScope;
    std::vector<std::unique_ptr<Scope>> scopeStack;
    
    // 泛型参数栈
    std::vector<std::vector<std::string>> genericParameterStack;
    
    // 模块管理
    std::unordered_map<std::string, std::vector<std::string>> importedModules;
    std::vector<std::string> exportedSymbols;
    
    // 内置类型初始化
    void initializeBuiltinTypes();
    void initializeBuiltinFunctions();
    
    // 辅助方法
    std::shared_ptr<Symbol> createBuiltinTypeSymbol(const std::string& name, std::shared_ptr<Type> type);
    std::shared_ptr<Symbol> createBuiltinFunctionSymbol(const std::string& name, std::shared_ptr<FunctionType> type);
};

} // namespace miniswift

#endif // MINISWIFT_SYMBOL_TABLE_H