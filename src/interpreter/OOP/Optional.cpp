#include "Optional.h"
#include "Property.h"
#include "Method.h"
#include "Subscript.h"
#include "../Interpreter.h"
#include "../../parser/ExprNodes.h"
#include <stdexcept>
#include <iostream>

namespace miniswift {

Value OptionalManager::createOptional(const Value& value) {
    return Value(OptionalValue(value));
}

Value OptionalManager::createNilOptional() {
    return Value(OptionalValue());
}

Value OptionalManager::createNil() {
    return createNilOptional();
}

bool OptionalManager::isNil(const Value& value) {
    if (value.type == ValueType::Nil) {
        return true;
    }
    if (value.type == ValueType::Optional) {
        return !value.asOptional().hasValue;
    }
    return false;
}

Value OptionalManager::unwrap(const Value& optionalValue) {
    if (optionalValue.type == ValueType::Optional) {
        const auto& optional = optionalValue.asOptional();
        if (optional.hasValue && optional.wrappedValue) {
            return *optional.wrappedValue;
        } else {
            throw std::runtime_error("Fatal error: Unexpectedly found nil while unwrapping an Optional value");
        }
    }
    // If it's not an optional, return as is
    return optionalValue;
}

Value OptionalManager::safeUnwrap(const Value& optionalValue) {
    if (optionalValue.isOptional()) {
        const auto& opt = optionalValue.asOptional();
        if (opt.hasValue && opt.wrappedValue) {
            return *opt.wrappedValue;
        }
        return createNil();
    }
    // If it's not an optional, return as is
    return optionalValue;
}

Value OptionalManager::chainPropertyAccess(const Value& object, const std::string& propertyName) {
    // If the object is nil, return nil
    if (isNil(object)) {
        return createNilOptional();
    }
    
    // Unwrap the object if it's optional
    Value unwrappedObject = safeUnwrap(object);
    if (isNil(unwrappedObject)) {
        return createNilOptional();
    }
    
    try {
        // Access the property
        Value result = accessProperty(unwrappedObject, propertyName);
        // Wrap the result in an optional
        return createOptional(result);
    } catch (const std::exception&) {
        // If property access fails, return nil
        return createNilOptional();
    }
}

Value OptionalManager::chainMethodCall(const Value& object, const std::string& methodName, const std::vector<Value>& arguments) {
    // If the object is nil, return nil
    if (isNil(object)) {
        return createNilOptional();
    }
    
    // Unwrap the object if it's optional
    Value unwrappedObject = safeUnwrap(object);
    if (isNil(unwrappedObject)) {
        return createNilOptional();
    }
    
    try {
        // Call the method
        Value result = callMethod(unwrappedObject, methodName, arguments);
        // Wrap the result in an optional
        return createOptional(result);
    } catch (const std::exception&) {
        // If method call fails, return nil
        return createNilOptional();
    }
}

Value OptionalManager::chainSubscriptAccess(const Value& object, const std::vector<Value>& indices) {
    // If the object is nil, return nil
    if (isNil(object)) {
        return createNilOptional();
    }
    
    // Unwrap the object if it's optional
    Value unwrappedObject = safeUnwrap(object);
    if (isNil(unwrappedObject)) {
        return createNilOptional();
    }
    
    try {
        // Access the subscript
        Value result = accessSubscript(unwrappedObject, indices);
        // Wrap the result in an optional
        return createOptional(result);
    } catch (const std::exception&) {
        // If subscript access fails, return nil
        return createNilOptional();
    }
}

Value OptionalManager::makeOptional(const Value& value) {
    if (value.type == ValueType::Optional) {
        return value; // Already optional
    }
    return createOptional(value);
}

Value OptionalManager::nilCoalescing(const Value& optional, const Value& defaultValue) {
    if (isNil(optional)) {
        return defaultValue;
    }
    return safeUnwrap(optional);
}

// Helper methods (these would need to be implemented based on your existing property/method/subscript systems)
Value OptionalManager::accessProperty(const Value& object, const std::string& propertyName) {
    // This is a placeholder - you'll need to integrate with your existing property system
    if (object.isStruct()) {
        auto& structValue = object.asStruct();
        if (structValue.properties) {
            // Use your existing property access logic
            // This is just a placeholder implementation
            throw std::runtime_error("Property access not implemented yet");
        }
    } else if (object.isClass()) {
        auto& classValue = object.asClass();
        if (classValue->properties) {
            // Use your existing property access logic
            // This is just a placeholder implementation
            throw std::runtime_error("Property access not implemented yet");
        }
    }
    throw std::runtime_error("Property '" + propertyName + "' not found");
}

Value OptionalManager::callMethod(const Value& object, const std::string& methodName, const std::vector<Value>& arguments) {
    (void)object;
    (void)methodName;
    (void)arguments;
    // This is a placeholder - you'll need to integrate with your existing method system
    throw std::runtime_error("Method call not implemented yet");
}

Value OptionalManager::accessSubscript(const Value& object, const std::vector<Value>& indices) {
    // This is a placeholder - you'll need to integrate with your existing subscript system
    if (object.isArray() && indices.size() == 1) {
        // Simple array access
        auto& array = object.asArrayRef();
        if (indices[0].type == ValueType::Int) {
            int index = std::get<int>(indices[0].value);
            if (index >= 0 && index < static_cast<int>(array.size())) {
                return array[index];
            }
        }
    } else if (object.isDictionary() && indices.size() == 1) {
        // Simple dictionary access
        auto& dict = object.asDictionaryRef();
        if (indices[0].type == ValueType::String) {
            std::string key = std::get<std::string>(indices[0].value);
            auto it = dict.find(key);
            if (it != dict.end()) {
                return it->second;
            }
        }
    }
    throw std::runtime_error("Subscript access failed");
}

// Interpreter integration methods
Value OptionalManager::chainProperty(const Value& object, const std::string& propertyName, Interpreter& interpreter) {
    // If the object is nil, return nil
    if (isNil(object)) {
        return createNil();
    }
    
    // If the object is optional, unwrap it first
    Value unwrappedObject = object;
    if (object.isOptional()) {
        unwrappedObject = safeUnwrap(object);
        if (isNil(unwrappedObject)) {
            return createNil();
        }
    }
    
    try {
        // Use interpreter's member access logic
        Value result = interpreter.getMemberValue(unwrappedObject, propertyName);
        // Return the result directly (don't wrap in optional here, the caller will handle it)
        return result;
    } catch (const std::exception& e) {
        // If property access fails, return nil
        return createNil();
    }
}

Value OptionalManager::chainMethod(const Value& object, const Call* callExpr, Interpreter& interpreter) {
    // If the object is nil, return nil
    if (isNil(object)) {
        return createNil();
    }
    
    // The object should already be unwrapped by the caller, so use it directly
    try {
        // Use interpreter's call logic
        Value result = interpreter.evaluate(*callExpr);
        // Return the result directly (don't wrap in optional here, the caller will handle it)
        return result;
    } catch (const std::exception&) {
        // If method call fails, return nil
        return createNil();
    }
}

Value OptionalManager::chainSubscript(const Value& object, const Value& index, Interpreter& interpreter) {
    (void)interpreter;
    // If the object is nil, return nil
    if (isNil(object)) {
        return createNil();
    }
    
    // The object should already be unwrapped by the caller, so use it directly
    try {
        // Create a temporary IndexAccess expression for evaluation
        // This is a simplified approach - in a real implementation you might want to
        // directly access the subscript logic
        if (object.isArray() && index.type == ValueType::Int) {
            auto& array = object.asArrayRef();
            int idx = std::get<int>(index.value);
            if (idx >= 0 && idx < static_cast<int>(array.size())) {
                return array[idx];
            }
        } else if (object.isDictionary() && index.type == ValueType::String) {
            auto& dict = object.asDictionaryRef();
            std::string key = std::get<std::string>(index.value);
            auto it = dict.find(key);
            if (it != dict.end()) {
                return it->second;
            }
        }
        // If subscript access fails, return nil
        return createNil();
    } catch (const std::exception&) {
        // If subscript access fails, return nil
        return createNil();
    }
}

} // namespace miniswift