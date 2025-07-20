#include "Subscript.h"
#include "../Interpreter.h"
#include "../../parser/StmtNodes.h"
#include <stdexcept>
#include <iostream>

namespace miniswift {

// SubscriptValue implementation
Value SubscriptValue::get(Interpreter& interpreter, const std::vector<Value>& indices) {
    if (!getter) {
        throw std::runtime_error("Subscript has no getter");
    }
    
    // Create new environment for subscript execution
    auto previous = interpreter.getCurrentEnvironment();
    auto subscriptEnv = std::make_shared<Environment>(closure);
    
    try {
        // Bind parameters to indices
        if (indices.size() != parameters.size()) {
            throw std::runtime_error("Subscript parameter count mismatch");
        }
        
        for (size_t i = 0; i < parameters.size(); ++i) {
            subscriptEnv->define(parameters[i].name.lexeme, indices[i], false, parameters[i].type.lexeme);
        }
        
        interpreter.setCurrentEnvironment(subscriptEnv);
        
        // Execute getter body
        Value result;
        try {
            interpreter.executeWithEnvironment(*getter, subscriptEnv);
            // If no return statement, return nil
            result = Value();
        } catch (const std::runtime_error& e) {
            // Check if this is a return exception
            std::string errorMsg = e.what();
            if (errorMsg == "return") {
                // This is a return statement, get the returned value
                result = interpreter.evaluate(*getter->statements[0]); // Simplified for now
            } else {
                throw;
            }
        }
        
        interpreter.setCurrentEnvironment(previous);
        return result;
        
    } catch (...) {
        interpreter.setCurrentEnvironment(previous);
        throw;
    }
}

void SubscriptValue::set(Interpreter& interpreter, const std::vector<Value>& indices, const Value& newValue) {
    if (isReadOnly || !setter) {
        throw std::runtime_error("Subscript is read-only");
    }
    
    // Create new environment for subscript execution
    auto previous = interpreter.getCurrentEnvironment();
    auto subscriptEnv = std::make_shared<Environment>(closure);
    
    try {
        // Bind parameters to indices
        if (indices.size() != parameters.size()) {
            throw std::runtime_error("Subscript parameter count mismatch");
        }
        
        for (size_t i = 0; i < parameters.size(); ++i) {
            subscriptEnv->define(parameters[i].name.lexeme, indices[i], false, parameters[i].type.lexeme);
        }
        
        // Bind newValue to special parameter
        subscriptEnv->define("newValue", newValue, false, "");
        
        interpreter.setCurrentEnvironment(subscriptEnv);
        
        // Execute setter body
        interpreter.executeWithEnvironment(*setter, subscriptEnv);
        
        interpreter.setCurrentEnvironment(previous);
        
    } catch (...) {
        interpreter.setCurrentEnvironment(previous);
        throw;
    }
}

bool SubscriptValue::matchesSignature(const std::vector<Value>& indices) const {
    if (indices.size() != parameters.size()) {
        return false;
    }
    
    // For now, we'll do basic type checking
    // In a full implementation, this would include more sophisticated type matching
    for (size_t i = 0; i < parameters.size(); ++i) {
        const auto& param = parameters[i];
        const auto& index = indices[i];
        
        // Skip type checking if parameter type is empty (type inference)
        if (param.type.lexeme.empty()) {
            continue;
        }
        
        // Basic type matching
        if (param.type.lexeme == "Int" && index.type != ValueType::Int) {
            return false;
        }
        if (param.type.lexeme == "String" && index.type != ValueType::String) {
            return false;
        }
        if (param.type.lexeme == "Double" && index.type != ValueType::Double) {
            return false;
        }
        if (param.type.lexeme == "Bool" && index.type != ValueType::Bool) {
            return false;
        }
    }
    
    return true;
}

// SubscriptManager implementation
void SubscriptManager::addSubscript(std::unique_ptr<SubscriptValue> subscript) {
    subscripts.push_back(std::move(subscript));
}

SubscriptValue* SubscriptManager::findSubscript(const std::vector<Value>& indices) {
    for (auto& subscript : subscripts) {
        if (subscript->matchesSignature(indices)) {
            return subscript.get();
        }
    }
    return nullptr;
}

Value SubscriptManager::getValue(Interpreter& interpreter, const std::vector<Value>& indices) {
    auto* subscript = findSubscript(indices);
    if (!subscript) {
        throw std::runtime_error("No matching subscript found for given indices");
    }
    
    return subscript->get(interpreter, indices);
}

void SubscriptManager::setValue(Interpreter& interpreter, const std::vector<Value>& indices, const Value& newValue) {
    auto* subscript = findSubscript(indices);
    if (!subscript) {
        throw std::runtime_error("No matching subscript found for given indices");
    }
    
    subscript->set(interpreter, indices, newValue);
}

// StaticSubscriptManager implementation
SubscriptManager* StaticSubscriptManager::getSubscriptManager(const std::string& typeName) {
    auto it = typeSubscripts.find(typeName);
    if (it == typeSubscripts.end()) {
        typeSubscripts[typeName] = std::make_unique<SubscriptManager>();
    }
    return typeSubscripts[typeName].get();
}

void StaticSubscriptManager::registerStaticSubscript(const std::string& typeName, std::unique_ptr<SubscriptValue> subscript) {
    auto* manager = getSubscriptManager(typeName);
    manager->addSubscript(std::move(subscript));
}

Value StaticSubscriptManager::getStaticValue(const std::string& typeName, Interpreter& interpreter, const std::vector<Value>& indices) {
    auto it = typeSubscripts.find(typeName);
    if (it == typeSubscripts.end()) {
        throw std::runtime_error("No static subscripts defined for type: " + typeName);
    }
    
    return it->second->getValue(interpreter, indices);
}

void StaticSubscriptManager::setStaticValue(const std::string& typeName, Interpreter& interpreter, const std::vector<Value>& indices, const Value& newValue) {
    auto it = typeSubscripts.find(typeName);
    if (it == typeSubscripts.end()) {
        throw std::runtime_error("No static subscripts defined for type: " + typeName);
    }
    
    it->second->setValue(interpreter, indices, newValue);
}

// BuiltinSubscripts implementation
void BuiltinSubscripts::registerBuiltins(StaticSubscriptManager& manager) {
    // Note: Built-in subscripts for arrays and dictionaries are handled
    // directly in the IndexAccess visitor for performance reasons
    // This method is reserved for future built-in type subscripts
}

Value BuiltinSubscripts::arrayGet(const Value& array, const Value& index) {
    if (array.type != ValueType::Array) {
        throw std::runtime_error("Expected array type");
    }
    
    if (index.type != ValueType::Int) {
        throw std::runtime_error("Array index must be an integer");
    }
    
    int idx = std::get<int>(index.value);
    const auto& arr = *array.asArray();
    
    if (idx < 0 || idx >= static_cast<int>(arr.size())) {
        throw std::runtime_error("Array index out of bounds");
    }
    
    return arr[idx];
}

void BuiltinSubscripts::arraySet(Value& array, const Value& index, const Value& newValue) {
    if (array.type != ValueType::Array) {
        throw std::runtime_error("Expected array type");
    }
    
    if (index.type != ValueType::Int) {
        throw std::runtime_error("Array index must be an integer");
    }
    
    int idx = std::get<int>(index.value);
    auto& arr = *array.asArray();
    
    if (idx < 0 || idx >= static_cast<int>(arr.size())) {
        throw std::runtime_error("Array index out of bounds");
    }
    
    arr[idx] = newValue;
}

Value BuiltinSubscripts::dictionaryGet(const Value& dictionary, const Value& key) {
    if (dictionary.type != ValueType::Dictionary) {
        throw std::runtime_error("Expected dictionary type");
    }
    
    std::string keyStr;
    switch (key.type) {
        case ValueType::String:
            keyStr = std::get<std::string>(key.value);
            break;
        case ValueType::Int:
            keyStr = std::to_string(std::get<int>(key.value));
            break;
        case ValueType::Double:
            keyStr = std::to_string(std::get<double>(key.value));
            break;
        default:
            throw std::runtime_error("Dictionary key must be a string or number");
    }
    
    const auto& dict = *dictionary.asDictionary();
    auto it = dict.find(keyStr);
    if (it != dict.end()) {
        return it->second;
    } else {
        return Value(); // nil for missing keys
    }
}

void BuiltinSubscripts::dictionarySet(Value& dictionary, const Value& key, const Value& newValue) {
    if (dictionary.type != ValueType::Dictionary) {
        throw std::runtime_error("Expected dictionary type");
    }
    
    std::string keyStr;
    switch (key.type) {
        case ValueType::String:
            keyStr = std::get<std::string>(key.value);
            break;
        case ValueType::Int:
            keyStr = std::to_string(std::get<int>(key.value));
            break;
        case ValueType::Double:
            keyStr = std::to_string(std::get<double>(key.value));
            break;
        default:
            throw std::runtime_error("Dictionary key must be a string or number");
    }
    
    auto& dict = *dictionary.asDictionary();
    dict[keyStr] = newValue;
}

Value BuiltinSubscripts::stringGet(const Value& string, const Value& index) {
    if (string.type != ValueType::String) {
        throw std::runtime_error("Expected string type");
    }
    
    if (index.type != ValueType::Int) {
        throw std::runtime_error("String index must be an integer");
    }
    
    int idx = std::get<int>(index.value);
    const std::string& str = std::get<std::string>(string.value);
    
    if (idx < 0 || idx >= static_cast<int>(str.length())) {
        throw std::runtime_error("String index out of bounds");
    }
    
    // Return single character as string
    return Value(std::string(1, str[idx]));
}

} // namespace miniswift