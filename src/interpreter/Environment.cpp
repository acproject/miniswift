#include "Environment.h"
#include <stdexcept>

namespace miniswift {

Environment::Environment() : enclosing(nullptr) {}

Environment::Environment(std::shared_ptr<Environment> enclosing) : enclosing(enclosing) {}

void Environment::define(const std::string& name, const Value& value, bool isConst, const std::string& typeName) {
    values[name] = {value, isConst, typeName};
}

void Environment::define(const std::string& name, const Value& value) {
    values[name] = {value, false, ""}; // Default: mutable, no type annotation
}

Value Environment::get(const Token& name) {
    if (values.count(name.lexeme)) {
        return values.at(name.lexeme).value;
    }

    if (enclosing != nullptr) {
        return enclosing->get(name);
    }

    throw std::runtime_error("Undefined variable '" + name.lexeme + "'.");
}

Value& Environment::getReference(const Token& name) {
    if (values.count(name.lexeme)) {
        return values.at(name.lexeme).value;
    }

    if (enclosing != nullptr) {
        return enclosing->getReference(name);
    }

    throw std::runtime_error("Undefined variable '" + name.lexeme + "'.");
}

void Environment::assign(const Token& name, const Value& value) {
    auto it = values.find(name.lexeme);
    if (it != values.end()) {
        if (it->second.isConst) {
            throw std::runtime_error("Cannot assign to value: '" + name.lexeme + "' is a 'let' constant.");
        }
        if (!it->second.typeName.empty()) { // If the variable has a type annotation
            std::string valueTypeName;
            switch (value.type) {
                case ValueType::Int:    valueTypeName = "Int";    break;
                case ValueType::Double: valueTypeName = "Double"; break;
                case ValueType::String: valueTypeName = "String"; break;
                case ValueType::Bool:   valueTypeName = "Bool";   break;
                case ValueType::Nil:    valueTypeName = "Nil";    break;
                case ValueType::Array:  valueTypeName = "Array";  break;
                case ValueType::Dictionary: valueTypeName = "Dictionary"; break;
                case ValueType::Function: valueTypeName = "Function"; break;
                case ValueType::Enum:   valueTypeName = "Enum";   break;
                case ValueType::Struct: valueTypeName = "Struct"; break;
                case ValueType::Class:  valueTypeName = "Class";  break;
                case ValueType::Constructor: valueTypeName = "Constructor"; break;
                case ValueType::Destructor: valueTypeName = "Destructor"; break;
                
                // Extended Integer Types
                case ValueType::Int8:   valueTypeName = "Int8";   break;
                case ValueType::Int16:  valueTypeName = "Int16";  break;
                case ValueType::Int32:  valueTypeName = "Int32";  break;
                case ValueType::Int64:  valueTypeName = "Int64";  break;
                case ValueType::UInt:   valueTypeName = "UInt";   break;
                case ValueType::UInt8:  valueTypeName = "UInt8";  break;
                case ValueType::UInt16: valueTypeName = "UInt16"; break;
                case ValueType::UInt64: valueTypeName = "UInt64"; break;
                
                // Additional Basic Types
                case ValueType::Float:     valueTypeName = "Float";     break;
                case ValueType::Character: valueTypeName = "Character"; break;
                
                // Special Types
                case ValueType::Set:  valueTypeName = "Set";  break;
                case ValueType::Any:  valueTypeName = "Any";  break;
                case ValueType::Void: valueTypeName = "Void"; break;
            }

            // Special case: assigning a Double to an Int variable
            if (it->second.typeName == "Int" && value.type == ValueType::Double) {
                double val = std::get<double>(value.value);
                if (val != static_cast<long long>(val)) { // Check if it's a whole number
                    throw std::runtime_error("Cannot assign non-integer value to variable of type 'Int'.");
                }
            } else if (it->second.typeName != valueTypeName) {
                // General type mismatch
                throw std::runtime_error("Cannot assign value of type '" + valueTypeName + "' to variable of type '" + it->second.typeName + "'.");
            }
        }
        it->second.value = value;
        return;
    }

    if (enclosing != nullptr) {
        enclosing->assign(name, value);
        return;
    }

    throw std::runtime_error("Undefined variable '" + name.lexeme + "'.");
}

bool Environment::exists(const std::string& name) const {
    if (values.count(name)) {
        return true;
    }
    
    if (enclosing != nullptr) {
        return enclosing->exists(name);
    }
    
    return false;
}

} // namespace miniswift