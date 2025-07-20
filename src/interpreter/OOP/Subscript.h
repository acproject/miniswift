#ifndef MINISWIFT_SUBSCRIPT_H
#define MINISWIFT_SUBSCRIPT_H

#include "../Value.h"
#include "../Environment.h"
#include "../../parser/StmtNodes.h"
#include <vector>
#include <memory>
#include <functional>

namespace miniswift {

// Forward declarations
class Interpreter;
struct Parameter;

// Subscript definition structure
struct SubscriptDefinition {
    std::vector<Parameter> parameters;  // Subscript parameters (indices)
    Token returnType;                   // Return type of subscript
    std::unique_ptr<BlockStmt> getter;  // Getter implementation
    std::unique_ptr<BlockStmt> setter;  // Setter implementation (optional)
    bool isReadOnly;                    // Whether subscript is read-only
    
    SubscriptDefinition(std::vector<Parameter> params, 
                       Token retType,
                       std::unique_ptr<BlockStmt> getterBody,
                       std::unique_ptr<BlockStmt> setterBody = nullptr)
        : parameters(std::move(params))
        , returnType(retType)
        , getter(std::move(getterBody))
        , setter(std::move(setterBody))
        , isReadOnly(setter == nullptr) {}
};

// Subscript value for runtime representation
class SubscriptValue {
public:
    SubscriptDefinition definition;
    std::shared_ptr<Environment> closure;  // Closure environment
    
    SubscriptValue(SubscriptDefinition def, std::shared_ptr<Environment> env)
        : definition(std::move(def)), closure(env) {}
    
    // Copy constructor
    SubscriptValue(const SubscriptValue& other)
        : definition(other.definition.parameters, other.definition.returnType,
                    other.definition.getter ? std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(other.definition.getter->clone().release())) : nullptr,
                    other.definition.setter ? std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(other.definition.setter->clone().release())) : nullptr),
          closure(other.closure) {}
    
    // Execute subscript getter
    Value get(Interpreter& interpreter, const std::vector<Value>& indices, const Value* instance = nullptr);
    
    // Execute subscript setter
    void set(Interpreter& interpreter, const std::vector<Value>& indices, const Value& newValue, Value* instance = nullptr);
    
    // Check if subscript matches given parameter types
    bool matchesSignature(const std::vector<Value>& indices) const;
};

// Subscript manager for types (structs/classes)
class SubscriptManager {
private:
    std::vector<std::unique_ptr<SubscriptValue>> subscripts;
    
public:
    // Add a subscript definition
    void addSubscript(std::unique_ptr<SubscriptValue> subscript);
    
    // Find matching subscript for given indices
    SubscriptValue* findSubscript(const std::vector<Value>& indices);
    
    // Get value using subscript
    Value getValue(Interpreter& interpreter, const std::vector<Value>& indices);
    
    // Set value using subscript
    void setValue(Interpreter& interpreter, const std::vector<Value>& indices, const Value& newValue);
    
    // Check if any subscripts are defined
    bool hasSubscripts() const { return !subscripts.empty(); }
    
    // Get number of subscripts
    size_t getSubscriptCount() const { return subscripts.size(); }
    
    // Get all subscripts (for copying)
    const std::vector<std::unique_ptr<SubscriptValue>>& getAllSubscripts() const { return subscripts; }
};

// Static subscript manager for type-level subscripts
class StaticSubscriptManager {
private:
    std::unordered_map<std::string, std::unique_ptr<SubscriptManager>> typeSubscripts;
    
public:
    // Get or create subscript manager for a type
    SubscriptManager* getSubscriptManager(const std::string& typeName);
    
    // Register a static subscript for a type
    void registerStaticSubscript(const std::string& typeName, std::unique_ptr<SubscriptValue> subscript);
    
    // Access static subscript
    Value getStaticValue(const std::string& typeName, Interpreter& interpreter, const std::vector<Value>& indices);
    void setStaticValue(const std::string& typeName, Interpreter& interpreter, const std::vector<Value>& indices, const Value& newValue);
};

// Built-in subscripts for standard types
class BuiltinSubscripts {
public:
    // Register built-in subscripts for arrays, dictionaries, etc.
    static void registerBuiltins(StaticSubscriptManager& manager);
    
    // Array subscript implementation
    static Value arrayGet(const Value& array, const Value& index);
    static void arraySet(Value& array, const Value& index, const Value& newValue);
    
    // Dictionary subscript implementation
    static Value dictionaryGet(const Value& dictionary, const Value& key);
    static void dictionarySet(Value& dictionary, const Value& key, const Value& newValue);
    
    // String subscript implementation (character access)
    static Value stringGet(const Value& string, const Value& index);
};

} // namespace miniswift

#endif // MINISWIFT_SUBSCRIPT_H