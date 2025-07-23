#include "SymbolTable.h"
#include <algorithm>

namespace miniswift {

// Symbol和VariableSymbol的构造函数已在头文件中内联定义

// FunctionSymbol实现 - 简化版本，匹配头文件定义
// 构造函数已在头文件中内联定义

// TypeSymbol实现 - 简化版本，匹配头文件定义
// 构造函数已在头文件中内联定义

// Scope实现
void Scope::define(const std::string& name, std::shared_ptr<Symbol> symbol) {
    symbols[name] = symbol;
}

std::shared_ptr<Symbol> Scope::lookup(const std::string& name) const {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Symbol> Scope::lookupLocal(const std::string& name) const {
    return lookup(name);
}

bool Scope::exists(const std::string& name) const {
    return symbols.find(name) != symbols.end();
}

bool Scope::existsLocal(const std::string& name) const {
    return exists(name);
}

// SymbolTable实现
SymbolTable::SymbolTable() {
    // 创建全局作用域
    globalScope = std::make_unique<Scope>();
    globalScope->setScopeType(Scope::ScopeType::Global);
    currentScope = globalScope.get();
    initializeBuiltinTypes();
    initializeBuiltinFunctions();
}

void SymbolTable::enterScope(Scope::ScopeType type) {
    auto newScope = std::make_unique<Scope>(currentScope);
    newScope->setScopeType(type);
    
    scopeStack.push_back(std::move(newScope));
    currentScope = scopeStack.back().get();
}

void SymbolTable::exitScope() {
    if (scopeStack.empty()) {
        // 不能退出全局作用域
        return;
    }
    
    scopeStack.pop_back();
    if (!scopeStack.empty()) {
        currentScope = scopeStack.back().get();
    } else {
        currentScope = globalScope.get();
    }
}

// 符号定义方法的简化实现
void SymbolTable::defineVariable(const std::string& name, std::shared_ptr<Type> type, 
                                bool isConst, AccessLevel access) {
    auto symbol = std::make_shared<VariableSymbol>(name, type, isConst, access);
    currentScope->define(name, symbol);
}

void SymbolTable::defineFunction(const std::string& name, std::shared_ptr<FunctionType> type,
                                AccessLevel access) {
    auto symbol = std::make_shared<FunctionSymbol>(name, type, access);
    currentScope->define(name, symbol);
}

void SymbolTable::defineType(const std::string& name, SymbolKind kind, std::shared_ptr<Type> type,
                            AccessLevel access) {
    auto symbol = std::make_shared<TypeSymbol>(name, kind, type, access);
    currentScope->define(name, symbol);
}

// 符号查找方法的简化实现
std::shared_ptr<Symbol> SymbolTable::lookup(const std::string& name) const {
    Scope* scope = currentScope;
    while (scope) {
        auto symbol = scope->lookup(name);
        if (symbol) {
            return symbol;
        }
        scope = scope->getParent();
    }
    return nullptr;
}

std::shared_ptr<Symbol> SymbolTable::lookupLocal(const std::string& name) const {
    return currentScope ? currentScope->lookup(name) : nullptr;
}

std::shared_ptr<VariableSymbol> SymbolTable::lookupVariable(const std::string& name) const {
    auto symbol = lookup(name);
    if (symbol && symbol->kind == SymbolKind::Variable) {
        return std::static_pointer_cast<VariableSymbol>(symbol);
    }
    return nullptr;
}

std::shared_ptr<FunctionSymbol> SymbolTable::lookupFunction(const std::string& name) const {
    auto symbol = lookup(name);
    if (symbol && symbol->kind == SymbolKind::Function) {
        return std::static_pointer_cast<FunctionSymbol>(symbol);
    }
    return nullptr;
}

std::shared_ptr<TypeSymbol> SymbolTable::lookupType(const std::string& name) const {
    auto symbol = lookup(name);
    if (symbol && symbol->kind == SymbolKind::Type) {
        return std::static_pointer_cast<TypeSymbol>(symbol);
    }
    return nullptr;
}

// 符号存在性检查
bool SymbolTable::exists(const std::string& name) const {
    return lookup(name) != nullptr;
}

bool SymbolTable::existsLocal(const std::string& name) const {
    return lookupLocal(name) != nullptr;
}

bool SymbolTable::existsInCurrentScope(const std::string& name) const {
    return existsLocal(name);
}

// 类型检查辅助
bool SymbolTable::isVariableDeclared(const std::string& name) const {
    return lookupVariable(name) != nullptr;
}

bool SymbolTable::isFunctionDeclared(const std::string& name) const {
    return lookupFunction(name) != nullptr;
}

bool SymbolTable::isTypeDeclared(const std::string& name) const {
    return lookupType(name) != nullptr;
}

// 函数重载支持
std::vector<std::shared_ptr<FunctionSymbol>> SymbolTable::lookupOverloadedFunctions(const std::string& name) const {
    std::vector<std::shared_ptr<FunctionSymbol>> result;
    auto func = lookupFunction(name);
    if (func) {
        result.push_back(func);
    }
    return result;
}

std::shared_ptr<FunctionSymbol> SymbolTable::findBestMatch(const std::string& name, 
                                                          const std::vector<std::shared_ptr<Type>>& argumentTypes) const {
    return lookupFunction(name); // 简化实现
}

// 泛型支持
void SymbolTable::defineGeneric(const std::string& name, const std::vector<std::string>& constraints) {
    // 简化实现
}

void SymbolTable::enterGenericScope(const std::vector<std::string>& genericParameters) {
    genericParameterStack.push_back(genericParameters);
}

void SymbolTable::exitGenericScope() {
    if (!genericParameterStack.empty()) {
        genericParameterStack.pop_back();
    }
}

bool SymbolTable::isGenericParameter(const std::string& name) const {
    for (const auto& params : genericParameterStack) {
        if (std::find(params.begin(), params.end(), name) != params.end()) {
            return true;
        }
    }
    return false;
}

// 模块支持
void SymbolTable::importModule(const std::string& moduleName) {
    importedModules[moduleName] = std::vector<std::string>();
}

void SymbolTable::exportSymbol(const std::string& name) {
    exportedSymbols.push_back(name);
}

std::vector<std::string> SymbolTable::getExportedSymbols() const {
    return exportedSymbols;
}

// 调试和诊断
void SymbolTable::printSymbolTable() const {
    // 简化实现
}

void SymbolTable::printCurrentScope() const {
    // 简化实现
}

std::vector<std::string> SymbolTable::getUndeclaredVariables() const {
    return std::vector<std::string>(); // 简化实现
}

std::vector<std::string> SymbolTable::getUnusedVariables() const {
    return std::vector<std::string>(); // 简化实现
}

// 符号使用标记
void SymbolTable::markVariableAsUsed(const std::string& name) {
    auto var = lookupVariable(name);
    if (var) {
        var->isUsed = true;
    }
}

void SymbolTable::markVariableAsInitialized(const std::string& name) {
    auto var = lookupVariable(name);
    if (var) {
        var->isInitialized = true;
    }
}

// 内置类型和函数初始化
void SymbolTable::initializeBuiltinTypes() {
    // 简化实现
}

void SymbolTable::initializeBuiltinFunctions() {
    // 简化实现
}

std::shared_ptr<Symbol> SymbolTable::createBuiltinTypeSymbol(const std::string& name, std::shared_ptr<Type> type) {
    return std::make_shared<TypeSymbol>(name, SymbolKind::Type, type);
}

std::shared_ptr<Symbol> SymbolTable::createBuiltinFunctionSymbol(const std::string& name, std::shared_ptr<FunctionType> type) {
    return std::make_shared<FunctionSymbol>(name, type);
}

} // namespace miniswift