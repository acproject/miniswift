#ifndef MINISWIFT_OPTIONAL_H
#define MINISWIFT_OPTIONAL_H

#include "../Value.h"
#include <memory>

namespace miniswift {

// Forward declarations
class Interpreter;
struct Call;

// Optional utility class for handling optional chaining operations
class OptionalManager {
public:
    // Create an optional value from a regular value
    static Value createOptional(const Value& value);
    
    // Create a nil optional
    static Value createNilOptional();
    
    // Create a nil value (alias for createNilOptional)
    static Value createNil();
    
    // Check if a value is nil (either Nil type or Optional with no value)
    static bool isNil(const Value& value);
    
    // Unwrap an optional value (returns the wrapped value or throws if nil)
    static Value unwrap(const Value& optionalValue);
    
    // Safe unwrap - returns the wrapped value or nil if the optional is nil
    static Value safeUnwrap(const Value& optionalValue);
    
    // Perform optional chaining on property access
    static Value chainPropertyAccess(const Value& object, const std::string& propertyName);
    
    // Perform optional chaining on method call
    static Value chainMethodCall(const Value& object, const std::string& methodName, const std::vector<Value>& arguments);
    
    // Perform optional chaining on subscript access
    static Value chainSubscriptAccess(const Value& object, const std::vector<Value>& indices);
    
    // Chain property access with interpreter context
    static Value chainProperty(const Value& object, const std::string& propertyName, Interpreter& interpreter);
    
    // Chain method call with interpreter context
    static Value chainMethod(const Value& object, const struct Call* callExpr, Interpreter& interpreter);
    
    // Chain subscript access with interpreter context
    static Value chainSubscript(const Value& object, const Value& index, Interpreter& interpreter);
    
    // Convert a regular value to optional if it's not already optional
    static Value makeOptional(const Value& value);
    
    // Nil coalescing operator (??) implementation
    static Value nilCoalescing(const Value& optional, const Value& defaultValue);
    
private:
    // Helper method to access properties on objects
    static Value accessProperty(const Value& object, const std::string& propertyName);
    
    // Helper method to call methods on objects
    static Value callMethod(const Value& object, const std::string& methodName, const std::vector<Value>& arguments);
    
    // Helper method to access subscripts on objects
    static Value accessSubscript(const Value& object, const std::vector<Value>& indices);
};

} // namespace miniswift

#endif // MINISWIFT_OPTIONAL_H