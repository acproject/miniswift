#include "Interpreter.h"
#include "OOP/Property.h"
#include "OOP/Constructor.h"
#include "OOP/Method.h"
#include <stdexcept>
#include <iostream>

// Exception class for handling return statements
class ReturnException : public std::runtime_error {
public:
    miniswift::Value value;
    ReturnException(miniswift::Value val) : std::runtime_error("return"), value(val) {}
};

namespace miniswift {

Interpreter::Interpreter() {
    globals = std::make_shared<Environment>();
    environment = globals;
    
    // Initialize inheritance management
    inheritanceManager = std::make_unique<InheritanceManager>();
    superHandler = std::make_unique<SuperHandler>(*inheritanceManager, *this);
    
    // Initialize subscript management
    staticSubscriptManager = std::make_unique<StaticSubscriptManager>();
    
    // Initialize error handling management
    errorPropagator = std::make_unique<ErrorPropagator>();
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
    // std::cout << "Starting interpretation with " << statements.size() << " statements" << std::endl;
    try {
        for (const auto& statement : statements) {
            statement->accept(*this);
        }
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << std::endl;
    }
}

void Interpreter::visit(const ExprStmt& stmt) {
    evaluate(*stmt.expression);
}

void Interpreter::visit(const PrintStmt& stmt) {
    // Handle multiple expressions
    for (size_t i = 0; i < stmt.expressions.size(); ++i) {
        if (i > 0) {
            std::cout << " "; // Add space between arguments
        }
        Value val = evaluate(*stmt.expressions[i]);
        
        // Print value without newline (except for the last one)
        printValueInline(val);
    }
    std::cout << std::endl; // Add newline at the end
}

void Interpreter::printValueInline(const Value& val) {
    switch (val.type) {
        case ValueType::Int:
            std::cout << std::get<int>(val.value);
            break;
        case ValueType::Double:
            std::cout << std::get<double>(val.value);
            break;
        case ValueType::Bool:
            std::cout << (std::get<bool>(val.value) ? "true" : "false");
            break;
        case ValueType::String:
            std::cout << std::get<std::string>(val.value);
            break;
        case ValueType::Array:
            printArrayInline(val.asArray());
            break;
        case ValueType::Dictionary:
            printDictionaryInline(val.asDictionary());
            break;
        case ValueType::Tuple:
            printTupleInline(val.asTuple());
            break;
        case ValueType::Nil:
            std::cout << "nil";
            break;
        case ValueType::Function:
            if (val.isClosure()) {
                std::cout << "<closure>";
            } else {
                std::cout << "<function>";
            }
            break;
        case ValueType::Enum: {
            const auto& enumVal = val.asEnum();
            std::cout << enumVal->enumName << "." << enumVal->caseName;
            if (!enumVal->associatedValues.empty()) {
                std::cout << "(";
                for (size_t i = 0; i < enumVal->associatedValues.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    printValueInline(enumVal->associatedValues[i]);
                }
                std::cout << ")";
            }
            break;
        }
        case ValueType::Struct: {
            const auto& structVal = val.asStruct();
            std::cout << structVal.structName << "(";
            bool first = true;
            for (const auto& member : *structVal.members) {
                if (!first) std::cout << ", ";
                std::cout << member.first << ": ";
                printValueInline(member.second);
                first = false;
            }
            std::cout << ")";
            break;
        }
        case ValueType::Class: {
            const auto& classVal = val.asClass();
            std::cout << classVal->className << "(";
            bool first = true;
            for (const auto& member : *classVal->members) {
                if (!first) std::cout << ", ";
                std::cout << member.first << ": ";
                printValueInline(member.second);
                first = false;
            }
            std::cout << ")";
            break;
        }
        case ValueType::Constructor:
            std::cout << "<constructor>";
            break;
        case ValueType::Destructor:
            std::cout << "<destructor>";
            break;
        case ValueType::Optional: {
            const auto& optionalVal = val.asOptional();
            if (optionalVal.hasValue && optionalVal.wrappedValue) {
                std::cout << "Optional(";
                printValueInline(*optionalVal.wrappedValue);
                std::cout << ")";
            } else {
                std::cout << "nil";
            }
            break;
        }
        case ValueType::Error: {
            const auto& errorVal = val.asError();
            std::cout << "Error(" << errorVal.getDescription() << ")";
            break;
        }
        case ValueType::Result: {
            auto resultPtr = val.asResult();
            if (resultPtr->isSuccess) {
                std::cout << "Result.success(";
                printValueInline(resultPtr->getValue());
                std::cout << ")";
            } else {
                std::cout << "Result.failure(" << resultPtr->getError().getDescription() << ")";
            }
            break;
        }
        
        // Extended Integer Types
        case ValueType::Int8:
            std::cout << static_cast<int>(std::get<int8_t>(val.value));
            break;
        case ValueType::Int16:
            std::cout << std::get<int16_t>(val.value);
            break;
        case ValueType::Int32:
            std::cout << std::get<int>(val.value);
            break;
        case ValueType::Int64:
            std::cout << std::get<int64_t>(val.value);
            break;
        case ValueType::UInt:
            std::cout << std::get<uint32_t>(val.value);
            break;
        case ValueType::UInt8:
            std::cout << static_cast<unsigned int>(std::get<uint8_t>(val.value));
            break;
        case ValueType::UInt16:
            std::cout << std::get<uint16_t>(val.value);
            break;
        case ValueType::UInt64:
            std::cout << std::get<uint64_t>(val.value);
            break;
        
        // Additional Basic Types
        case ValueType::Float:
            std::cout << std::get<float>(val.value);
            break;
        case ValueType::Character:
            std::cout << "'" << static_cast<unsigned char>(std::get<char>(val.value)) << "'";
            break;
        
        // Special Types
        case ValueType::Set:
            std::cout << "<set>";
            break;
        case ValueType::Any:
            std::cout << "<any>";
            break;
        case ValueType::Void:
            std::cout << "()";
            break;
    }
}

void Interpreter::visit(const VarStmt& stmt) {
    Value value; // Default-initialized (nil)
    if (stmt.initializer) {
        value = evaluate(*stmt.initializer);
        
        // Check if the variable type is optional and wrap the value if needed
        if (!stmt.type.lexeme.empty() && stmt.type.lexeme.back() == '?' && value.type != ValueType::Optional) {
            value = OptionalManager::createOptional(value);
        }
    }
    environment->define(stmt.name.lexeme, value, stmt.isConst, stmt.type.lexeme);
}

void Interpreter::visit(const VarExpr& expr) {
    try {
        result = environment->get(expr.name);
    } catch (const std::runtime_error&) {
        // If variable not found, try to find it as a member of 'self'
        try {
            Value selfValue = environment->get(Token{TokenType::Identifier, "self", 0});
            std::cout << "DEBUG: Found self object, trying to access property: " << expr.name.lexeme << std::endl;
            if (selfValue.isStruct()) {
                auto& structValue = selfValue.asStruct();
                // Try property system first
                if (structValue.properties && structValue.properties->hasProperty(expr.name.lexeme)) {
                    std::cout << "DEBUG: Found property in struct property system" << std::endl;
                    result = structValue.properties->getProperty(*this, expr.name.lexeme);
                    return;
                }
                // Fallback to legacy member access
                auto it = structValue.members->find(expr.name.lexeme);
                if (it != structValue.members->end()) {
                    std::cout << "DEBUG: Found property in struct legacy members" << std::endl;
                    result = it->second;
                    return;
                }
                std::cout << "DEBUG: Property not found in struct" << std::endl;
            } else if (selfValue.isClass()) {
                auto& classValue = selfValue.asClass();
                std::cout << "DEBUG: Self is a class instance: " << classValue->className << std::endl;
                // Try property system first
                if (classValue->properties && classValue->properties->hasProperty(expr.name.lexeme)) {
                    std::cout << "DEBUG: Found property in class property system" << std::endl;
                    result = classValue->properties->getProperty(*this, expr.name.lexeme);
                    return;
                }
                // Fallback to legacy member access
                auto it = classValue->members->find(expr.name.lexeme);
                if (it != classValue->members->end()) {
                    std::cout << "DEBUG: Found property in class legacy members" << std::endl;
                    result = it->second;
                    return;
                }
                std::cout << "DEBUG: Property '" << expr.name.lexeme << "' not found in class" << std::endl;
                if (classValue->properties) {
                    std::cout << "DEBUG: Available properties in class:" << std::endl;
                    auto availableProps = classValue->properties->getAllPropertyNames();
                    for (const auto& prop : availableProps) {
                        std::cout << "  - " << prop << std::endl;
                    }
                } else {
                    std::cout << "DEBUG: No property container found" << std::endl;
                }
            }
        } catch (const std::runtime_error& e) {
            std::cout << "DEBUG: Failed to get self object: " << e.what() << std::endl;
            // 'self' not found, continue with original error
        }
        
        // Re-throw original error if member not found in self
        throw std::runtime_error("Undefined variable '" + expr.name.lexeme + "'");
    }
}

void Interpreter::visit(const Assign& expr) {
    Value value = evaluate(*expr.value);
    
    // Check if target is a simple variable or member access
    if (auto varExpr = dynamic_cast<const VarExpr*>(expr.target.get())) {
        // Simple variable assignment
        try {
            environment->assign(varExpr->name, value);
        } catch (const std::runtime_error&) {
            // If variable not found, try to assign it as a member of 'self'
            try {
                Value& selfValue = environment->getReference(Token{TokenType::Identifier, "self", 0});
                if (selfValue.isStruct()) {
                    auto& structValue = selfValue.asStruct();
                    // Try property system first
                    if (structValue.properties && structValue.properties->hasProperty(varExpr->name.lexeme)) {
                        structValue.properties->setProperty(*this, varExpr->name.lexeme, value);
                    } else {
                        // Fallback to legacy member assignment
                        (*structValue.members)[varExpr->name.lexeme] = value;
                    }
                } else if (selfValue.isClass()) {
                    auto& classValue = selfValue.asClass();
                    // Try property system first
                    if (classValue->properties && classValue->properties->hasProperty(varExpr->name.lexeme)) {
                        classValue->properties->setProperty(*this, varExpr->name.lexeme, value);
                    } else {
                        // Fallback to legacy member assignment
                        (*classValue->members)[varExpr->name.lexeme] = value;
                    }
                } else {
                    throw std::runtime_error("Undefined variable '" + varExpr->name.lexeme + "'");
                }
            } catch (const std::runtime_error&) {
                // 'self' not found or member assignment failed, re-throw original error
                throw std::runtime_error("Undefined variable '" + varExpr->name.lexeme + "'");
            }
        }
    } else if (auto memberAccess = dynamic_cast<const MemberAccess*>(expr.target.get())) {
        // Member access assignment: object.member = value
        // For simple variable access, modify the original variable directly
        if (auto varExpr = dynamic_cast<const VarExpr*>(memberAccess->object.get())) {
            // Get reference to the original variable in environment
            Token varName = varExpr->name;
            Value& originalObject = environment->getReference(varName);
            
            if (originalObject.isStruct()) {
                auto& structValue = originalObject.asStruct();
                // Try property system first
                if (structValue.properties && structValue.properties->hasProperty(memberAccess->member.lexeme)) {
                    structValue.properties->setProperty(*this, memberAccess->member.lexeme, value);
                } else {
                    // Fallback to legacy member assignment
                    (*structValue.members)[memberAccess->member.lexeme] = value;
                }
            } else if (originalObject.isClass()) {
                auto& classValue = originalObject.asClass();
                // Try property system first
                if (classValue->properties && classValue->properties->hasProperty(memberAccess->member.lexeme)) {
                    classValue->properties->setProperty(*this, memberAccess->member.lexeme, value);
                } else {
                    // Fallback to legacy member assignment
                    (*classValue->members)[memberAccess->member.lexeme] = value;
                }
            } else {
                throw std::runtime_error("Only structs and classes have members");
            }
        } else {
            // For complex expressions, fall back to the old behavior
            Value object = evaluate(*memberAccess->object);
            
            if (object.isStruct()) {
                auto& structValue = object.asStruct();
                // Try property system first
                if (structValue.properties && structValue.properties->hasProperty(memberAccess->member.lexeme)) {
                    structValue.properties->setProperty(*this, memberAccess->member.lexeme, value);
                } else {
                    // Fallback to legacy member assignment
                    (*structValue.members)[memberAccess->member.lexeme] = value;
                }
            } else if (object.isClass()) {
                auto& classValue = object.asClass();
                // Try property system first
                if (classValue->properties && classValue->properties->hasProperty(memberAccess->member.lexeme)) {
                    classValue->properties->setProperty(*this, memberAccess->member.lexeme, value);
                } else {
                    // Fallback to legacy member assignment
                    (*classValue->members)[memberAccess->member.lexeme] = value;
                }
            } else {
                throw std::runtime_error("Only structs and classes have members");
            }
        }
    } else if (auto indexAccess = dynamic_cast<const IndexAccess*>(expr.target.get())) {
        // Array/Dictionary/Struct/Class index assignment: array[index] = value
        Value object = evaluate(*indexAccess->object);
        Value index = evaluate(*indexAccess->index);
        
        if (object.type == ValueType::Array) {
            auto& arr = object.asArray();
            if (index.type == ValueType::Int) {
                int idx = std::get<int>(index.value);
                if (idx >= 0 && idx < static_cast<int>(arr->size())) {
                    (*arr)[idx] = value;
                } else {
                    throw std::runtime_error("Array index out of bounds");
                }
            } else {
                throw std::runtime_error("Array index must be an integer");
            }
        } else if (object.type == ValueType::Dictionary) {
            auto& dict = object.asDictionary();
            if (index.type == ValueType::String) {
                std::string key = std::get<std::string>(index.value);
                (*dict)[key] = value;
            } else {
                throw std::runtime_error("Dictionary key must be a string");
            }
        } else if (object.type == ValueType::Class) {
            // Handle class subscript assignment
            auto classInstance = object.asClass();
            std::vector<Value> arguments = {index};
            
            // Try to find matching subscript in the instance's subscript manager
            auto subscriptValue = classInstance->subscripts->findSubscript(arguments);
            if (subscriptValue) {
                subscriptValue->set(*this, arguments, value, &object);
                return;
            }
            
            // If not found in instance, try static subscripts
            auto typeSubscriptManager = staticSubscriptManager->getSubscriptManager(classInstance->className);
            subscriptValue = typeSubscriptManager->findSubscript(arguments);
            if (subscriptValue) {
                subscriptValue->set(*this, arguments, value, &object);
                return;
            }
            
            throw std::runtime_error("No matching subscript found for class instance assignment");
        } else if (object.type == ValueType::Struct) {
            // Handle struct subscript assignment
            auto& structValue = object.asStruct();
            std::vector<Value> arguments = {index};
            
            // Try to find matching subscript in the struct's subscript manager
            auto subscriptValue = structValue.subscripts->findSubscript(arguments);
            if (subscriptValue) {
                subscriptValue->set(*this, arguments, value, &object);
                return;
            }
            
            // If not found in instance, try static subscripts
            auto typeSubscriptManager = staticSubscriptManager->getSubscriptManager(structValue.structName);
            subscriptValue = typeSubscriptManager->findSubscript(arguments);
            if (subscriptValue) {
                subscriptValue->set(*this, arguments, value, &object);
                return;
            }
            
            throw std::runtime_error("No matching subscript found for struct assignment");
        } else {
            throw std::runtime_error("Object is not indexable");
        }
    } else if (auto subscriptAccess = dynamic_cast<const SubscriptAccess*>(expr.target.get())) {
        // Subscript assignment: object[arg1, arg2, ...] = value
        
        // Evaluate all arguments
        std::vector<Value> arguments;
        for (const auto& arg : subscriptAccess->indices) {
            arguments.push_back(evaluate(*arg));
        }
        
        // For simple variable access, modify the original variable directly
        if (auto varExpr = dynamic_cast<const VarExpr*>(subscriptAccess->object.get())) {
            // Get reference to the original variable in environment
            Token varName = varExpr->name;
            Value& originalObject = environment->getReference(varName);
            
            // Handle subscript assignment based on object type
            if (originalObject.type == ValueType::Class) {
                auto classInstance = originalObject.asClass();
                
                // Try to find matching subscript in the instance's subscript manager
                auto subscriptValue = classInstance->subscripts->findSubscript(arguments);
                if (subscriptValue) {
                    subscriptValue->set(*this, arguments, value, &originalObject);
                    return;
                }
                
                // If not found in instance, try static subscripts
                auto typeSubscriptManager = staticSubscriptManager->getSubscriptManager(classInstance->className);
                subscriptValue = typeSubscriptManager->findSubscript(arguments);
                if (subscriptValue) {
                    subscriptValue->set(*this, arguments, value, &originalObject);
                    return;
                }
                
                throw std::runtime_error("No matching subscript found for class instance assignment");
            } else if (originalObject.type == ValueType::Struct) {
                auto& structValue = originalObject.asStruct();
                
                // Try to find matching subscript in the struct's subscript manager
                auto subscriptValue = structValue.subscripts->findSubscript(arguments);
                if (subscriptValue) {
                    subscriptValue->set(*this, arguments, value, &originalObject);
                    return;
                }
                
                // If not found in instance, try static subscripts
                auto typeSubscriptManager = staticSubscriptManager->getSubscriptManager(structValue.structName);
                subscriptValue = typeSubscriptManager->findSubscript(arguments);
                if (subscriptValue) {
                    subscriptValue->set(*this, arguments, value, &originalObject);
                    return;
                }
                
                throw std::runtime_error("No matching subscript found for struct assignment");
            } else {
                throw std::runtime_error("Object is not subscriptable for assignment");
            }
        } else {
            // For complex expressions, fall back to the old behavior
            Value object = evaluate(*subscriptAccess->object);
            
            // Handle subscript assignment based on object type
            if (object.type == ValueType::Class) {
                // Get the class instance
                auto classInstance = object.asClass();
                
                // Try to find matching subscript in the instance's subscript manager
                auto subscriptValue = classInstance->subscripts->findSubscript(arguments);
                if (subscriptValue) {
                    subscriptValue->set(*this, arguments, value, &object);
                    return;
                }
                
                // If not found in instance, try static subscripts
                auto typeSubscriptManager = staticSubscriptManager->getSubscriptManager(classInstance->className);
                subscriptValue = typeSubscriptManager->findSubscript(arguments);
                if (subscriptValue) {
                    subscriptValue->set(*this, arguments, value, &object);
                    return;
                }
                
                throw std::runtime_error("No matching subscript found for class instance assignment");
            } else if (object.type == ValueType::Struct) {
                // Get the struct value
                auto& structValue = object.asStruct();
                
                // Try to find matching subscript in the struct's subscript manager
                auto subscriptValue = structValue.subscripts->findSubscript(arguments);
                if (subscriptValue) {
                    subscriptValue->set(*this, arguments, value, &object);
                    return;
                }
                
                // If not found in instance, try static subscripts
                auto typeSubscriptManager = staticSubscriptManager->getSubscriptManager(structValue.structName);
                subscriptValue = typeSubscriptManager->findSubscript(arguments);
                if (subscriptValue) {
                    subscriptValue->set(*this, arguments, value, &object);
                    return;
                }
                
                throw std::runtime_error("No matching subscript found for struct assignment");
            } else {
                throw std::runtime_error("Object is not subscriptable for assignment");
            }
        }
    } else {
        throw std::runtime_error("Invalid assignment target");
    }
    
    result = value;
}

void Interpreter::visit(const Binary& expr) {
    // Handle logical operators with short-circuit evaluation
    if (expr.op.type == TokenType::AmpAmp) {
        Value left = evaluate(*expr.left);
        if (!isTruthy(left)) {
            result = Value(false);
            return;
        }
        Value right = evaluate(*expr.right);
        result = Value(isTruthy(right));
        return;
    }
    
    if (expr.op.type == TokenType::PipePipe) {
        Value left = evaluate(*expr.left);
        if (isTruthy(left)) {
            result = Value(true);
            return;
        }
        Value right = evaluate(*expr.right);
        result = Value(isTruthy(right));
        return;
    }
    
    Value left = evaluate(*expr.left);
    Value right = evaluate(*expr.right);

    // Helper function to check if a value is numeric
    auto isNumeric = [](const Value& val) {
        return val.type == ValueType::Int || val.type == ValueType::Double || val.type == ValueType::Float ||
               val.type == ValueType::Int8 || val.type == ValueType::Int16 || val.type == ValueType::Int32 || val.type == ValueType::Int64 ||
               val.type == ValueType::UInt || val.type == ValueType::UInt8 || val.type == ValueType::UInt16 || val.type == ValueType::UInt64;
    };
    
    // Helper function to convert value to double for arithmetic
    auto toDouble = [](const Value& val) -> double {
        switch (val.type) {
            case ValueType::Int: return static_cast<double>(std::get<int>(val.value));
            case ValueType::Double: return std::get<double>(val.value);
            case ValueType::Float: return static_cast<double>(std::get<float>(val.value));
            case ValueType::Int8: return static_cast<double>(std::get<int8_t>(val.value));
            case ValueType::Int16: return static_cast<double>(std::get<int16_t>(val.value));
            case ValueType::Int32: return static_cast<double>(std::get<int>(val.value));
            case ValueType::Int64: return static_cast<double>(std::get<int64_t>(val.value));
            case ValueType::UInt: return static_cast<double>(std::get<uint32_t>(val.value));
            case ValueType::UInt8: return static_cast<double>(std::get<uint8_t>(val.value));
            case ValueType::UInt16: return static_cast<double>(std::get<uint16_t>(val.value));
            case ValueType::UInt64: return static_cast<double>(std::get<uint64_t>(val.value));
            default: return 0.0;
        }
    };
    
    // Type checking for non-logical operators
    if (expr.op.type != TokenType::BangEqual && expr.op.type != TokenType::EqualEqual && expr.op.type != TokenType::QuestionQuestion) {
        if (isNumeric(left) && isNumeric(right)) {
            // Numeric operations
        } else if (left.type == ValueType::String && right.type == ValueType::String && expr.op.type == TokenType::Plus) {
            // String concatenation
        } else {
            throw std::runtime_error("Operands must be two numbers or two strings.");
        }
    }

    switch (expr.op.type) {
        case TokenType::Minus: {
            double leftVal = toDouble(left);
            double rightVal = toDouble(right);
            if (left.type == ValueType::Int && right.type == ValueType::Int) {
                result = Value(static_cast<int>(leftVal - rightVal));
            } else {
                result = Value(leftVal - rightVal);
            }
            return;
        }
        case TokenType::Plus:
            if (left.type == ValueType::String) {
                result = Value(std::get<std::string>(left.value) + std::get<std::string>(right.value));
            } else {
                double leftVal = toDouble(left);
                double rightVal = toDouble(right);
                if (left.type == ValueType::Int && right.type == ValueType::Int) {
                    result = Value(static_cast<int>(leftVal + rightVal));
                } else {
                    result = Value(leftVal + rightVal);
                }
            }
            return;
        case TokenType::Slash: {
            double leftVal = toDouble(left);
            double rightVal = toDouble(right);
            result = Value(leftVal / rightVal);
            return;
        }
        case TokenType::Star: {
            double leftVal = toDouble(left);
            double rightVal = toDouble(right);
            if (left.type == ValueType::Int && right.type == ValueType::Int) {
                result = Value(static_cast<int>(leftVal * rightVal));
            } else {
                result = Value(leftVal * rightVal);
            }
            return;
        }
        case TokenType::RAngle: {
            double leftVal = toDouble(left);
            double rightVal = toDouble(right);
            result = Value(leftVal > rightVal);
            return;
        }
        case TokenType::GreaterEqual: {
            double leftVal = toDouble(left);
            double rightVal = toDouble(right);
            result = Value(leftVal >= rightVal);
            return;
        }
        case TokenType::LAngle: {
            double leftVal = toDouble(left);
            double rightVal = toDouble(right);
            result = Value(leftVal < rightVal);
            return;
        }
        case TokenType::LessEqual: {
            double leftVal = toDouble(left);
            double rightVal = toDouble(right);
            result = Value(leftVal <= rightVal);
            return;
        }
        case TokenType::BangEqual:
            result = Value(left.value != right.value);
            return;
        case TokenType::EqualEqual:
            result = Value(left.value == right.value);
            return;
        case TokenType::QuestionQuestion:
            // Nil-coalescing operator: return left if not nil, otherwise return right
            if (left.type == ValueType::Nil) {
                result = right;
            } else {
                result = left;
            }
            return;
        default: break; // Should not be reached
    }
}

void Interpreter::visit(const Ternary& expr) {
    Value condition = evaluate(*expr.condition);
    
    if (isTruthy(condition)) {
        result = evaluate(*expr.thenBranch);
    } else {
        result = evaluate(*expr.elseBranch);
    }
}

void Interpreter::visit(const BitwiseExpr& expr) {
    Value left = evaluate(*expr.left);
    Value right = evaluate(*expr.right);
    
    // Helper function to convert value to integer for bitwise operations
    auto toInt = [](const Value& val) -> int {
        switch (val.type) {
            case ValueType::Int: return std::get<int>(val.value);
            case ValueType::Int8: return static_cast<int>(std::get<int8_t>(val.value));
            case ValueType::Int16: return static_cast<int>(std::get<int16_t>(val.value));
            case ValueType::Int32: return std::get<int>(val.value);
            case ValueType::Int64: return static_cast<int>(std::get<int64_t>(val.value));
            case ValueType::UInt: return static_cast<int>(std::get<uint32_t>(val.value));
            case ValueType::UInt8: return static_cast<int>(std::get<uint8_t>(val.value));
            case ValueType::UInt16: return static_cast<int>(std::get<uint16_t>(val.value));
            case ValueType::UInt64: return static_cast<int>(std::get<uint64_t>(val.value));
            default: return 0;
        }
    };
    
    int leftVal = toInt(left);
    int rightVal = toInt(right);
    
    switch (expr.op.type) {
        case TokenType::BitwiseAnd:
            result = Value(leftVal & rightVal);
            break;
        case TokenType::BitwiseOr:
            result = Value(leftVal | rightVal);
            break;
        case TokenType::BitwiseXor:
            result = Value(leftVal ^ rightVal);
            break;
        case TokenType::LeftShift:
            result = Value(leftVal << rightVal);
            break;
        case TokenType::RightShift:
            result = Value(leftVal >> rightVal);
            break;
        default:
            throw std::runtime_error("Unknown bitwise operator.");
    }
}

void Interpreter::visit(const OverflowExpr& expr) {
    Value left = evaluate(*expr.left);
    Value right = evaluate(*expr.right);
    
    // Helper function to convert value to integer
    auto toInt = [](const Value& val) -> int {
        switch (val.type) {
            case ValueType::Int: return std::get<int>(val.value);
            case ValueType::Int8: return static_cast<int>(std::get<int8_t>(val.value));
            case ValueType::Int16: return static_cast<int>(std::get<int16_t>(val.value));
            case ValueType::Int32: return std::get<int>(val.value);
            case ValueType::Int64: return static_cast<int>(std::get<int64_t>(val.value));
            case ValueType::UInt: return static_cast<int>(std::get<uint32_t>(val.value));
            case ValueType::UInt8: return static_cast<int>(std::get<uint8_t>(val.value));
            case ValueType::UInt16: return static_cast<int>(std::get<uint16_t>(val.value));
            case ValueType::UInt64: return static_cast<int>(std::get<uint64_t>(val.value));
            default: return 0;
        }
    };
    
    int leftVal = toInt(left);
    int rightVal = toInt(right);
    
    // Overflow arithmetic operations (wrapping behavior)
    switch (expr.op.type) {
        case TokenType::OverflowPlus: {
            // Use unsigned arithmetic to get wrapping behavior
            unsigned int result_val = static_cast<unsigned int>(leftVal) + static_cast<unsigned int>(rightVal);
            result = Value(static_cast<int>(result_val));
            break;
        }
        case TokenType::OverflowMinus: {
            unsigned int result_val = static_cast<unsigned int>(leftVal) - static_cast<unsigned int>(rightVal);
            result = Value(static_cast<int>(result_val));
            break;
        }
        case TokenType::OverflowStar: {
            unsigned int result_val = static_cast<unsigned int>(leftVal) * static_cast<unsigned int>(rightVal);
            result = Value(static_cast<int>(result_val));
            break;
        }
        default:
            throw std::runtime_error("Unknown overflow operator.");
    }
}

void Interpreter::visit(const CustomOperatorExpr& expr) {
    // Look up the custom operator function in the environment
    try {
        Value operatorFunction = environment->get(expr.op);
        
        if (!operatorFunction.isFunction()) {
            throw std::runtime_error("Custom operator '" + expr.op.lexeme + "' is not a function.");
        }
        
        // Evaluate operands
        std::vector<Value> arguments;
        
        if (expr.left) {
            // Binary operator: left op right
            Value left = evaluate(*expr.left);
            Value right = evaluate(*expr.right);
            arguments = {left, right};
        } else {
            // Prefix operator: op right
            Value right = evaluate(*expr.right);
            arguments = {right};
        }
        
        auto callable = operatorFunction.asFunction();
        
        if (callable->isFunction) {
            // Check argument count
            if (arguments.size() != callable->functionDecl->parameters.size()) {
                throw std::runtime_error("Expected " + 
                    std::to_string(callable->functionDecl->parameters.size()) + 
                    " arguments but got " + std::to_string(arguments.size()) + ".");
            }
            
            // Create new environment for function execution
            auto previous = environment;
            environment = std::make_shared<Environment>(callable->closure);
            
            // Create new defer stack level for this function
            deferStack.push(std::vector<std::unique_ptr<Stmt>>());
            
            // Bind parameters by position (operator functions don't use external labels)
            for (size_t i = 0; i < callable->functionDecl->parameters.size(); ++i) {
                environment->define(
                    callable->functionDecl->parameters[i].name.lexeme,
                    arguments[i],
                    false, // parameters are not const
                    callable->functionDecl->parameters[i].type.lexeme
                );
            }
            
            try {
                // Execute function body
                callable->functionDecl->body->accept(*this);
                
                // Execute deferred statements before function exits
                executeDeferredStatements();
                
                // If no return statement was executed, return nil
                result = Value();
            } catch (const ReturnException& returnValue) {
                // Execute deferred statements before function exits
                executeDeferredStatements();
                
                // Function returned a value
                result = *returnValue.value;
            }
            
            // Restore previous environment
            environment = previous;
        } else {
            throw std::runtime_error("Custom operator '" + expr.op.lexeme + "' is not a function.");
        }
    } catch (const std::runtime_error&) {
        throw std::runtime_error("Custom operator '" + expr.op.lexeme + "' not implemented.");
    }
}

void Interpreter::visit(const ResultBuilderExpr& expr) {
    // Result builder expressions would be transformed at compile time
    // For now, just evaluate the components and return the first one
    if (!expr.components.empty()) {
        result = evaluate(*expr.components[0]);
    } else {
        result = Value(); // nil
    }
}

void Interpreter::visit(const Grouping& expr) {
    result = evaluate(*expr.expression);
}

void Interpreter::visit(const Literal& expr) {
    switch (expr.value.type) {
        case TokenType::True:
            result = Value(true);
            break;
        case TokenType::False:
            result = Value(false);
            break;
        case TokenType::Nil:
            result = Value();
            break;
        case TokenType::IntegerLiteral:
            result = Value(std::stoi(expr.value.lexeme));
            break;
        case TokenType::FloatingLiteral:
            result = Value(std::stod(expr.value.lexeme));
            break;
        case TokenType::StringLiteral:
            result = Value(expr.value.lexeme);
            break;
        case TokenType::CharacterLiteral:
            // Character literal should contain exactly one character
            if (!expr.value.lexeme.empty()) {
                // Convert to unsigned char to avoid signed char issues
                unsigned char ch = static_cast<unsigned char>(expr.value.lexeme[0]);
                result = Value(static_cast<char>(ch));
            } else {
                throw std::runtime_error("Invalid character literal.");
            }
            break;
        case TokenType::InterpolatedStringLiteral:
            // For now, treat interpolated string literals as regular strings
            // TODO: Implement proper string interpolation evaluation
            result = Value(expr.value.lexeme);
            break;
        default:
            break; // Should not be reached
    }
}

void Interpreter::visit(const StringInterpolation& expr) {
    std::string result_str;
    
    for (const auto& part : expr.parts) {
        if (part.expression) {
            // This is an interpolated expression
            Value value = evaluate(*part.expression);
            
            // Convert value to string
            switch (value.type) {
                case ValueType::Int:
                    result_str += std::to_string(std::get<int>(value.value));
                    break;
                case ValueType::Double:
                    result_str += std::to_string(std::get<double>(value.value));
                    break;
                case ValueType::Bool:
                    result_str += std::get<bool>(value.value) ? "true" : "false";
                    break;
                case ValueType::String:
                    result_str += std::get<std::string>(value.value);
                    break;
                case ValueType::Nil:
                    result_str += "nil";
                    break;
                
                // Extended Integer Types
                case ValueType::Int8:
                    result_str += std::to_string(static_cast<int>(std::get<int8_t>(value.value)));
                    break;
                case ValueType::Int16:
                    result_str += std::to_string(std::get<int16_t>(value.value));
                    break;
                case ValueType::Int32:
                    result_str += std::to_string(std::get<int>(value.value));
                    break;
                case ValueType::Int64:
                    result_str += std::to_string(std::get<int64_t>(value.value));
                    break;
                case ValueType::UInt:
                    result_str += std::to_string(std::get<uint32_t>(value.value));
                    break;
                case ValueType::UInt8:
                    result_str += std::to_string(static_cast<unsigned int>(std::get<uint8_t>(value.value)));
                    break;
                case ValueType::UInt16:
                    result_str += std::to_string(std::get<uint16_t>(value.value));
                    break;
                case ValueType::UInt64:
                    result_str += std::to_string(std::get<uint64_t>(value.value));
                    break;
                
                // Additional Basic Types
                case ValueType::Float:
                    result_str += std::to_string(std::get<float>(value.value));
                    break;
                case ValueType::Character:
                    result_str += static_cast<unsigned char>(std::get<char>(value.value));
                    break;
                
                // Special Types
                case ValueType::Set:
                    result_str += "<set>";
                    break;
                case ValueType::Any:
                    result_str += "<any>";
                    break;
                case ValueType::Void:
                    result_str += "()";
                    break;
                
                default:
                    result_str += valueToString(value);
                    break;
            }
        } else {
            // This is a string literal part
            result_str += part.text;
        }
    }
    
    result = Value(result_str);
}

void Interpreter::visit(const Unary& expr) {
    Value right = evaluate(*expr.right);

    switch (expr.op.type) {
        case TokenType::Minus: {
            if (right.type == ValueType::Int) {
                result = Value(-std::get<int>(right.value));
            } else if (right.type == ValueType::Double) {
                result = Value(-std::get<double>(right.value));
            } else if (right.type == ValueType::Float) {
                result = Value(-std::get<float>(right.value));
            } else if (right.type == ValueType::Int8) {
                result = Value(static_cast<int8_t>(-std::get<int8_t>(right.value)));
            } else if (right.type == ValueType::Int16) {
                result = Value(static_cast<int16_t>(-std::get<int16_t>(right.value)));
            } else if (right.type == ValueType::Int32) {
                result = Value(-std::get<int>(right.value));
            } else if (right.type == ValueType::Int64) {
                result = Value(-std::get<int64_t>(right.value));
            } else {
                throw std::runtime_error("Operand must be a number.");
            }
            return;
        }
        case TokenType::Bang:
            result = Value(!isTruthy(right));
            return;
        default: break; // Should not be reached
    }
}

Value Interpreter::evaluate(const Expr& expr) {
    expr.accept(*this);
    return result;
}

bool Interpreter::isTruthy(const Value& value) {
    if (value.type == ValueType::Nil) return false;
    if (value.type == ValueType::Bool) return std::get<bool>(value.value);
    return true;
}

void Interpreter::visit(const ArrayLiteral& expr) {
    std::vector<Value> elements;
    for (const auto& element : expr.elements) {
        elements.push_back(evaluate(*element));
    }
    result = Value(elements);
}

void Interpreter::visit(const DictionaryLiteral& expr) {
    std::unordered_map<std::string, Value> dict;
    for (const auto& pair : expr.pairs) {
        Value key = evaluate(*pair.key);
        Value value = evaluate(*pair.value);
        
        // Convert key to string for dictionary storage
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
            case ValueType::Float:
                keyStr = std::to_string(std::get<float>(key.value));
                break;
            case ValueType::Int8:
                keyStr = std::to_string(static_cast<int>(std::get<int8_t>(key.value)));
                break;
            case ValueType::Int16:
                keyStr = std::to_string(std::get<int16_t>(key.value));
                break;
            case ValueType::Int32:
                keyStr = std::to_string(std::get<int>(key.value));
                break;
            case ValueType::Int64:
                keyStr = std::to_string(std::get<int64_t>(key.value));
                break;
            case ValueType::UInt:
                keyStr = std::to_string(std::get<uint32_t>(key.value));
                break;
            case ValueType::UInt8:
                keyStr = std::to_string(static_cast<unsigned int>(std::get<uint8_t>(key.value)));
                break;
            case ValueType::UInt16:
                keyStr = std::to_string(std::get<uint16_t>(key.value));
                break;
            case ValueType::UInt64:
                keyStr = std::to_string(std::get<uint64_t>(key.value));
                break;
            case ValueType::Character:
                keyStr = std::string(1, static_cast<unsigned char>(std::get<char>(key.value)));
                break;
            default:
                throw std::runtime_error("Dictionary keys must be strings or numbers.");
        }
        
        dict[keyStr] = value;
    }
    result = Value(dict);
}

void Interpreter::visit(const TupleLiteral& expr) {
    std::vector<Value> elements;
    for (const auto& element : expr.elements) {
        elements.push_back(evaluate(*element));
    }
    result = Value(TupleValue(std::move(elements)));
}

void Interpreter::visit(const IndexAccess& expr) {
    Value object = evaluate(*expr.object);
    Value index = evaluate(*expr.index);
    
    if (object.type == ValueType::Array) {
        // Helper function to check if index is an integer type
        auto isIntegerType = [](const Value& val) {
            return val.type == ValueType::Int || val.type == ValueType::Int8 || val.type == ValueType::Int16 ||
                   val.type == ValueType::Int32 || val.type == ValueType::Int64 || val.type == ValueType::UInt ||
                   val.type == ValueType::UInt8 || val.type == ValueType::UInt16 || val.type == ValueType::UInt64;
        };
        
        // Helper function to convert index to int
        auto toInt = [](const Value& val) -> int {
            switch (val.type) {
                case ValueType::Int: return std::get<int>(val.value);
                case ValueType::Int8: return static_cast<int>(std::get<int8_t>(val.value));
                case ValueType::Int16: return static_cast<int>(std::get<int16_t>(val.value));
                case ValueType::Int32: return static_cast<int>(std::get<int>(val.value));
                case ValueType::Int64: return static_cast<int>(std::get<int64_t>(val.value));
                case ValueType::UInt: return static_cast<int>(std::get<uint32_t>(val.value));
                case ValueType::UInt8: return static_cast<int>(std::get<uint8_t>(val.value));
                case ValueType::UInt16: return static_cast<int>(std::get<uint16_t>(val.value));
                case ValueType::UInt64: return static_cast<int>(std::get<uint64_t>(val.value));
                default: return 0;
            }
        };
        
        if (!isIntegerType(index)) {
            throw std::runtime_error("Array index must be an integer.");
        }
        
        int idx = toInt(index);
        const auto& arr = *object.asArray();
        
        if (idx < 0 || idx >= static_cast<int>(arr.size())) {
            throw std::runtime_error("Array index out of bounds.");
        }
        
        result = arr[idx];
    } else if (object.type == ValueType::Dictionary) {
        std::string key;
        switch (index.type) {
            case ValueType::String:
                key = std::get<std::string>(index.value);
                break;
            case ValueType::Int:
                key = std::to_string(std::get<int>(index.value));
                break;
            case ValueType::Double:
                key = std::to_string(std::get<double>(index.value));
                break;
            case ValueType::Float:
                key = std::to_string(std::get<float>(index.value));
                break;
            case ValueType::Int8:
                key = std::to_string(static_cast<int>(std::get<int8_t>(index.value)));
                break;
            case ValueType::Int16:
                key = std::to_string(std::get<int16_t>(index.value));
                break;
            case ValueType::Int32:
                key = std::to_string(std::get<int>(index.value));
                break;
            case ValueType::Int64:
                key = std::to_string(std::get<int64_t>(index.value));
                break;
            case ValueType::UInt:
                key = std::to_string(std::get<uint32_t>(index.value));
                break;
            case ValueType::UInt8:
                key = std::to_string(static_cast<unsigned int>(std::get<uint8_t>(index.value)));
                break;
            case ValueType::UInt16:
                key = std::to_string(std::get<uint16_t>(index.value));
                break;
            case ValueType::UInt64:
                key = std::to_string(std::get<uint64_t>(index.value));
                break;
            case ValueType::Character:
                key = std::string(1, static_cast<unsigned char>(std::get<char>(index.value)));
                break;
            default:
                throw std::runtime_error("Dictionary key must be a string or number.");
        }
        
        const auto& dict = *object.asDictionary();
        auto it = dict.find(key);
        if (it != dict.end()) {
            result = it->second;
        } else {
            result = Value(); // nil for missing keys
        }
    } else if (object.type == ValueType::Class) {
        // Handle subscript access on class instances
        auto classInstance = object.asClass();
        
        // Convert single index to vector for subscript system
        std::vector<Value> arguments = {index};
        
        // Try to find matching subscript in the instance's subscript manager
        auto subscriptValue = classInstance->subscripts->findSubscript(arguments);
        if (subscriptValue) {
            result = subscriptValue->get(*this, arguments, &object);
            return;
        }
        
        // If not found in instance, try static subscripts
        auto typeSubscriptManager = staticSubscriptManager->getSubscriptManager(classInstance->className);
        subscriptValue = typeSubscriptManager->findSubscript(arguments);
        if (subscriptValue) {
            result = subscriptValue->get(*this, arguments, &object);
            return;
        }
        
        throw std::runtime_error("No matching subscript found for class instance");
    } else if (object.type == ValueType::Struct) {
        // Handle subscript access on struct instances
        auto& structValue = object.asStruct();
        
        // Convert single index to vector for subscript system
        std::vector<Value> arguments = {index};
        
        // Try to find matching subscript in the struct's subscript manager
        auto subscriptValue = structValue.subscripts->findSubscript(arguments);
        if (subscriptValue) {
            result = subscriptValue->get(*this, arguments, &object);
            return;
        }
        
        // If not found in instance, try static subscripts
        auto typeSubscriptManager = staticSubscriptManager->getSubscriptManager(structValue.structName);
        subscriptValue = typeSubscriptManager->findSubscript(arguments);
        if (subscriptValue) {
            result = subscriptValue->get(*this, arguments, &object);
            return;
        }
        
        throw std::runtime_error("No matching subscript found for struct");
    } else {
        throw std::runtime_error("Object is not subscriptable.");
    }
}

void Interpreter::visit(const SubscriptAccess& expr) {
    Value object = evaluate(*expr.object);
    
    std::cout << "SubscriptAccess: object type = " << static_cast<int>(object.type) << std::endl;
    std::cout << "ValueType::Struct = " << static_cast<int>(ValueType::Struct) << std::endl;
    std::cout << "ValueType::Class = " << static_cast<int>(ValueType::Class) << std::endl;
    
    // Evaluate all arguments
    std::vector<Value> arguments;
    for (const auto& arg : expr.indices) {
        arguments.push_back(evaluate(*arg));
    }
    
    // Handle subscript access based on object type
    if (object.type == ValueType::Class) {
        // Get the class instance
        auto classInstance = object.asClass();
        
        // Try to find matching subscript in the instance's subscript manager
        auto subscriptValue = classInstance->subscripts->findSubscript(arguments);
        if (subscriptValue) {
            result = subscriptValue->get(*this, arguments, &object);
            return;
        }
        
        // If not found in instance, try static subscripts
        auto typeSubscriptManager = staticSubscriptManager->getSubscriptManager(classInstance->className);
        subscriptValue = typeSubscriptManager->findSubscript(arguments);
        if (subscriptValue) {
            result = subscriptValue->get(*this, arguments, &object);
            return;
        }
        
        throw std::runtime_error("No matching subscript found for class instance");
    } else if (object.type == ValueType::Struct) {
         // Get the struct value
         auto& structValue = object.asStruct();
         
         // Try to find matching subscript in the struct's subscript manager
         auto subscriptValue = structValue.subscripts->findSubscript(arguments);
         if (subscriptValue) {
             result = subscriptValue->get(*this, arguments, &object);
             return;
         }
         
         // If not found in instance, try static subscripts
         auto typeSubscriptManager = staticSubscriptManager->getSubscriptManager(structValue.structName);
         subscriptValue = typeSubscriptManager->findSubscript(arguments);
         if (subscriptValue) {
             result = subscriptValue->get(*this, arguments, &object);
             return;
         }
        
        throw std::runtime_error("No matching subscript found for struct");
    } else {
        throw std::runtime_error("Object is not subscriptable");
    }
}

void Interpreter::printArray(const Array& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr->size(); ++i) {
        if (i > 0) std::cout << ", ";
        printValue((*arr)[i]);
    }
    std::cout << "]" << std::endl;
}

void Interpreter::printDictionary(const Dictionary& dict) {
    std::cout << "{";
    bool first = true;
    for (const auto& pair : *dict) {
        if (!first) std::cout << ", ";
        std::cout << "\"" << pair.first << "\": ";
        printValue(pair.second);
        first = false;
    }
    std::cout << "}" << std::endl;
}

void Interpreter::printArrayInline(const Array& arr) {
    std::cout << "[";
    for (size_t i = 0; i < arr->size(); ++i) {
        if (i > 0) std::cout << ", ";
        printValueInline((*arr)[i]);
    }
    std::cout << "]";
}

void Interpreter::printDictionaryInline(const Dictionary& dict) {
    std::cout << "{";
    bool first = true;
    for (const auto& pair : *dict) {
        if (!first) std::cout << ", ";
        std::cout << "\"" << pair.first << "\": ";
        printValueInline(pair.second);
        first = false;
    }
    std::cout << "}";
}

void Interpreter::printTuple(const Tuple& tuple) {
    std::cout << "(";
    for (size_t i = 0; i < tuple.elements->size(); ++i) {
        if (i > 0) std::cout << ", ";
        printValue((*tuple.elements)[i]);
    }
    std::cout << ")" << std::endl;
}

void Interpreter::printTupleInline(const Tuple& tuple) {
    std::cout << "(";
    for (size_t i = 0; i < tuple.elements->size(); ++i) {
        if (i > 0) std::cout << ", ";
        printValueInline((*tuple.elements)[i]);
    }
    std::cout << ")";
}

std::string Interpreter::valueToString(const Value& val) {
    switch (val.type) {
        case ValueType::Int:
            return std::to_string(std::get<int>(val.value));
        case ValueType::Double:
            return std::to_string(std::get<double>(val.value));
        case ValueType::Bool:
            return std::get<bool>(val.value) ? "true" : "false";
        case ValueType::String:
            return std::get<std::string>(val.value);
        case ValueType::Nil:
            return "nil";
        case ValueType::Array: {
            const auto& arr = val.asArray();
            std::string result = "[";
            for (size_t i = 0; i < arr->size(); ++i) {
                if (i > 0) result += ", ";
                result += valueToString((*arr)[i]);
            }
            result += "]";
            return result;
        }
        case ValueType::Dictionary:
            return "<dictionary>";
        case ValueType::Tuple:
            return "<tuple>";
        case ValueType::Function:
            return val.isClosure() ? "<closure>" : "<function>";
        case ValueType::Enum: {
            const auto& enumVal = val.asEnum();
            return enumVal->enumName + "." + enumVal->caseName;
        }
        case ValueType::Struct: {
            const auto& structVal = val.asStruct();
            return "<" + structVal.structName + ">";
        }
        case ValueType::Class: {
            const auto& classVal = val.asClass();
            return "<" + classVal->className + ">";
        }
        case ValueType::Constructor:
            return "<constructor>";
        case ValueType::Destructor:
            return "<destructor>";
        case ValueType::Optional: {
            const auto& optionalVal = val.asOptional();
            if (optionalVal.hasValue && optionalVal.wrappedValue) {
                return "Optional(" + valueToString(*optionalVal.wrappedValue) + ")";
            } else {
                return "nil";
            }
        }
        case ValueType::Error:
            return "<error>";
        case ValueType::Result: {
            auto resultPtr = val.asResult();
            if (resultPtr->isSuccess) {
                return "Result.success(" + valueToString(resultPtr->getValue()) + ")";
            } else {
                return "Result.failure(" + resultPtr->getError().getDescription() + ")";
            }
        }
        
        // Extended Integer Types
        case ValueType::Int8:
            return std::to_string(static_cast<int>(std::get<int8_t>(val.value)));
        case ValueType::Int16:
            return std::to_string(std::get<int16_t>(val.value));
        case ValueType::Int32:
            return std::to_string(std::get<int>(val.value));
        case ValueType::Int64:
            return std::to_string(std::get<int64_t>(val.value));
        case ValueType::UInt:
            return std::to_string(std::get<uint32_t>(val.value));
        case ValueType::UInt8:
            return std::to_string(static_cast<unsigned int>(std::get<uint8_t>(val.value)));
        case ValueType::UInt16:
            return std::to_string(std::get<uint16_t>(val.value));
        case ValueType::UInt64:
            return std::to_string(std::get<uint64_t>(val.value));
        
        // Additional Basic Types
        case ValueType::Float:
            return std::to_string(std::get<float>(val.value));
        case ValueType::Character:
            return std::string(1, static_cast<unsigned char>(std::get<char>(val.value)));
        
        // Special Types
        case ValueType::Set:
            return "<set>";
        case ValueType::Any:
            return "<any>";
        case ValueType::Void:
            return "()";
        
        default:
            return "<unknown>";
    }
}

void Interpreter::printValue(const Value& val) {
    switch (val.type) {
        case ValueType::Int:
            std::cout << std::get<int>(val.value);
            break;
        case ValueType::Double:
            std::cout << std::get<double>(val.value);
            break;
        case ValueType::Bool:
            std::cout << (std::get<bool>(val.value) ? "true" : "false");
            break;
        case ValueType::String:
            std::cout << "\"" << std::get<std::string>(val.value) << "\"";
            break;
        case ValueType::Array:
            printArray(val.asArray());
            break;
        case ValueType::Dictionary:
            printDictionary(val.asDictionary());
            break;
        case ValueType::Tuple:
            printTuple(val.asTuple());
            break;
        case ValueType::Nil:
            std::cout << "nil";
            break;
        case ValueType::Function:
            if (val.isClosure()) {
                std::cout << "<closure>";
            } else {
                std::cout << "<function>";
            }
            break;
        case ValueType::Enum: {
            const auto& enumVal = val.asEnum();
            std::cout << enumVal->enumName << "." << enumVal->caseName;
            if (!enumVal->associatedValues.empty()) {
                std::cout << "(";
                for (size_t i = 0; i < enumVal->associatedValues.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    printValue(enumVal->associatedValues[i]);
                }
                std::cout << ")";
            }
            break;
        }
        case ValueType::Struct: {
            const auto& structVal = val.asStruct();
            std::cout << structVal.structName << "(";
            bool first = true;
            for (const auto& member : *structVal.members) {
                if (!first) std::cout << ", ";
                std::cout << member.first << ": ";
                printValue(member.second);
                first = false;
            }
            std::cout << ")";
            break;
        }
        case ValueType::Class: {
            const auto& classVal = val.asClass();
            std::cout << classVal->className << "(";
            bool first = true;
            for (const auto& member : *classVal->members) {
                if (!first) std::cout << ", ";
                std::cout << member.first << ": ";
                printValue(member.second);
                first = false;
            }
            std::cout << ")";
            break;
        }
        case ValueType::Constructor:
            std::cout << "<constructor>";
            break;
        case ValueType::Destructor:
            std::cout << "<destructor>";
            break;
        case ValueType::Optional: {
            const auto& optionalVal = val.asOptional();
            if (optionalVal.hasValue && optionalVal.wrappedValue) {
                std::cout << "Optional(";
                printValue(*optionalVal.wrappedValue);
                std::cout << ")";
            } else {
                std::cout << "nil";
            }
            break;
        }
        case ValueType::Error:
            std::cout << "<error>";
            break;
        case ValueType::Result:
            std::cout << "<result>";
            break;
        
        // Extended Integer Types
        case ValueType::Int8:
            std::cout << static_cast<int>(std::get<int8_t>(val.value));
            break;
        case ValueType::Int16:
            std::cout << std::get<int16_t>(val.value);
            break;
        case ValueType::Int32:
            std::cout << std::get<int>(val.value);
            break;
        case ValueType::Int64:
            std::cout << std::get<int64_t>(val.value);
            break;
        case ValueType::UInt:
            std::cout << std::get<uint32_t>(val.value);
            break;
        case ValueType::UInt8:
            std::cout << static_cast<unsigned int>(std::get<uint8_t>(val.value));
            break;
        case ValueType::UInt16:
            std::cout << std::get<uint16_t>(val.value);
            break;
        case ValueType::UInt64:
            std::cout << std::get<uint64_t>(val.value);
            break;
        
        // Additional Basic Types
        case ValueType::Float:
            std::cout << std::get<float>(val.value);
            break;
        case ValueType::Character:
            std::cout << "'" << std::get<char>(val.value) << "'";
            break;
        
        // Special Types
        case ValueType::Set:
            std::cout << "<set>";
            break;
        case ValueType::Any:
            std::cout << "<any>";
            break;
        case ValueType::Void:
            std::cout << "()";
            break;
    }
}

// Execute block statement: { statements }
void Interpreter::visit(const BlockStmt& stmt) {
    // Create new environment for block scope
    auto previous = environment;
    environment = std::make_shared<Environment>(environment);
    
    try {
        for (const auto& statement : stmt.statements) {
            statement->accept(*this);
        }
    } catch (...) {
        // Restore previous environment even if exception occurs
        environment = previous;
        throw;
    }
    
    // Restore previous environment
    environment = previous;
}

// Execute if statement: if condition { thenBranch } else { elseBranch }
void Interpreter::visit(const IfStmt& stmt) {
    Value condition = evaluate(*stmt.condition);
    
    if (isTruthy(condition)) {
        stmt.thenBranch->accept(*this);
    } else if (stmt.elseBranch) {
        stmt.elseBranch->accept(*this);
    }
}

// Execute if-let statement: if let variable = expression { thenBranch } else { elseBranch }
void Interpreter::visit(const IfLetStmt& stmt) {
    Value expressionValue = evaluate(*stmt.expression);
    
    // Check if the value is not nil (optional binding succeeds)
    if (expressionValue.type != ValueType::Nil) {
        // Create new environment for the if-let scope
        auto previous = environment;
        environment = std::make_shared<Environment>(environment);
        
        try {
            // Bind the unwrapped value to the variable
            environment->define(stmt.variable.lexeme, expressionValue, false, "Any");
            
            // Execute the then branch
            stmt.thenBranch->accept(*this);
        } catch (...) {
            // Restore previous environment even if exception occurs
            environment = previous;
            throw;
        }
        
        // Restore previous environment
        environment = previous;
    } else if (stmt.elseBranch) {
        // Optional binding failed, execute else branch
        stmt.elseBranch->accept(*this);
    }
}

// Execute while statement: while condition { body }
void Interpreter::visit(const WhileStmt& stmt) {
    while (true) {
        Value condition = evaluate(*stmt.condition);
        if (!isTruthy(condition)) break;
        
        stmt.body->accept(*this);
    }
}

// Execute for statement: for initializer; condition; increment { body }
void Interpreter::visit(const ForStmt& stmt) {
    // Create new environment for for loop scope
    auto previous = environment;
    environment = std::make_shared<Environment>(environment);
    
    try {
        // Execute initializer
        if (stmt.initializer) {
            stmt.initializer->accept(*this);
        }
        
        // Loop
        while (true) {
            // Check condition
            if (stmt.condition) {
                Value condition = evaluate(*stmt.condition);
                if (!isTruthy(condition)) break;
            }
            
            // Execute body
            stmt.body->accept(*this);
            
            // Execute increment
            if (stmt.increment) {
                evaluate(*stmt.increment);
            }
        }
    } catch (...) {
        // Restore previous environment even if exception occurs
        environment = previous;
        throw;
    }
    
    // Restore previous environment
    environment = previous;
}

// Execute for-in statement: for variable in collection { body } or for (var1, var2) in collection { body }
void Interpreter::visit(const ForInStmt& stmt) {
    // Create new environment for for-in loop scope
    auto previous = environment;
    environment = std::make_shared<Environment>(environment);
    
    try {
        // Evaluate the collection
        Value collection = evaluate(*stmt.collection);
        
        if (collection.type == ValueType::Array) {
            const auto& arr = *collection.asArray();
            
            if (stmt.variables.size() == 1) {
                // Simple iteration: for item in array
                for (const auto& item : arr) {
                    environment->define(stmt.variables[0].lexeme, item, false, "Any");
                    stmt.body->accept(*this);
                }
            } else if (stmt.variables.size() == 2) {
                // Tuple destructuring: for (index, value) in array.enumerated()
                // For now, assume this is enumerated() call and provide index and value
                for (size_t i = 0; i < arr.size(); ++i) {
                    environment->define(stmt.variables[0].lexeme, Value(static_cast<int>(i)), false, "Int");
                    environment->define(stmt.variables[1].lexeme, arr[i], false, "Any");
                    stmt.body->accept(*this);
                }
            } else {
                throw std::runtime_error("Invalid number of variables for array iteration");
            }
        } else if (collection.type == ValueType::Dictionary) {
            // First check if this is a range expression by looking for range keys
            const auto& dict = collection.asDictionaryRef();
            auto startIt = dict.find("start");
            auto endIt = dict.find("end");
            auto typeIt = dict.find("type");
            
            if (startIt != dict.end() && endIt != dict.end() && typeIt != dict.end()) {
                // This is a range expression: for i in 1...4 or for i in 1..<4
                if (stmt.variables.size() == 1) {
                    Value startVal = startIt->second;
                    Value endVal = endIt->second;
                    Value typeVal = typeIt->second;
                    
                    if (startVal.type == ValueType::Int && endVal.type == ValueType::Int) {
                        int start = startVal.asInt32();
                        int end = endVal.asInt32();
                        bool isClosed = (typeVal.type == ValueType::String && std::get<std::string>(typeVal.value) == "closed");
                        
                        // Iterate through the range
                        int limit = isClosed ? end : end - 1;
                        for (int i = start; i <= limit; ++i) {
                            environment->define(stmt.variables[0].lexeme, Value(i), false, "Int");
                            stmt.body->accept(*this);
                        }
                    } else {
                        throw std::runtime_error("Range bounds must be integers");
                    }
                } else {
                    throw std::runtime_error("Range iteration requires exactly 1 variable");
                }
            } else {
                // This is a regular dictionary
                const auto& dictRef = *collection.asDictionary();
                
                if (stmt.variables.size() == 2) {
                    // Dictionary iteration: for (key, value) in dictionary
                    for (const auto& pair : dictRef) {
                        environment->define(stmt.variables[0].lexeme, Value(pair.first), false, "String");
                        environment->define(stmt.variables[1].lexeme, pair.second, false, "Any");
                        stmt.body->accept(*this);
                    }
                } else {
                    throw std::runtime_error("Dictionary iteration requires exactly 2 variables");
                }
            }
        } else {
            throw std::runtime_error("Can only iterate over arrays, dictionaries, and ranges");
        }
    } catch (...) {
        // Restore previous environment even if exception occurs
        environment = previous;
        throw;
    }
    
    // Restore previous environment
    environment = previous;
}

// Execute for-await statement: for await variable in asyncSequence { body }
void Interpreter::visit(const ForAwaitStmt& stmt) {
    // Create new environment for for-await loop scope
    auto previous = environment;
    environment = std::make_shared<Environment>(environment);
    
    try {
        // Evaluate the async sequence
        Value asyncSequence = evaluate(*stmt.asyncSequence);
        
        // For now, treat async sequences like regular sequences
        // In a full implementation, this would handle async iteration
        if (asyncSequence.type == ValueType::Array) {
            const auto& arr = *asyncSequence.asArray();
            
            if (stmt.variables.size() == 1) {
                // Simple async iteration: for await item in asyncArray
                for (const auto& item : arr) {
                    environment->define(stmt.variables[0].lexeme, item, false, "Any");
                    stmt.body->accept(*this);
                }
            } else {
                throw std::runtime_error("Invalid number of variables for async sequence iteration");
            }
        } else {
            throw std::runtime_error("Can only iterate over async sequences (arrays for now)");
        }
    } catch (...) {
        // Restore previous environment even if exception occurs
        environment = previous;
        throw;
    }
    
    // Restore previous environment
    environment = previous;
}

// Execute function declaration: func name(parameters) -> ReturnType { body }
void Interpreter::visit(const FunctionStmt& stmt) {
    auto function = std::make_shared<Function>(&stmt, environment);
    environment->define(stmt.name.lexeme, Value(function), false, "Function");
}

// Execute return statement: return expression?
void Interpreter::visit(const ReturnStmt& stmt) {
    Value value;
    if (stmt.value) {
        value = evaluate(*stmt.value);
    }
    throw ReturnException(value);
}

void Interpreter::visit(const ContinueStmt& stmt) {
    std::string label = stmt.label.lexeme.empty() ? "" : stmt.label.lexeme;
    throw ContinueException(label);
}

void Interpreter::visit(const BreakStmt& stmt) {
    std::string label = stmt.label.lexeme.empty() ? "" : stmt.label.lexeme;
    throw BreakException(label);
}

void Interpreter::visit(const FallthroughStmt& stmt) {
    throw FallthroughException();
}

// Execute function call: callee(arguments)
void Interpreter::visit(const Call& expr) {
    // std::cout << "DEBUG: Call expression detected" << std::endl;
    
    // Debug: print callee type
    if (auto memberAccess = dynamic_cast<const MemberAccess*>(expr.callee.get())) {
        std::cout << "DEBUG: Callee is MemberAccess: " << memberAccess->member.lexeme << std::endl;
        if (auto superExpr = dynamic_cast<const Super*>(memberAccess->object.get())) {
            std::cout << "DEBUG: MemberAccess object is Super" << std::endl;
        }
    } else if (auto superExpr = dynamic_cast<const Super*>(expr.callee.get())) {
        std::cout << "DEBUG: Callee is Super directly" << std::endl;
    } else {
        std::cout << "DEBUG: Callee is neither MemberAccess nor Super" << std::endl;
    }
    
    // Check if this is a super method call BEFORE evaluating the callee
    if (auto memberAccess = dynamic_cast<const MemberAccess*>(expr.callee.get())) {
        std::cout << "DEBUG: Checking if MemberAccess object is Super..." << std::endl;
        if (auto superExpr = dynamic_cast<const Super*>(memberAccess->object.get())) {
            std::cout << "DEBUG: This is a super method call via MemberAccess: " << memberAccess->member.lexeme << std::endl;
            
            // Get current method class context from environment
            std::string currentMethodClass;
            try {
                Value methodClassContext = environment->get(Token(TokenType::Identifier, "__current_method_class__", 0));
                if (methodClassContext.type == ValueType::String) {
                    currentMethodClass = std::get<std::string>(methodClassContext.value);
                    // std::cout << "DEBUG: Found __current_method_class__: " << currentMethodClass << std::endl;
                } else {
                    // std::cout << "DEBUG: __current_method_class__ is not a string" << std::endl;
                    throw std::runtime_error("super can only be used within class methods");
                }
            } catch (const std::runtime_error& e) {
                // std::cout << "DEBUG: Failed to get __current_method_class__: " << e.what() << std::endl;
                throw std::runtime_error("super can only be used within class methods");
            }
            
            // Evaluate arguments
            std::vector<Value> arguments;
            for (const auto& argument : expr.arguments) {
                arguments.push_back(evaluate(*argument));
            }
            
            // Use SuperHandler to call the super method
            try {
                result = superHandler->callSuperMethod(currentMethodClass, memberAccess->member.lexeme, arguments, environment);
                return;
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Failed to call super method '" + memberAccess->member.lexeme + "': " + e.what());
            }
        }
    }
    
    Value calleeValue = evaluate(*expr.callee);
    
    // Check if callee is MemberAccess
    if (auto memberAccess = dynamic_cast<const MemberAccess*>(expr.callee.get())) {
        std::cout << "DEBUG: This is a member access call: " << memberAccess->member.lexeme << std::endl;

        
        // First check if this is a nested type constructor call
        if (auto varExpr = dynamic_cast<const VarExpr*>(memberAccess->object.get())) {
            std::string fullTypeName = varExpr->name.lexeme + "." + memberAccess->member.lexeme;
            // Check if this is a nested struct constructor
            if (auto* structPropManager = getStructPropertyManager(fullTypeName)) {
                // Create a new struct instance
                auto propContainer = std::make_shared<InstancePropertyContainer>(*structPropManager, environment);
                propContainer->initializeDefaults(*this);
                StructValue structValue(fullTypeName, propContainer);
                
                // Copy subscripts from static manager to instance
                auto* typeSubscriptManager = staticSubscriptManager->getSubscriptManager(fullTypeName);
                if (typeSubscriptManager) {
                    for (const auto& subscript : typeSubscriptManager->getAllSubscripts()) {
                        structValue.subscripts->addSubscript(std::make_unique<SubscriptValue>(*subscript));
                    }
                }
                
                result = Value(structValue);
                return;
            }
            
            // Check if this is a nested class constructor
            if (auto* classPropManager = getClassPropertyManager(fullTypeName)) {
                // Create a new class instance
                auto propContainer = std::make_unique<InstancePropertyContainer>(*classPropManager, environment);
                propContainer->initializeDefaultsWithInheritance(*this, fullTypeName);
                auto classInstance = std::make_shared<ClassInstance>(fullTypeName, std::move(propContainer));
                result = Value(classInstance);
                return;
            }
        }
        
        // Handle method calls on objects (including extension methods)
        Value object = evaluate(*memberAccess->object);
        
        // Check for built-in array methods
        if (object.isArray()) {
            if (memberAccess->member.lexeme == "append") {
                if (expr.arguments.size() != 1) {
                    throw std::runtime_error("Array.append expects exactly 1 argument.");
                }
                
                Value argument = evaluate(*expr.arguments[0]);
                auto array = object.asArray();
                array->push_back(argument);
                result = Value(); // append returns void
                return;
            }
            // Add other array methods here if needed
            // For arrays, we don't call getMemberValue for unknown methods
            // as it will throw "Array has no member" error
            throw std::runtime_error("Array has no member '" + memberAccess->member.lexeme + "'");
        }
        
        Value method;
        try {
            method = getMemberValue(object, memberAccess->member.lexeme);

        } catch (const std::runtime_error& e) {

            throw;
        }
        
        if (method.isFunction()) {
            auto callable = method.asFunction();
            
            if (callable->isFunction) {
                std::cout << "DEBUG: Executing method call through MemberAccess path" << std::endl;
                // For inherited methods, the function definition doesn't include 'self' parameter
                // but we need to execute it in the context of the current object
                std::vector<Value> arguments;
                
                // Add the provided arguments (no self parameter for inherited methods)
                for (const auto& argument : expr.arguments) {
                    arguments.push_back(evaluate(*argument));
                }
                
                // Check parameter count - inherited methods should match exactly
                if (arguments.size() != callable->functionDecl->parameters.size()) {
                    throw std::runtime_error("Expected " + 
                        std::to_string(callable->functionDecl->parameters.size()) + 
                        " arguments but got " + std::to_string(arguments.size()) + ".");
                }
                

                
                // Create new environment for function execution
                auto previous = environment;
                environment = std::make_shared<Environment>(callable->closure);
                
                // Set current class context for super keyword support
                if (object.isClass()) {
                    auto classInstance = object.asClass();
                    environment->define("__current_class__", Value(classInstance->getClassName()), false, "String");
                    
                    // Find which class this method is actually defined in
                    std::string methodDefiningClass = classInstance->getClassName();
                    auto currentClassMethod = inheritanceManager->findMethodInClass(classInstance->getClassName(), memberAccess->member.lexeme);
                    if (!currentClassMethod) {
                        // Method not found in current class, find in inheritance chain
                        auto inheritanceChain = inheritanceManager->getInheritanceChain(classInstance->getClassName());
                        for (const auto& ancestorClass : inheritanceChain) {
                            if (inheritanceManager->findMethodInClass(ancestorClass, memberAccess->member.lexeme)) {
                                methodDefiningClass = ancestorClass;
                                break;
                            }
                        }
                    }
                    environment->define("__current_method_class__", Value(methodDefiningClass), false, "String");
                    // std::cout << "DEBUG: Set __current_method_class__ to: " << methodDefiningClass << " for method: " << memberAccess->member.lexeme << std::endl;
                }
                
                // Create new defer stack level for this method
                deferStack.push(std::vector<std::unique_ptr<Stmt>>());
                
                // Bind 'self' parameter for instance method
                // std::cout << "DEBUG: Binding self parameter in MemberAccess path" << std::endl;
                environment->define("self", object, false, "Self");
                
                // Bind the actual method parameters
                for (size_t i = 0; i < callable->functionDecl->parameters.size(); ++i) {
                    environment->define(
                        callable->functionDecl->parameters[i].name.lexeme,
                        arguments[i], // Use arguments directly
                        false, // parameters are not const
                        callable->functionDecl->parameters[i].type.lexeme
                    );
                }
                
                try {
                    // Execute function body
                    callable->functionDecl->body->accept(*this);
                    
                    // Execute deferred statements before function exits
                    executeDeferredStatements();
                    
                    // If no return statement was executed, return nil
                    result = Value();
                } catch (const ReturnException& returnValue) {
                    // Execute deferred statements before function exits
                    executeDeferredStatements();
                    
                    // Function returned a value
                    result = *returnValue.value;
                }
                
                // Restore previous environment
                environment = previous;
                return;
            }
        }
        
        // If not a method call, fall through to regular call handling
    } else if (auto varExpr = dynamic_cast<const VarExpr*>(expr.callee.get())) {
        // Check if this is a class constructor call
        if (auto* classPropManager = getClassPropertyManager(varExpr->name.lexeme)) {
            
            // Create a new class instance
            auto propContainer = std::make_unique<InstancePropertyContainer>(*classPropManager, environment);
            propContainer->initializeDefaults(*this);
            auto classInstance = std::make_shared<ClassInstance>(varExpr->name.lexeme, std::move(propContainer));
            result = Value(classInstance);
            return;
        }
        
        // Check if this is a struct constructor call
        if (auto* structPropManager = getStructPropertyManager(varExpr->name.lexeme)) {
            
            // Create a new struct instance
            auto propContainer = std::make_shared<InstancePropertyContainer>(*structPropManager, environment);
            propContainer->initializeDefaults(*this);
            StructValue structValue(varExpr->name.lexeme, propContainer);
            
            // Copy subscripts from static manager to instance
            auto* typeSubscriptManager = staticSubscriptManager->getSubscriptManager(varExpr->name.lexeme);
            if (typeSubscriptManager) {
                for (const auto& subscript : typeSubscriptManager->getAllSubscripts()) {
                    structValue.subscripts->addSubscript(std::make_unique<SubscriptValue>(*subscript));
                }
            }
            
            result = Value(structValue);
            return;
        }
    } else if (auto superExpr = dynamic_cast<const Super*>(expr.callee.get())) {
        std::cout << "DEBUG: This is a super method call: " << superExpr->method.lexeme << std::endl;
        
        // Handle super method call: super.method(arguments)
        // Get current method class context from environment
        std::string currentMethodClass;
        try {
            Value methodClassContext = environment->get(Token(TokenType::Identifier, "__current_method_class__", 0));
            if (methodClassContext.type == ValueType::String) {
                currentMethodClass = std::get<std::string>(methodClassContext.value);
            } else {
                throw std::runtime_error("super can only be used within class methods");
            }
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("super can only be used within class methods");
        }
        
        // Get the superclass of the current method's class
        auto superclass = inheritanceManager->getSuperclass(currentMethodClass);
        if (superclass.empty()) {
            throw std::runtime_error("Class '" + currentMethodClass + "' has no superclass");
        }
        
        // Find the method in the superclass
        auto superMethod = inheritanceManager->findMethodInClass(superclass, superExpr->method.lexeme);
        if (!superMethod) {
            throw std::runtime_error("Method '" + superExpr->method.lexeme + "' not found in superclass '" + superclass + "'");
        }
        
        // Get self object from environment
        Value selfObject;
        try {
            selfObject = environment->get(Token(TokenType::Identifier, "self", 0));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("super can only be used within instance methods");
        }
        
        // Evaluate arguments
        std::vector<Value> arguments;
        for (const auto& argument : expr.arguments) {
            arguments.push_back(evaluate(*argument));
        }
        
        // Check parameter count
        if (arguments.size() != superMethod->parameters.size()) {
            throw std::runtime_error("Expected " + 
                std::to_string(superMethod->parameters.size()) + 
                " arguments but got " + std::to_string(arguments.size()) + ".");
        }
        
        // Create new environment for super method execution using MethodCallEnvironment
        auto previous = environment;
        environment = std::make_shared<MethodCallEnvironment>(previous, selfObject, this);
        std::cout << "DEBUG: Created MethodCallEnvironment for super method execution" << std::endl;
        
        // Set current class context for nested super calls
        environment->define("__current_class__", Value(superclass), false, "String");
        environment->define("__current_method_class__", Value(superclass), false, "String");
        
        // Create new defer stack level for this method
        deferStack.push(std::vector<std::unique_ptr<Stmt>>());
        
        // Note: 'self' is already bound by MethodCallEnvironment constructor
        
        // Bind the actual method parameters
        for (size_t i = 0; i < superMethod->parameters.size(); ++i) {
            environment->define(
                superMethod->parameters[i].name.lexeme,
                arguments[i],
                false, // parameters are not const
                superMethod->parameters[i].type.lexeme
            );
        }
        
        try {
            // Execute super method body
            superMethod->body->accept(*this);
            
            // Execute deferred statements before method exits
            executeDeferredStatements();
            
            // If no return statement was executed, return nil
            result = Value();
        } catch (const ReturnException& returnValue) {
            // Execute deferred statements before method exits
            executeDeferredStatements();
            
            // Method returned a value
            result = *returnValue.value;
        }
        
        // Restore previous environment
        environment = previous;
        return;
    }
    
    Value callee = evaluate(*expr.callee);
    
    if (!callee.isFunction()) {
        throw std::runtime_error("Can only call functions and closures.");
    }
    
    std::vector<Value> arguments;
    for (const auto& argument : expr.arguments) {
        arguments.push_back(evaluate(*argument));
    }
    
    auto callable = callee.asFunction();
    
    if (callable->isFunction) {
        // Check if this is a constructor call by examining the function name
        std::string functionName = callable->functionDecl->name.lexeme;
        
        // Check if this is a struct constructor
        if (auto* structPropManager = getStructPropertyManager(functionName)) {
            // Create a new struct instance
            auto propContainer = std::make_shared<InstancePropertyContainer>(*structPropManager, environment);
            propContainer->initializeDefaults(*this);
            StructValue structValue(functionName, propContainer);
            
            // Copy subscripts from static manager to instance
            auto* typeSubscriptManager = staticSubscriptManager->getSubscriptManager(functionName);
            if (typeSubscriptManager) {
                for (const auto& subscript : typeSubscriptManager->getAllSubscripts()) {
                    structValue.subscripts->addSubscript(std::make_unique<SubscriptValue>(*subscript));
                }
            }
            
            result = Value(structValue);
            return;
        }
        
        // Check if this is a class constructor
        if (auto* classPropManager = getClassPropertyManager(functionName)) {
            // Create a new class instance
            auto propContainer = std::make_unique<InstancePropertyContainer>(*classPropManager, environment);
            propContainer->initializeDefaults(*this);
            auto classInstance = std::make_shared<ClassInstance>(functionName, std::move(propContainer));
            result = Value(classInstance);
            return;
        }
        
        // Check if this is a super method call
        std::cout << "DEBUG: FunctionCall - Checking for super method call" << std::endl;
        try {
            Value isSuperMethod = callable->closure->get(Token{TokenType::Identifier, "__is_super_method__", 0});
            std::cout << "DEBUG: FunctionCall - Found __is_super_method__ marker" << std::endl;
            if (isSuperMethod.type == ValueType::Bool && std::get<bool>(isSuperMethod.value)) {
                std::cout << "DEBUG: Detected super method call, delegating to SuperHandler" << std::endl;
                Value superClass = callable->closure->get(Token{TokenType::Identifier, "__super_class__", 0});
                Value superMethod = callable->closure->get(Token{TokenType::Identifier, "__super_method__", 0});
                
                std::string currentClass = std::get<std::string>(superClass.value);
                std::string methodName = std::get<std::string>(superMethod.value);
                
                result = superHandler->callSuperMethod(currentClass, methodName, arguments, environment);
                return;
            } else {
                std::cout << "DEBUG: FunctionCall - __is_super_method__ is false or not bool" << std::endl;
            }
        } catch (const std::runtime_error& e) {
            std::cout << "DEBUG: FunctionCall - Exception when checking super method: " << e.what() << std::endl;
            // Not a super method call, continue with regular function call
        }
        
        // Handle regular function call
        if (arguments.size() != callable->functionDecl->parameters.size()) {
            throw std::runtime_error("Expected " + 
                std::to_string(callable->functionDecl->parameters.size()) + 
                " arguments but got " + std::to_string(arguments.size()) + ".");
        }
        
        // Create new environment for function execution
        auto previous = environment;
        environment = std::make_shared<Environment>(callable->closure);
        
        // Create new defer stack level for this function
        deferStack.push(std::vector<std::unique_ptr<Stmt>>());
        
        // Bind parameters
        for (size_t i = 0; i < callable->functionDecl->parameters.size(); ++i) {
            environment->define(
                callable->functionDecl->parameters[i].name.lexeme,
                arguments[i],
                false, // parameters are not const
                callable->functionDecl->parameters[i].type.lexeme
            );
        }
        
        try {
            // Execute function body
            callable->functionDecl->body->accept(*this);
            
            // Execute deferred statements before function exits
            executeDeferredStatements();
            
            // If no return statement was executed, return nil
            result = Value();
        } catch (const ReturnException& returnValue) {
            // Execute deferred statements before function exits
            executeDeferredStatements();
            
            // Function returned a value
            result = *returnValue.value;
        }
        
        // Restore previous environment
        environment = previous;
    } else {
        // Handle closure call
        if (arguments.size() != callable->closureDecl->parameters.size()) {
            throw std::runtime_error("Expected " + 
                std::to_string(callable->closureDecl->parameters.size()) + 
                " arguments but got " + std::to_string(arguments.size()) + ".");
        }
        
        // Create new environment for closure execution
        auto previous = environment;
        environment = std::make_shared<Environment>(callable->closure);
        
        // Create new defer stack level for this closure
        deferStack.push(std::vector<std::unique_ptr<Stmt>>());
        
        // Bind parameters
        for (size_t i = 0; i < callable->closureDecl->parameters.size(); ++i) {
            environment->define(
                callable->closureDecl->parameters[i].name.lexeme,
                arguments[i],
                false, // parameters are not const
                callable->closureDecl->parameters[i].type.lexeme
            );
        }
        
        try {
            // Execute closure body
            for (const auto& statement : callable->closureDecl->body) {
                statement->accept(*this);
            }
            
            // Execute deferred statements before closure exits
            executeDeferredStatements();
            
            // If no return statement was executed, return nil
            result = Value();
        } catch (const ReturnException& returnValue) {
            // Execute deferred statements before closure exits
            executeDeferredStatements();
            
            // Closure returned a value
            result = *returnValue.value;
        }
        
        // Restore previous environment
        environment = previous;
    }
}

// Execute labeled function call: callee(label1: arg1, label2: arg2)
void Interpreter::visit(const LabeledCall& expr) {
    // std::cout << "DEBUG: LabeledCall expression detected" << std::endl;
    
    // Check if this is a super method call BEFORE evaluating the callee
    if (auto memberAccess = dynamic_cast<const MemberAccess*>(expr.callee.get())) {
        if (auto superExpr = dynamic_cast<const Super*>(memberAccess->object.get())) {
            // std::cout << "DEBUG: This is a super method call via LabeledCall: " << memberAccess->member.lexeme << std::endl;
            
            // Get current method class context from environment
            std::string currentMethodClass;
            try {
                Value methodClassContext = environment->get(Token(TokenType::Identifier, "__current_method_class__", 0));
                if (methodClassContext.type == ValueType::String) {
                    currentMethodClass = std::get<std::string>(methodClassContext.value);
                } else {
                    throw std::runtime_error("super can only be used within class methods");
                }
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("super can only be used within class methods");
            }
            
            // Evaluate arguments
            std::vector<Value> arguments;
            for (const auto& argument : expr.arguments) {
                arguments.push_back(evaluate(*argument));
            }
            
            // Use SuperHandler to call the super method
            try {
                result = superHandler->callSuperMethod(currentMethodClass, memberAccess->member.lexeme, arguments, environment);
                return;
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Failed to call super method '" + memberAccess->member.lexeme + "': " + e.what());
            }
        }
    }
    
    // Check if callee is MemberAccess for special method handling
    if (auto memberAccess = dynamic_cast<const MemberAccess*>(expr.callee.get())) {
        // Handle array append method
        if (memberAccess->member.lexeme == "append") {
            Value object = evaluate(*memberAccess->object);
            if (object.isArray()) {
                if (expr.arguments.size() != 1) {
                    throw std::runtime_error("Array append expects exactly 1 argument.");
                }
                Value newElement = evaluate(*expr.arguments[0]);
                 auto& arrayValue = object.asArrayRef();
                 arrayValue.push_back(newElement);
                result = Value(); // append returns void
                return;
            }
        }
        // For other member access, fall through to normal evaluation
    }
    
    // Check if this is a struct or class initialization call
    if (auto varExpr = dynamic_cast<const VarExpr*>(expr.callee.get())) {
        std::string typeName = varExpr->name.lexeme;
        
        // Check if this is a struct constructor
        if (auto* structPropManager = getStructPropertyManager(typeName)) {
            // Create a new struct instance
            auto propContainer = std::make_shared<InstancePropertyContainer>(*structPropManager, environment);
            propContainer->initializeDefaults(*this);
            StructValue structValue(typeName, propContainer);
            
            // Copy subscripts from static manager to instance
            auto* typeSubscriptManager = staticSubscriptManager->getSubscriptManager(typeName);
            if (typeSubscriptManager) {
                for (const auto& subscript : typeSubscriptManager->getAllSubscripts()) {
                    structValue.subscripts->addSubscript(std::make_unique<SubscriptValue>(*subscript));
                }
            }
            
            // Set member values using labeled arguments
            for (size_t i = 0; i < expr.argumentLabels.size() && i < expr.arguments.size(); ++i) {
                const std::string& memberName = expr.argumentLabels[i].lexeme;
                if (!memberName.empty()) {
                    Value memberValue = evaluate(*expr.arguments[i]);
                    structValue.properties->setProperty(*this, memberName, memberValue);
                }
            }
            
            result = Value(structValue);
            return;
        }
        
        // Check if this is a class constructor
        if (auto* classPropManager = getClassPropertyManager(typeName)) {
            // Create a new class instance with inheritance support
            auto propContainer = std::make_unique<InstancePropertyContainer>(*classPropManager, environment, *this, typeName);
            propContainer->initializeDefaultsWithInheritance(*this, typeName);
            ClassInstance classInstance(typeName, std::move(propContainer));
            
            // Copy subscripts from static manager to instance
            auto* typeSubscriptManager = staticSubscriptManager->getSubscriptManager(typeName);
            if (typeSubscriptManager) {
                for (const auto& subscript : typeSubscriptManager->getAllSubscripts()) {
                    classInstance.subscripts->addSubscript(std::make_unique<SubscriptValue>(*subscript));
                }
            }
            
            // Set member values using labeled arguments
            for (size_t i = 0; i < expr.argumentLabels.size() && i < expr.arguments.size(); ++i) {
                const std::string& memberName = expr.argumentLabels[i].lexeme;
                if (!memberName.empty()) {
                    Value memberValue = evaluate(*expr.arguments[i]);
                    classInstance.properties->setProperty(*this, memberName, memberValue);
                }
            }
            
            result = Value(std::make_shared<ClassInstance>(std::move(classInstance)));
            return;
        }
    }
    
    // Check if this is a method call (callee is a member access)
    if (auto memberAccess = dynamic_cast<const MemberAccess*>(expr.callee.get())) {
        Value object = evaluate(*memberAccess->object);
        Value method;
        
        try {
            method = getMemberValue(object, memberAccess->member.lexeme);
        } catch (const std::runtime_error& e) {
            throw;
        }
        
        if (method.isFunction()) {
            // This is a method call - prepare arguments with 'self' as first parameter
            std::vector<Value> arguments;
            arguments.push_back(object); // Add 'self' as first argument
            
            // Add the provided arguments
            for (const auto& argument : expr.arguments) {
                arguments.push_back(evaluate(*argument));
            }
            
            auto callable = method.asFunction();
            
            if (callable->isFunction) {
                // Handle function call with self parameter
                // For instance methods, we need to account for the implicit 'self' parameter
                // The method definition doesn't include 'self' in its parameter count
                size_t expectedArgs = callable->functionDecl->parameters.size();
                size_t actualArgs = arguments.size() - 1; // Subtract 1 for the 'self' parameter we added
                
                if (actualArgs != expectedArgs) {
                    throw std::runtime_error("Expected " + 
                        std::to_string(expectedArgs) + 
                        " arguments but got " + std::to_string(actualArgs) + ".");
                }
                
                // Create new environment for function execution
                auto previous = environment;
                environment = std::make_shared<Environment>(callable->closure);
                
                // Set current class context for super keyword support
                if (object.isClass()) {
                    auto classInstance = object.asClass();
                    environment->define("__current_class__", Value(classInstance->getClassName()), false, "String");
                    
                    // Find which class this method is actually defined in
                    std::string methodDefiningClass = classInstance->getClassName();
                    auto currentClassMethod = inheritanceManager->findMethodInClass(classInstance->getClassName(), memberAccess->member.lexeme);
                    if (!currentClassMethod) {
                        // Method not found in current class, find in inheritance chain
                        auto inheritanceChain = inheritanceManager->getInheritanceChain(classInstance->getClassName());
                        for (const auto& ancestorClass : inheritanceChain) {
                            if (inheritanceManager->findMethodInClass(ancestorClass, memberAccess->member.lexeme)) {
                                methodDefiningClass = ancestorClass;
                                break;
                            }
                        }
                    }
                    environment->define("__current_method_class__", Value(methodDefiningClass), false, "String");
                    // std::cout << "DEBUG: Set __current_method_class__ to: " << methodDefiningClass << " for method: " << memberAccess->member.lexeme << std::endl;
                }
                
                // Create new defer stack level for this method
                deferStack.push(std::vector<std::unique_ptr<Stmt>>());
                
                // Bind 'self' parameter for instance method
                // std::cout << "DEBUG: Binding self parameter in second method call path" << std::endl;
                environment->define("self", object, false, "Self");
                
                // Bind parameters (skip the first argument which is 'self')
                for (size_t i = 0; i < callable->functionDecl->parameters.size(); ++i) {
                    environment->define(
                        callable->functionDecl->parameters[i].name.lexeme,
                        arguments[i + 1], // Skip the 'self' argument
                        false, // parameters are not const
                        callable->functionDecl->parameters[i].type.lexeme
                    );
                }
                
                try {
                    // Execute function body
                    callable->functionDecl->body->accept(*this);
                    
                    // Execute deferred statements before function exits
                    executeDeferredStatements();
                    
                    // If no return statement was executed, return nil
                    result = Value();
                } catch (const ReturnException& returnValue) {
                    // Execute deferred statements before function exits
                    executeDeferredStatements();
                    
                    // Function returned a value
                    result = *returnValue.value;
                }
                
                // Restore previous environment
                environment = previous;
                return;
            }
        }
    }
    
    Value callee = evaluate(*expr.callee);
    
    if (!callee.isFunction()) {
        throw std::runtime_error("Can only call functions and closures.");
    }
    
    std::vector<Value> arguments;
    for (const auto& argument : expr.arguments) {
        arguments.push_back(evaluate(*argument));
    }
    
    auto callable = callee.asFunction();
    
    if (callable->isFunction) {
        // For labeled calls, we need to match arguments to parameters by label
        if (arguments.size() != callable->functionDecl->parameters.size()) {
            throw std::runtime_error("Expected " + 
                std::to_string(callable->functionDecl->parameters.size()) + 
                " arguments but got " + std::to_string(arguments.size()) + ".");
        }
        
        // Create new environment for function execution
        auto previous = environment;
        environment = std::make_shared<Environment>(callable->closure);
        
        // Create new defer stack level for this function
        deferStack.push(std::vector<std::unique_ptr<Stmt>>());
        
        // Bind parameters by matching labels to external parameter names
        std::vector<Value> orderedArguments(arguments.size());
        
        for (size_t i = 0; i < expr.argumentLabels.size(); ++i) {
            const std::string& label = expr.argumentLabels[i].lexeme;
            bool found = false;
            
            // Find the parameter with matching external name
            for (size_t j = 0; j < callable->functionDecl->parameters.size(); ++j) {
                const auto& param = callable->functionDecl->parameters[j];
                // Handle both labeled parameters and unlabeled parameters (with "_")
                if ((param.externalName.lexeme == label) || 
                    (param.externalName.lexeme == "_" && label.empty())) {
                    orderedArguments[j] = arguments[i];
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                // For unlabeled arguments, try to match by position
                if (label.empty() && i < callable->functionDecl->parameters.size()) {
                    const auto& param = callable->functionDecl->parameters[i];
                    if (param.externalName.lexeme == "_") {
                        orderedArguments[i] = arguments[i];
                        found = true;
                    }
                }
            }
            
            if (!found) {
                throw std::runtime_error("No parameter found for argument label '" + label + "'");
            }
        }
        
        // Bind parameters in the correct order
        for (size_t i = 0; i < callable->functionDecl->parameters.size(); ++i) {
            environment->define(
                callable->functionDecl->parameters[i].name.lexeme,
                orderedArguments[i],
                false, // parameters are not const
                callable->functionDecl->parameters[i].type.lexeme
            );
        }
        
        try {
            // Execute function body
            callable->functionDecl->body->accept(*this);
            
            // Execute deferred statements before function exits
            executeDeferredStatements();
            
            // If no return statement was executed, return nil
            result = Value();
        } catch (const ReturnException& returnValue) {
            // Execute deferred statements before function exits
            executeDeferredStatements();
            
            // Function returned a value
            result = *returnValue.value;
        }
        
        // Restore previous environment
        environment = previous;
    } else {
        // Handle closure call - closures don't support labeled arguments for now
        throw std::runtime_error("Labeled arguments not supported for closures.");
    }
}

// Execute closure expression: { (parameters) -> ReturnType in body }
void Interpreter::visit(const Closure& expr) {
    auto closure = std::make_shared<Function>(&expr, environment);
    result = Value(closure);
}

// Execute enum declaration: enum Name: RawType { cases }
void Interpreter::visit(const EnumStmt& stmt) {
    // Set current type context for subscript registration
    environment->define("__current_type__", Value(stmt.name.lexeme), false, "String");
    
    // Process subscripts
    for (const auto& subscript : stmt.subscripts) {
        subscript->accept(*this);
    }
    
    // Process nested types
    for (const auto& nestedType : stmt.nestedTypes) {
        // Set nested type context for proper naming
        std::string previousContext;
        try {
            Value context = environment->get(Token(TokenType::Identifier, "__nested_context__", 0));
            if (context.type == ValueType::String) {
                previousContext = std::get<std::string>(context.value);
            }
        } catch (const std::runtime_error&) {
            // No previous context
        }
        
        std::string nestedContext = stmt.name.lexeme;
        if (!previousContext.empty()) {
            nestedContext = previousContext + "." + stmt.name.lexeme;
        }
        environment->define("__nested_context__", Value(nestedContext), false, "String");
        
        nestedType->accept(*this);
        
        // Restore previous context
        if (!previousContext.empty()) {
            environment->assign(Token(TokenType::Identifier, "__nested_context__", 0), Value(previousContext));
        } else {
            // Clear the nested context by setting it to empty string
            environment->assign(Token(TokenType::Identifier, "__nested_context__", 0), Value(std::string()));
        }
    }
    
    // Store enum definition in environment for later use
    std::string typeName = stmt.name.lexeme;
    
    // Check if we're in a nested context
    try {
        Value context = environment->get(Token(TokenType::Identifier, "__nested_context__", 0));
        if (context.type == ValueType::String) {
            std::string nestedContext = std::get<std::string>(context.value);
            if (!nestedContext.empty()) {
                typeName = nestedContext + "." + stmt.name.lexeme;
            }
        }
    } catch (const std::runtime_error&) {
        // No nested context, use simple name
    }
    
    environment->define(typeName, Value(), false, "Enum");
    
    // Clean up type context
    try {
        environment->assign(Token(TokenType::Identifier, "__current_type__", 0), Value());
    } catch (...) {
        // Ignore if __current_type__ doesn't exist
    }
}

// Execute enum access: EnumType.caseName or EnumType.caseName(arguments)
void Interpreter::visit(const EnumAccess& expr) {
    // Get the enum type name from the expression
    std::string enumTypeName;
    if (expr.enumType) {
        if (auto varExpr = dynamic_cast<const VarExpr*>(expr.enumType.get())) {
            enumTypeName = varExpr->name.lexeme;
        } else {
            throw std::runtime_error("Invalid enum access expression");
        }
    } else {
        // Handle shorthand .caseName syntax - would need context to determine enum type
        throw std::runtime_error("Shorthand enum access not yet supported");
    }
    
    // Evaluate associated values if any
    std::vector<Value> associatedValues;
    for (const auto& arg : expr.arguments) {
        associatedValues.push_back(evaluate(*arg));
    }
    
    // Create enum value
    EnumValue enumValue(enumTypeName, expr.caseName.lexeme, std::move(associatedValues));
    result = Value(std::make_shared<EnumValue>(enumValue));
}

// Execute struct declaration: struct Name { members }
void Interpreter::visit(const StructStmt& stmt) {
    // Set current type context for subscript registration
    environment->define("__current_type__", Value(stmt.name.lexeme), false, "String");
    
    // Store struct definition in environment for later use
    std::string typeName = stmt.name.lexeme;
    
    // Check if we're in a nested context (but only for nested types, not the current type)
    std::string currentNestedContext;
    try {
        Value context = environment->get(Token(TokenType::Identifier, "__nested_context__", 0));
        if (context.type == ValueType::String) {
            currentNestedContext = std::get<std::string>(context.value);
            // Only use nested context if it doesn't end with our own name
             // (to avoid OuterStruct.OuterStruct situation)
             std::string suffix = "." + stmt.name.lexeme;
             if (!currentNestedContext.empty() && 
                 currentNestedContext != stmt.name.lexeme &&
                 (currentNestedContext.length() < suffix.length() || 
                  currentNestedContext.substr(currentNestedContext.length() - suffix.length()) != suffix)) {
                 typeName = currentNestedContext + "." + stmt.name.lexeme;
             }
        }
    } catch (const std::runtime_error&) {
        // No nested context, use simple name
    }
    
    // Register struct properties with the full type name
    registerStructProperties(typeName, stmt.members);
    
    // Process struct methods - register them in global environment
    for (const auto& method : stmt.methods) {
        // Create mangled name for the method (TypeName.methodName)
        std::string mangledName = typeName + "." + method->name.lexeme;
        
        // Create a new parameter list with 'self' as the first parameter
        std::vector<Parameter> newParameters;
        
        // Add 'self' parameter as the first parameter
        Token selfToken{TokenType::Identifier, "self", 0};
        Token structTypeToken{TokenType::Identifier, typeName, 0};
        Parameter selfParam{selfToken, structTypeToken, false, false}; // name, type, isInout, isVariadic
        newParameters.push_back(selfParam);
        
        // Add original method parameters
        for (const auto& param : method->parameters) {
            newParameters.push_back(param);
        }
        
        // Clone the method body
        auto clonedBody = method->body->clone();
        
        // Create a new FunctionStmt with 'self' parameter
        auto newMethodStmt = std::make_shared<FunctionStmt>(
            method->name,
            std::move(newParameters),
            method->returnType,
            std::move(clonedBody),
            method->accessLevel,
            method->genericParams,
            method->whereClause,
            method->isMutating,
            method->canThrow,
            method->isAsync,
            method->isMain
        );
        
        // Store the new function statement to keep it alive
        methodFunctions[mangledName] = newMethodStmt;
        
        // Create a function value for the method
        auto callable = std::make_shared<Function>(newMethodStmt.get(), environment);
        
        // Store the method in the global environment
        globals->define(mangledName, Value(callable), false, "Function");
    }
    
    // Process subscripts
    for (const auto& subscript : stmt.subscripts) {
        subscript->accept(*this);
    }
    
    // Process nested types
    for (const auto& nestedType : stmt.nestedTypes) {
        // Set nested type context for proper naming
        std::string previousContext;
        try {
            Value context = environment->get(Token(TokenType::Identifier, "__nested_context__", 0));
            if (context.type == ValueType::String) {
                previousContext = std::get<std::string>(context.value);
            }
        } catch (const std::runtime_error&) {
            // No previous context
        }
        
        std::string nestedContext = stmt.name.lexeme;
        if (!previousContext.empty()) {
            nestedContext = previousContext + "." + stmt.name.lexeme;
        }
        environment->define("__nested_context__", Value(nestedContext), false, "String");
        
        nestedType->accept(*this);
        
        // Restore previous context
        if (!previousContext.empty()) {
            environment->assign(Token(TokenType::Identifier, "__nested_context__", 0), Value(previousContext));
        } else {
            // Clear the nested context by setting it to empty string
            environment->assign(Token(TokenType::Identifier, "__nested_context__", 0), Value(std::string()));
        }
    }
    
    // Check if property manager was created
    auto* propManager = getStructPropertyManager(typeName);
    if (!propManager) {
        std::cout << "Failed to create property manager for struct: " << typeName << std::endl;
    }
    
    // Create a constructor for the struct type
    Token constructorName{TokenType::Identifier, stmt.name.lexeme, 0};
    std::vector<Parameter> emptyParams;
    Token voidType{TokenType::Identifier, "Void", 0};
    
    // Create constructor body that creates and returns a struct instance
    std::vector<std::unique_ptr<Stmt>> constructorStmts;
    auto constructorBody = std::make_unique<BlockStmt>(std::move(constructorStmts));
    
    // Create the constructor function statement
    auto constructorFunc = std::make_shared<FunctionStmt>(
        constructorName,
        emptyParams,
        voidType,
        std::move(constructorBody)
    );
    
    // Store the function statement to keep it alive
    constructorFunctions[typeName] = constructorFunc;
    
    // Create a special callable that creates struct instances
    auto callable = std::make_shared<Function>(constructorFunc.get(), environment);
    
    // Store the constructor in the environment with the type name
    environment->define(typeName, Value(callable), false, "Constructor");
    
    // Clean up type context
    try {
        environment->assign(Token(TokenType::Identifier, "__current_type__", 0), Value());
    } catch (...) {
        // Ignore if __current_type__ doesn't exist
    }
}

// Execute class declaration: class Name { members }
void Interpreter::visit(const ClassStmt& stmt) {
    // Set current type context for subscript registration
    environment->define("__current_type__", Value(stmt.name.lexeme), false, "String");
    
    // Store class definition in environment for later use
    std::string typeName = stmt.name.lexeme;
    
    // Check if we're in a nested context
    try {
        Value context = environment->get(Token(TokenType::Identifier, "__nested_context__", 0));
        if (context.type == ValueType::String) {
            std::string nestedContext = std::get<std::string>(context.value);
            if (!nestedContext.empty()) {
                typeName = nestedContext + "." + stmt.name.lexeme;
            }
        }
    } catch (const std::runtime_error&) {
        // No nested context, use simple name
    }
    
    // Register class properties with the full type name
    registerClassProperties(typeName, stmt.members);
    
    // Process subscripts
    for (const auto& subscript : stmt.subscripts) {
        subscript->accept(*this);
    }
    
    // Process nested types
    for (const auto& nestedType : stmt.nestedTypes) {
        // Set nested type context for proper naming
        std::string previousContext;
        try {
            Value context = environment->get(Token(TokenType::Identifier, "__nested_context__", 0));
            if (context.type == ValueType::String) {
                previousContext = std::get<std::string>(context.value);
            }
        } catch (const std::runtime_error&) {
            // No previous context
        }
        
        std::string nestedContext = stmt.name.lexeme;
        if (!previousContext.empty()) {
            nestedContext = previousContext + "." + stmt.name.lexeme;
        }
        environment->define("__nested_context__", Value(nestedContext), false, "String");
        
        nestedType->accept(*this);
        
        // Restore previous context
        if (!previousContext.empty()) {
            environment->assign(Token(TokenType::Identifier, "__nested_context__", 0), Value(previousContext));
        } else {
            // Clear the nested context by setting it to empty string
            environment->assign(Token(TokenType::Identifier, "__nested_context__", 0), Value(std::string()));
        }
    }
    
    // Register inheritance relationship
    std::string superclassName = "";
    if (stmt.superclass.type != TokenType::Nil && !stmt.superclass.lexeme.empty()) {
        superclassName = stmt.superclass.lexeme;
    }
    inheritanceManager->registerClass(stmt.name.lexeme, superclassName);
    
    // Register class methods for inheritance
    for (const auto& method : stmt.methods) {
        inheritanceManager->registerMethod(stmt.name.lexeme, method->name.lexeme, 
                                         std::shared_ptr<FunctionStmt>(method.get(), [](FunctionStmt*) {}));
    }
    
    // Create a default constructor if no explicit constructor is defined
    bool hasExplicitConstructor = false;
    for (const auto& method : stmt.methods) {
        if (method->name.lexeme == "init") {
            hasExplicitConstructor = true;
            break;
        }
    }
    
    if (!hasExplicitConstructor) {
        // Create a default constructor that returns a new class instance
        Token constructorName{TokenType::Identifier, stmt.name.lexeme, 0};
        std::vector<Parameter> emptyParams;
        Token voidType{TokenType::Identifier, "Void", 0};
        
        // Create constructor body that creates and returns a class instance
        std::vector<std::unique_ptr<Stmt>> constructorStmts;
        
        // Create return statement that returns a new class instance
        // For now, we'll create an empty body and handle instance creation in the callable
        auto constructorBody = std::make_unique<BlockStmt>(std::move(constructorStmts));
        
        // Create the constructor function statement
        auto constructorFunc = std::make_shared<FunctionStmt>(
            constructorName,
            emptyParams,
            voidType,
            std::move(constructorBody)
        );
        
        // Store the function statement to keep it alive
        constructorFunctions[typeName] = constructorFunc;
        
        // Create a special callable that creates class instances
        auto callable = std::make_shared<Function>(constructorFunc.get(), environment);
        
        // Store the constructor in the environment with the type name
        environment->define(typeName, Value(callable), false, "Constructor");
    }
    
    // For classes with explicit constructors, store class definition
    if (hasExplicitConstructor) {
        environment->define(typeName, Value(), false, "Class");
    }
    
    // Clean up type context
    try {
        environment->assign(Token(TokenType::Identifier, "__current_type__", 0), Value());
    } catch (...) {
        // Ignore if __current_type__ doesn't exist
    }
}

// Execute init declaration: init(parameters) { body }
void Interpreter::visit(const InitStmt& stmt) {
    // Create constructor definition with proper parameters
    InitType initType = stmt.initType;
    
    // Clone the body to create a unique_ptr<BlockStmt>
    auto clonedStmt = stmt.body->clone();
    auto bodyClone = std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(clonedStmt.release()));
    
    // Create constructor definition
    ConstructorDefinition constructorDef(initType, stmt.parameters, std::move(bodyClone));
    
    // Create constructor value and store in environment
    auto constructorValue = std::make_shared<ConstructorValue>(constructorDef, environment);
    environment->define("init", Value(constructorValue), false, "Constructor");
}

// Execute deinit declaration: deinit { body }
void Interpreter::visit(const DeinitStmt& stmt) {
    // Clone the body to create a unique_ptr<BlockStmt>
    auto clonedStmt = stmt.body->clone();
    auto bodyClone = std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(clonedStmt.release()));
    
    // Create destructor definition
    DestructorDefinition destructorDef(std::move(bodyClone));
    
    // Create destructor value and store in environment
    auto destructorValue = std::make_shared<DestructorValue>(destructorDef, environment);
    environment->define("deinit", Value(destructorValue), false, "Destructor");
}

// Execute subscript declaration: subscript(parameters) -> ReturnType { get { } set { } }
void Interpreter::visit(const SubscriptStmt& stmt) {
    std::cout << "Subscript declaration processed with " << stmt.parameters.size() << " parameters" << std::endl;
    
    // Get current type context (should be set when processing struct/class)
    std::string currentType;
    try {
        Value typeContext = environment->get(Token(TokenType::Identifier, "__current_type__", 0));
        if (typeContext.type == ValueType::String) {
            currentType = std::get<std::string>(typeContext.value);
        } else {
            throw std::runtime_error("subscript can only be declared within struct or class");
        }
    } catch (const std::runtime_error&) {
        throw std::runtime_error("subscript can only be declared within struct or class");
    }
    
    // Extract getter and setter from accessors
    std::unique_ptr<BlockStmt> getter = nullptr;
    std::unique_ptr<BlockStmt> setter = nullptr;
    
    for (const auto& accessor : stmt.accessors) {
        if (accessor.type == AccessorType::GET) {
            getter = std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(accessor.body->clone().release()));
        } else if (accessor.type == AccessorType::SET) {
            setter = std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(accessor.body->clone().release()));
        }
    }
    
    if (!getter) {
        throw std::runtime_error("subscript must have a getter");
    }
    
    // Create subscript definition
    auto subscriptDef = SubscriptDefinition(
        stmt.parameters,
        stmt.returnType,
        std::move(getter),
        std::move(setter)
    );
    
    // Create subscript value with current environment as closure
    auto subscriptValue = std::make_unique<SubscriptValue>(std::move(subscriptDef), environment);
    
    // Register with appropriate manager
    if (stmt.isStatic) {
        staticSubscriptManager->registerStaticSubscript(currentType, std::move(subscriptValue));
    } else {
        // For instance subscripts, we need to register with the type's subscript manager
        // This will be handled when creating instances
        staticSubscriptManager->getSubscriptManager(currentType)->addSubscript(std::move(subscriptValue));
    }
    
    std::cout << "Subscript registered for type: " << currentType << std::endl;
}

// Execute member access: object.member
void Interpreter::visit(const MemberAccess& expr) {
    // Check if this is a super member access
    if (auto superExpr = dynamic_cast<const Super*>(expr.object.get())) {
        std::cout << "DEBUG: MemberAccess on Super expression for member: " << expr.member.lexeme << std::endl;
        
        // Get current method class context from environment
        std::string currentMethodClass;
        try {
            Value methodClassContext = environment->get(Token(TokenType::Identifier, "__current_method_class__", 0));
            if (methodClassContext.type == ValueType::String) {
                currentMethodClass = std::get<std::string>(methodClassContext.value);
            } else {
                throw std::runtime_error("super can only be used within class methods");
            }
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("super can only be used within class methods");
        }
        
        // Use SuperHandler to get the super property/method
        try {
            result = superHandler->getSuperProperty(currentMethodClass, expr.member.lexeme, environment);
            return;
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Failed to access super member '" + expr.member.lexeme + "': " + e.what());
        }
    }
    
    // Check if this is a type access (e.g., OuterType.NestedType)
    if (auto varExpr = dynamic_cast<const VarExpr*>(expr.object.get())) {
        std::string typeName = varExpr->name.lexeme;
        
        // Check if this is accessing a nested type
        std::string nestedTypeName = typeName + "." + expr.member.lexeme;
        try {
            Value nestedType = environment->get(Token(TokenType::Identifier, nestedTypeName, 0));
            result = nestedType;
            return;
        } catch (const std::runtime_error&) {
            // Not a nested type, check if this is an enum access
            try {
                Value enumType = environment->get(Token(TokenType::Identifier, typeName, 0));
    
                // Check if this is an enum type by checking if it exists and is not a function or struct
                if (enumType.type == ValueType::Nil || (enumType.type == ValueType::Enum)) {
        
                    // This might be an enum type, try to create enum value
                    EnumValue enumValue(typeName, expr.member.lexeme, {});
                    result = Value(std::make_shared<EnumValue>(enumValue));
                    return;
                }
    
            } catch (const std::runtime_error&) {
                // Not an enum type either
            }
            
            // Check if the object is a type name (constructor)
            try {
                Value typeConstructor = environment->get(Token(TokenType::Identifier, typeName, 0));
                if (typeConstructor.isFunction()) {
                    // This is a type name, use getMemberValue to handle it
                    result = getMemberValue(typeConstructor, expr.member.lexeme);
                    return;
                }
            } catch (const std::runtime_error&) {
                // Not a type name either, continue with normal member access
            }
        }
    }
    
    Value object = evaluate(*expr.object);
    result = getMemberValue(object, expr.member.lexeme);
}

// Execute struct initialization: StructName(member1: value1, member2: value2)
void Interpreter::visit(const StructInit& expr) {
    // Check if this is a class or struct
    auto* structPropManager = getStructPropertyManager(expr.structName.lexeme);
    auto* classPropManager = getClassPropertyManager(expr.structName.lexeme);
    

    
    if (classPropManager) {
        // Create class instance with inheritance support

        
        auto propContainer = std::make_unique<InstancePropertyContainer>(*classPropManager, environment, *this, expr.structName.lexeme);
        
        // Initialize provided members
        for (const auto& member : expr.members) {
            Value memberValue = evaluate(*member.second);

            propContainer->setProperty(*this, member.first.lexeme, memberValue);
        }
        
        // Initialize default values for unspecified properties (with inheritance support)
        propContainer->initializeDefaultsWithInheritance(*this, expr.structName.lexeme);
        
        auto classInstance = std::make_shared<ClassInstance>(expr.structName.lexeme, std::move(propContainer));
        
        // Copy subscripts from static manager to instance
        auto* typeSubscriptManager = staticSubscriptManager->getSubscriptManager(expr.structName.lexeme);
        if (typeSubscriptManager) {
            // Copy all subscripts from type to instance
            for (const auto& subscript : typeSubscriptManager->getAllSubscripts()) {
                classInstance->subscripts->addSubscript(std::make_unique<SubscriptValue>(*subscript));
            }
        }
        
        result = Value(classInstance);
    } else if (structPropManager) {
        // Create struct with property support

        
        auto propContainer = std::make_shared<InstancePropertyContainer>(*structPropManager, environment);
        
        // Initialize provided members
        for (const auto& member : expr.members) {
            Value memberValue = evaluate(*member.second);

            propContainer->setProperty(*this, member.first.lexeme, memberValue);
        }
        
        // Initialize default values for unspecified properties
        propContainer->initializeDefaults(*this);
        
        StructValue structValue(expr.structName.lexeme, propContainer);
        
        // Copy subscripts from static manager to instance
        auto* typeSubscriptManager = staticSubscriptManager->getSubscriptManager(expr.structName.lexeme);
        if (typeSubscriptManager) {
            // Copy all subscripts from type to instance
            for (const auto& subscript : typeSubscriptManager->getAllSubscripts()) {
                structValue.subscripts->addSubscript(std::make_unique<SubscriptValue>(*subscript));
            }
        }
        
        result = Value(structValue);
    } else {
        // Fallback: try to create class instance without property manager

        auto classInstance = std::make_shared<ClassInstance>(expr.structName.lexeme);
        
        // Initialize members with provided values
        for (const auto& member : expr.members) {
            Value memberValue = evaluate(*member.second);
            (*classInstance->members)[member.first.lexeme] = memberValue;
        }
        
        // Copy subscripts from static manager to instance
        auto* typeSubscriptManager = staticSubscriptManager->getSubscriptManager(expr.structName.lexeme);
        if (typeSubscriptManager) {
            // Copy all subscripts from type to instance
            for (const auto& subscript : typeSubscriptManager->getAllSubscripts()) {
                classInstance->subscripts->addSubscript(std::make_unique<SubscriptValue>(*subscript));
            }
        }
        
        result = Value(classInstance);
    }
}

// Property system helper methods

void Interpreter::executeWithEnvironment(const Stmt& stmt, std::shared_ptr<Environment> env) {
    auto previous = environment;
    environment = env;
    
    try {
        stmt.accept(*this);
    } catch (const ReturnException& returnValue) {
        // 捕获返回值并存储在环境中
        env->define("return", *returnValue.value);
        environment = previous;
        throw; // 重新抛出以便上层处理
    } catch (...) {
        environment = previous;
        throw;
    }
    
    environment = previous;
}

PropertyManager* Interpreter::getStructPropertyManager(const std::string& structName) {
    auto it = structPropertyManagers.find(structName);
    return (it != structPropertyManagers.end()) ? it->second.get() : nullptr;
}

PropertyManager* Interpreter::getClassPropertyManager(const std::string& className) {
    auto it = classPropertyManagers.find(className);
    return (it != classPropertyManagers.end()) ? it->second.get() : nullptr;
}

void Interpreter::registerStructProperties(const std::string& structName, const std::vector<StructMember>& members) {
    auto propManager = std::make_unique<PropertyManager>();
    
    for (const auto& member : members) {
        PropertyDefinition propDef(member.name, member.type);
        propDef.isVar = member.isVar;
        propDef.isStatic = member.isStatic;
        propDef.isLazy = member.isLazy;
        propDef.defaultValue = member.defaultValue ? member.defaultValue->clone() : nullptr;
        
        // Determine property type based on accessors
        if (member.isComputedProperty()) {
            propDef.propertyType = PropertyType::COMPUTED;
        } else if (member.isLazy) {
            propDef.propertyType = PropertyType::LAZY;
        } else {
            propDef.propertyType = PropertyType::STORED;
        }
        
        // Copy accessors with their bodies (using unique_ptr)
        for (const auto& accessor : member.accessors) {
            propDef.accessors.emplace_back(
                accessor.type,
                accessor.body ? std::unique_ptr<Stmt>(accessor.body->clone()) : nullptr,
                accessor.parameterName
            );
        }
        
        propManager->addProperty(std::move(propDef));
    }
    
    structPropertyManagers[structName] = std::move(propManager);
}

void Interpreter::registerClassProperties(const std::string& className, const std::vector<StructMember>& members) {
    auto propManager = std::make_unique<PropertyManager>();
    
    for (const auto& member : members) {
        PropertyDefinition propDef(member.name, member.type);
        propDef.isVar = member.isVar;
        propDef.isStatic = member.isStatic;
        propDef.isLazy = member.isLazy;
        propDef.defaultValue = member.defaultValue ? member.defaultValue->clone() : nullptr;
        
        // Determine property type based on accessors
        if (member.isComputedProperty()) {
            propDef.propertyType = PropertyType::COMPUTED;
        } else if (member.isLazy) {
            propDef.propertyType = PropertyType::LAZY;
        } else {
            propDef.propertyType = PropertyType::STORED;
        }
        
        // Copy accessors with their bodies (using unique_ptr)
        for (const auto& accessor : member.accessors) {
            propDef.accessors.emplace_back(
                accessor.type,
                accessor.body ? std::unique_ptr<Stmt>(accessor.body->clone()) : nullptr,
                accessor.parameterName
            );
        }
        
        propManager->addProperty(std::move(propDef));
    }
    
    classPropertyManagers[className] = std::move(propManager);
}



Value Interpreter::getMemberValue(const Value& object, const std::string& memberName) {


    
    // If the object is an optional, we cannot access its members directly
    if (object.isOptional()) {
        throw std::runtime_error("Cannot access member '" + memberName + "' on optional value. Use optional chaining (?.) instead.");
    }
    
    // Handle function types (constructors) - check for nested types
    if (object.isFunction()) {
        auto function = object.asFunction();
        // Try to find nested type with the function name + member name
        std::string functionName;
        if (function->isFunction && function->functionDecl) {
            functionName = function->functionDecl->name.lexeme;
        } else {
            throw std::runtime_error("Cannot access members on closure");
        }
        
        std::string nestedTypeName = functionName + "." + memberName;
        try {
            Value nestedType = environment->get(Token(TokenType::Identifier, nestedTypeName, 0));
            return nestedType;
        } catch (const std::runtime_error&) {
            throw std::runtime_error("Type '" + functionName + "' has no member '" + memberName + "'");
        }
    }
    
    if (object.isStruct()) {
        const auto& structValue = object.asStruct();
        
        
        // Try property system first
        if (structValue.properties && structValue.properties->hasProperty(memberName)) {

            return structValue.properties->getProperty(*this, memberName);
        }
        
        // Fallback to legacy member access
        auto it = structValue.members->find(memberName);
        if (it != structValue.members->end()) {

            return it->second;
        }
        
        // Try to find extension methods
        std::string mangledName = structValue.structName + "." + memberName;
        
        try {
            Value extensionMethod = globals->get(Token(TokenType::Identifier, mangledName, 0));

            if (extensionMethod.type == ValueType::Function) {
                // Create a bound method that includes the struct instance as 'self'
                auto function = extensionMethod.asFunction();
                // For now, return the function directly - the Call visitor will handle binding
                return extensionMethod;
            }
        } catch (const std::runtime_error& e) {

        }
        
        throw std::runtime_error("Struct '" + structValue.structName + "' has no member '" + memberName + "'");
    } else if (object.isClass()) {
        const auto& classValue = object.asClass();
        
        // Try property system first
        if (classValue->properties && classValue->properties->hasProperty(memberName)) {
            return classValue->properties->getProperty(*this, memberName);
        }
        
        // Fallback to legacy member access
        auto it = classValue->members->find(memberName);
        if (it != classValue->members->end()) {
            return it->second;
        }
        
        // Try to find methods in current class first, then inherited methods
        auto currentClassMethod = inheritanceManager->findMethodInClass(classValue->className, memberName);
        if (currentClassMethod) {
            // Create a callable from the current class method (including overridden methods)
            auto callable = std::make_shared<Function>(currentClassMethod.get(), environment);
            return Value(callable);
        }
        
        // If not found in current class, try inherited methods
        auto inheritedMethod = inheritanceManager->findMethodRecursive(inheritanceManager->getSuperclass(classValue->className), memberName);
        if (inheritedMethod) {
            // Create a callable from the inherited method
            auto callable = std::make_shared<Function>(inheritedMethod.get(), environment);
            return Value(callable);
        }
        
        // Try to find extension methods
        std::string mangledName = classValue->className + "." + memberName;
        try {
            Value extensionMethod = globals->get(Token(TokenType::Identifier, mangledName, 0));
            if (extensionMethod.type == ValueType::Function) {
                // Create a bound method that includes the class instance as 'self'
                auto function = extensionMethod.asFunction();
                // For now, return the function directly - the Call visitor will handle binding
                return extensionMethod;
            }
        } catch (const std::runtime_error&) {
            // Extension method not found, continue to error
        }
        
        throw std::runtime_error("Class '" + classValue->className + "' has no member '" + memberName + "'");
    } else if (object.isArray()) {
        // Handle built-in array properties
        if (memberName == "count") {
            // Return array size as a computed property
            return Value(static_cast<int>(object.asArrayRef().size()));
        } else if (memberName == "isEmpty") {
            // Return whether array is empty
            return Value(object.asArrayRef().empty());
        } else {
            throw std::runtime_error("Array has no member '" + memberName + "'");
        }
    } else if (object.isDictionary()) {
        // Handle super object (Dictionary type with __super_class__ key)
        const auto& dict = object.asDictionaryRef();
        auto superClassIt = dict.find("__super_class__");
        if (superClassIt != dict.end() && superClassIt->second.type == ValueType::String) {
            // This is a super object, use SuperHandler to get the property/method
            std::string currentClass = std::get<std::string>(superClassIt->second.value);
            return superHandler->getSuperProperty(currentClass, memberName, environment);
        } else {
            throw std::runtime_error("Dictionary has no member '" + memberName + "'");
        }
    } else {
        throw std::runtime_error("Only structs and classes have members");
    }
}

void Interpreter::setMemberValue(Value& object, const std::string& memberName, const Value& value) {
    if (object.isStruct()) {
        auto& structValue = object.asStruct();
        
        // Try property system first
        if (structValue.properties && structValue.properties->hasProperty(memberName)) {
            structValue.properties->setProperty(*this, memberName, value);
            return;
        }
        
        // Fallback to legacy member assignment
        (*structValue.members)[memberName] = value;
    } else if (object.isClass()) {
        auto& classValue = object.asClass();
        
        // Try property system first
        if (classValue->properties && classValue->properties->hasProperty(memberName)) {
            classValue->properties->setProperty(*this, memberName, value);
            return;
        }
        
        // Fallback to legacy member assignment
        (*classValue->members)[memberName] = value;
    } else {
        throw std::runtime_error("Only structs and classes have members");
    }
}

// Execute super expression: super keyword
void Interpreter::visit(const Super& expr) {
    // Get current method class context from environment
    std::string currentMethodClass;
    try {
        Value methodClassContext = environment->get(Token(TokenType::Identifier, "__current_method_class__", 0));
        if (methodClassContext.type == ValueType::String) {
            currentMethodClass = std::get<std::string>(methodClassContext.value);
        } else {
            throw std::runtime_error("super can only be used within class methods");
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("super can only be used within class methods");
    }
    
    // Create a special super value that contains the current method class
    std::unordered_map<std::string, Value> superInfo;
    superInfo["__super_class__"] = Value(currentMethodClass);
    result = Value(superInfo);
}

// Execute optional chaining expression: object?.property, object?.method(), object?[index]
void Interpreter::visit(const OptionalChaining& expr) {
    Value object = evaluate(*expr.object);
    
    // If object is nil or an optional with no value, return nil
    if (OptionalManager::isNil(object)) {
        result = OptionalManager::createNil();
        return;
    }
    
    // Unwrap optional if needed
    Value unwrappedObject = OptionalManager::safeUnwrap(object);
    if (OptionalManager::isNil(unwrappedObject)) {
        result = OptionalManager::createNil();
        return;
    }
    
    try {
        Value chainResult;
        
        switch (expr.chainType) {
            case OptionalChaining::ChainType::Property: {
                if (auto varExpr = dynamic_cast<const VarExpr*>(expr.accessor.get())) {
                    chainResult = OptionalManager::chainProperty(unwrappedObject, varExpr->name.lexeme, *this);
                } else {
                    throw std::runtime_error("Invalid property accessor in optional chaining");
                }
                break;
            }
            
            case OptionalChaining::ChainType::Method: {
                if (auto callExpr = dynamic_cast<const Call*>(expr.accessor.get())) {
                    chainResult = OptionalManager::chainMethod(unwrappedObject, callExpr, *this);
                } else {
                    throw std::runtime_error("Invalid method accessor in optional chaining");
                }
                break;
            }
            
            case OptionalChaining::ChainType::Subscript: {
                if (auto indexExpr = dynamic_cast<const IndexAccess*>(expr.accessor.get())) {
                    Value index = evaluate(*indexExpr->index);
                    chainResult = OptionalManager::chainSubscript(unwrappedObject, index, *this);
                } else {
                    throw std::runtime_error("Invalid subscript accessor in optional chaining");
                }
                break;
            }
        }
        
        // If chainResult is nil, return it directly; otherwise wrap in optional
        if (OptionalManager::isNil(chainResult)) {
            result = chainResult;
        } else {
            result = OptionalManager::createOptional(chainResult);
        }
        
    } catch (const std::runtime_error& e) {
        // If any error occurs during chaining, return nil
        result = OptionalManager::createNil();
    }
}

// Execute protocol declaration: protocol Name { requirements }
void Interpreter::visit(const ProtocolStmt& stmt) {
    std::cout << "Protocol declaration: " << stmt.name.lexeme << std::endl;
    
    // For now, we'll just register the protocol name in the environment
    // In a full implementation, this would involve:
    // 1. Creating a protocol registry
    // 2. Storing protocol requirements for conformance checking
    // 3. Implementing protocol dispatch mechanisms
    
    // Create a simple protocol representation
    std::unordered_map<std::string, Value> protocolInfo;
    protocolInfo["name"] = Value(stmt.name.lexeme);
    protocolInfo["requirements_count"] = Value(static_cast<double>(stmt.requirements.size()));
    
    // Store inherited protocols
    std::vector<Value> inheritedProtocolsList;
    for (const auto& inherited : stmt.inheritedProtocols) {
        inheritedProtocolsList.push_back(Value(inherited.lexeme));
    }
    protocolInfo["inherited_protocols"] = Value(inheritedProtocolsList);
    
    // Store protocol in global environment
    globals->define(stmt.name.lexeme, Value(protocolInfo));
    
    std::cout << "Protocol '" << stmt.name.lexeme << "' registered with " 
              << stmt.requirements.size() << " requirements" << std::endl;
}

// Execute extension declaration: extension TypeName: Protocol1, Protocol2 { members }
void Interpreter::visit(const ExtensionStmt& stmt) {
    // std::cout << "Extension declaration for: " << stmt.typeName.lexeme << std::endl;
    
    // Set current type context for member registration
    environment->define("__current_type__", Value(stmt.typeName.lexeme), false, "String");
    
    // Get existing property managers for the extended type
    auto* structPropManager = getStructPropertyManager(stmt.typeName.lexeme);
    auto* classPropManager = getClassPropertyManager(stmt.typeName.lexeme);
    
    if (!structPropManager && !classPropManager) {
        throw std::runtime_error("Cannot extend unknown type '" + stmt.typeName.lexeme + "'");
    }
    
    // Register computed properties from extension
    if (structPropManager) {
        for (const auto& property : stmt.properties) {
            PropertyDefinition propDef(property.name, property.type);
            propDef.isVar = property.isVar;
            propDef.isStatic = property.isStatic;
            propDef.isLazy = property.isLazy;
            propDef.propertyType = PropertyType::COMPUTED;
            
            // Copy accessors
            for (const auto& accessor : property.accessors) {
                propDef.accessors.emplace_back(
                    accessor.type,
                    accessor.body ? std::unique_ptr<Stmt>(accessor.body->clone()) : nullptr,
                    accessor.parameterName
                );
            }
            
            structPropManager->addProperty(std::move(propDef));
        }
    } else if (classPropManager) {
        for (const auto& property : stmt.properties) {
            PropertyDefinition propDef(property.name, property.type);
            propDef.isVar = property.isVar;
            propDef.isStatic = property.isStatic;
            propDef.isLazy = property.isLazy;
            propDef.propertyType = PropertyType::COMPUTED;
            
            // Copy accessors
            for (const auto& accessor : property.accessors) {
                propDef.accessors.emplace_back(
                    accessor.type,
                    accessor.body ? std::unique_ptr<Stmt>(accessor.body->clone()) : nullptr,
                    accessor.parameterName
                );
            }
            
            classPropManager->addProperty(std::move(propDef));
        }
    }
    
    // Register methods from extension
    for (const auto& method : stmt.methods) {
        // Create a function value and store it in the global environment
        // with a mangled name that includes the type name
        std::string mangledName = stmt.typeName.lexeme + "." + method->name.lexeme;
        auto function = std::make_shared<Function>(method.get(), environment);
        globals->define(mangledName, Value(function), false, "Function");
        
        // std::cout << "Registered extension method: " << mangledName << std::endl;
    }
    
    // Register convenience initializers from extension
    for (const auto& initializer : stmt.initializers) {
        // Create constructor definition
        auto clonedBody = initializer->body->clone();
        auto bodyClone = std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(clonedBody.release()));
        
        ConstructorDefinition constructorDef(initializer->initType, initializer->parameters, std::move(bodyClone));
        auto constructorValue = std::make_shared<ConstructorValue>(constructorDef, environment);
        
        // Store with mangled name
        std::string mangledName = stmt.typeName.lexeme + ".init";
        globals->define(mangledName, Value(constructorValue), false, "Constructor");
        
        // std::cout << "Registered extension initializer: " << mangledName << std::endl;
    }
    
    // Register subscripts from extension
    for (const auto& subscript : stmt.subscripts) {
        subscript->accept(*this);
    }
    
    // If extension adds protocol conformance, register it
    if (!stmt.conformedProtocols.empty()) {
        for (const auto& protocol : stmt.conformedProtocols) {
            std::cout << "Extension adds protocol conformance: " << stmt.typeName.lexeme 
                      << " : " << protocol.lexeme << std::endl;
            // In a full implementation, this would update the type's protocol conformance registry
        }
    }
    
    // Clean up type context
    try {
        environment->assign(Token(TokenType::Identifier, "__current_type__", 0), Value());
    } catch (...) {
        // Ignore if __current_type__ doesn't exist
    }
    
    // std::cout << "Extension for '" << stmt.typeName.lexeme << "' processed successfully" << std::endl;
}

// Execute range expression: start..<end or start...end
void Interpreter::visit(const Range& expr) {
    Value startValue = evaluate(*expr.start);
    Value endValue = evaluate(*expr.end);
    
    // For now, create a simple range representation as a map
    std::unordered_map<std::string, Value> rangeMap;
    rangeMap["start"] = startValue;
    rangeMap["end"] = endValue;
    rangeMap["type"] = Value(expr.rangeType == Range::RangeType::HalfOpen ? "half_open" : "closed");
    
    result = Value(rangeMap);
}

// Execute generic type instantiation: TypeName<Type1, Type2>
void Interpreter::visit(const GenericTypeInstantiationExpr& expr) {
    // For now, we'll treat generic type instantiation as a type identifier
    // In a full implementation, this would involve:
    // 1. Type checking and validation
    // 2. Generic type specialization
    // 3. Template instantiation
    
    // Create a simple representation that can be used for constructor calls
    result = Value(expr.typeName.lexeme);
}

// Error handling statement implementations
void Interpreter::visit(const ThrowStmt& stmt) {
    Value errorValue = evaluate(*stmt.error);
    
    // Execute any deferred statements before throwing
    executeDeferredStatements();
    
    // Create a simple runtime error from the value
    auto error = std::make_unique<RuntimeError>(valueToString(errorValue));
    throw ThrowException(std::move(error));
}

void Interpreter::visit(const DoCatchStmt& stmt) {
    // Create error context for this do-catch block
    ErrorContext context;
    for (const auto& catchClause : stmt.catchClauses) {
        if (!catchClause.errorType.empty()) {
            context.catchableErrorTypes.push_back(catchClause.errorType);
        }
    }
    
    pushErrorContext(context);
    
    try {
        // Execute the do block
        stmt.doBody->accept(*this);
    } catch (const ThrowException& e) {
        // Check if any catch clause can handle this error
        bool handled = false;
        
        for (const auto& catchClause : stmt.catchClauses) {
            if (!catchClause.errorType.empty()) {
                // Check if error type matches
                if (e.error.getDescription() == catchClause.errorType) {
                    // Bind error to variable if specified
                    if (!catchClause.variable.lexeme.empty()) {
                        environment->define(catchClause.variable.lexeme, 
                                          Value(e.error.getDescription()), false, "Error");
                    }
                    
                    // Execute catch block
                    catchClause.body->accept(*this);
                    handled = true;
                    break;
                }
            } else {
                // Catch-all clause
                if (!catchClause.variable.lexeme.empty()) {
                    environment->define(catchClause.variable.lexeme, 
                                      Value(e.error.getDescription()), false, "Error");
                }
                
                catchClause.body->accept(*this);
                handled = true;
                break;
            }
        }
        
        if (!handled) {
            // Re-throw if no catch clause handled the error
            popErrorContext();
            throw;
        }
    }
    
    popErrorContext();
}

void Interpreter::visit(const DeferStmt& stmt) {
    // Add the deferred statement to the defer stack
    pushDeferredStatement(stmt.body->clone());
}

void Interpreter::visit(const GuardStmt& stmt) {
    Value conditionValue = evaluate(*stmt.condition);
    
    if (!isTruthy(conditionValue)) {
        // Execute the else block (which must contain a return, throw, break, or continue)
        stmt.elseBody->accept(*this);
    }
    // If condition is true, continue execution normally
}

void Interpreter::visit(const GuardLetStmt& stmt) {
    Value expressionValue = evaluate(*stmt.expression);
    
    // Check if the value is not nil (optional binding succeeds)
    if (expressionValue.type == ValueType::Nil) {
        // Optional binding failed, execute else block
        stmt.elseBody->accept(*this);
    } else {
        // Bind the unwrapped value to the variable in the current environment
        environment->define(stmt.variable.lexeme, expressionValue, false, "Any");
        // Continue execution normally (guard let succeeded)
    }
}

void Interpreter::visit(const SwitchStmt& stmt) {
    Value switchValue = evaluate(*stmt.expression);
    bool matched = false;
    
    for (const auto& switchCase : stmt.cases) {
        if (switchCase.isDefault) {
            // Default case - execute if no other case matched
            if (!matched) {
                for (const auto& statement : switchCase.statements) {
                    statement->accept(*this);
                }
                matched = true;
            }
            break;
        } else {
            // Regular case - check if pattern matches
            Value caseValue = evaluate(*switchCase.pattern);
            
            // Simple equality check for now
            bool caseMatches = false;
            if (switchValue.type == caseValue.type) {
                switch (switchValue.type) {
                    case ValueType::Int:
                        caseMatches = std::get<int>(switchValue.value) == std::get<int>(caseValue.value);
                        break;
                    case ValueType::Double:
                        caseMatches = std::get<double>(switchValue.value) == std::get<double>(caseValue.value);
                        break;
                    case ValueType::String:
                        caseMatches = std::get<std::string>(switchValue.value) == std::get<std::string>(caseValue.value);
                        break;
                    case ValueType::Bool:
                        caseMatches = std::get<bool>(switchValue.value) == std::get<bool>(caseValue.value);
                        break;
                    default:
                        caseMatches = false;
                        break;
                }
            }
            
            if (caseMatches) {
                for (const auto& statement : switchCase.statements) {
                    statement->accept(*this);
                }
                matched = true;
                break; // Swift switch cases don't fall through by default
            }
        }
    }
    
    if (!matched) {
        throw std::runtime_error("Switch statement must be exhaustive or have a default case.");
    }
}

// Error handling expression implementations
void Interpreter::visit(const TryExpr& expr) {
    try {
        Value value = evaluate(*expr.expression);
        
        if (expr.isForced) {
            // try! - force unwrap, crash on error
            result = value;
        } else if (expr.isOptional) {
            // try? - convert errors to nil
            result = value;
        } else {
            // Normal try - propagate errors
            result = value;
        }
    } catch (const ThrowException& e) {
        if (expr.isForced) {
            // Crash for try!
            throw std::runtime_error("try! failed: " + e.error.getDescription());
        } else if (expr.isOptional) {
            // Convert to nil for try?
            result = Value("nil");
        } else {
            // Re-throw for normal try
            throw;
        }
    }
}

void Interpreter::visit(const ResultTypeExpr& expr) {
    // For now, treat Result type as a type identifier
    // In a full implementation, this would create a proper Result type
    result = Value("Result<SuccessType, ErrorType>");
}

void Interpreter::visit(const ErrorLiteral& expr) {
    // Create an error value as a string for now
    std::string errorStr = expr.errorType + ": " + expr.message;
    result = Value(errorStr);
}

// Error handling helper methods
void Interpreter::pushErrorContext(const ErrorContext& context) {
    errorContextStack.push(context);
}

void Interpreter::popErrorContext() {
    if (!errorContextStack.empty()) {
        errorContextStack.pop();
    }
}

ErrorContext& Interpreter::getCurrentErrorContext() {
    if (errorContextStack.empty()) {
        static ErrorContext defaultContext;
        return defaultContext;
    }
    return errorContextStack.top();
}

bool Interpreter::canCatchError(const ErrorValue& error) const {
    if (errorContextStack.empty()) {
        return false;
    }
    
    const auto& context = errorContextStack.top();
    return context.canCatch(error.getDescription());
}

void Interpreter::executeDeferredStatements() {
    if (!deferStack.empty()) {
        auto deferredStatements = std::move(deferStack.top());
        deferStack.pop();
        
        // Execute deferred statements in reverse order (LIFO)
        for (auto it = deferredStatements.rbegin(); it != deferredStatements.rend(); ++it) {
            try {
                (*it)->accept(*this);
            } catch (...) {
                // Ignore errors in deferred statements to prevent infinite loops
            }
        }
    }
}

void Interpreter::pushDeferredStatement(std::unique_ptr<Stmt> stmt) {
    if (deferStack.empty()) {
        // Create a defer stack level for global scope if needed
        deferStack.push(std::vector<std::unique_ptr<Stmt>>());
    }
    deferStack.top().push_back(std::move(stmt));
}

ValueResult Interpreter::wrapInResult(const Value& value, bool isSuccess, const ErrorValue* error) {
    if (isSuccess) {
        return ValueResult::success(value);
    } else if (error) {
        return ValueResult::failure(*error);
    } else {
        auto defaultError = std::make_unique<RuntimeError>("Unknown error");
        return ValueResult::failure(ErrorValue(std::move(defaultError)));
    }
}

// Type checking and casting implementations
void Interpreter::visit(const TypeCheck& expr) {
    Value value = evaluate(*expr.expression);
    std::string targetTypeName = expr.targetType.lexeme;
    
    // Check if the value is of the target type
    bool isOfType = false;
    
    // Handle basic type checking
    if (targetTypeName == "Int" && value.type == ValueType::Int) {
        isOfType = true;
    } else if (targetTypeName == "Double" && value.type == ValueType::Double) {
        isOfType = true;
    } else if (targetTypeName == "String" && value.type == ValueType::String) {
        isOfType = true;
    } else if (targetTypeName == "Bool" && value.type == ValueType::Bool) {
        isOfType = true;
    } else if (targetTypeName == "Array" && value.type == ValueType::Array) {
        isOfType = true;
    } else if (targetTypeName == "Dictionary" && value.type == ValueType::Dictionary) {
        isOfType = true;
    } else if (targetTypeName == "Optional" && value.type == ValueType::Optional) {
        isOfType = true;
    } else if (value.type == ValueType::Class) {
        // For class types, check class name
        const auto& classVal = value.asClass();
        isOfType = (classVal->className == targetTypeName);
    } else if (value.type == ValueType::Struct) {
        // For struct types, check struct name
        const auto& structVal = value.asStruct();
        isOfType = (structVal.structName == targetTypeName);
    } else if (value.type == ValueType::Enum) {
        // For enum types, check enum name
        const auto& enumVal = value.asEnum();
        isOfType = (enumVal->enumName == targetTypeName);
    }
    
    result = Value(isOfType);
}

void Interpreter::visit(const TypeCast& expr) {
    Value value = evaluate(*expr.expression);
    std::string targetTypeName = expr.targetType.lexeme;
    
    // Handle different cast types
    switch (expr.castType) {
        case TypeCast::CastType::Safe: {
            // Safe casting (as) - returns optional
            Value castResult = performTypeCast(value, targetTypeName, false);
            if (castResult.type != ValueType::Nil) {
                result = OptionalManager::createOptional(castResult);
            } else {
                result = Value(); // nil
            }
            break;
        }
        case TypeCast::CastType::Optional: {
            // Optional casting (as?) - returns optional
            Value castResult = performTypeCast(value, targetTypeName, false);
            if (castResult.type != ValueType::Nil) {
                result = OptionalManager::createOptional(castResult);
            } else {
                result = Value(); // nil
            }
            break;
        }
        case TypeCast::CastType::Forced: {
            // Forced casting (as!) - crashes on failure
            Value castResult = performTypeCast(value, targetTypeName, true);
            if (castResult.type == ValueType::Nil) {
                throw std::runtime_error("Forced cast failed: Cannot cast " + 
                                        valueTypeToString(value.type) + " to " + targetTypeName);
            }
            result = castResult;
            break;
        }
    }
}

Value Interpreter::performTypeCast(const Value& value, const std::string& targetType, bool isForced) {
    // Handle basic type conversions
    if (targetType == "String") {
        return Value(valueToString(value));
    }
    
    if (targetType == "Int") {
        switch (value.type) {
            case ValueType::Double:
                return Value(static_cast<int>(std::get<double>(value.value)));
            case ValueType::Float:
                return Value(static_cast<int>(std::get<float>(value.value)));
            case ValueType::String: {
                try {
                    return Value(std::stoi(std::get<std::string>(value.value)));
                } catch (...) {
                    return Value(); // nil
                }
            }
            case ValueType::Int:
                return value; // Already the right type
            default:
                return Value(); // nil
        }
    }
    
    if (targetType == "Double") {
        switch (value.type) {
            case ValueType::Int:
                return Value(static_cast<double>(std::get<int>(value.value)));
            case ValueType::Float:
                return Value(static_cast<double>(std::get<float>(value.value)));
            case ValueType::String: {
                try {
                    return Value(std::stod(std::get<std::string>(value.value)));
                } catch (...) {
                    return Value(); // nil
                }
            }
            case ValueType::Double:
                return value; // Already the right type
            default:
                return Value(); // nil
        }
    }
    
    // For same type, return the value as-is
    if ((targetType == "Int" && value.type == ValueType::Int) ||
        (targetType == "Double" && value.type == ValueType::Double) ||
        (targetType == "String" && value.type == ValueType::String) ||
        (targetType == "Bool" && value.type == ValueType::Bool)) {
        return value;
    }
    
    // For class/struct/enum types, check if they match
    if (value.type == ValueType::Class) {
        const auto& classVal = value.asClass();
        if (classVal->className == targetType) {
            return value;
        }
    } else if (value.type == ValueType::Struct) {
        const auto& structVal = value.asStruct();
        if (structVal.structName == targetType) {
            return value;
        }
    } else if (value.type == ValueType::Enum) {
        const auto& enumVal = value.asEnum();
        if (enumVal->enumName == targetType) {
            return value;
        }
    }
    
    // Suppress unused parameter warning
    (void)isForced;
    
    // Cast failed
    return Value(); // nil
}

std::string Interpreter::valueTypeToString(ValueType type) {
    switch (type) {
        case ValueType::Int: return "Int";
        case ValueType::Double: return "Double";
        case ValueType::String: return "String";
        case ValueType::Bool: return "Bool";
        case ValueType::Array: return "Array";
        case ValueType::Dictionary: return "Dictionary";
        case ValueType::Optional: return "Optional";
        case ValueType::Class: return "Class";
        case ValueType::Struct: return "Struct";
        case ValueType::Enum: return "Enum";
        case ValueType::Nil: return "Nil";
        default: return "Unknown";
    }
}

// Advanced statement implementations
void Interpreter::visit(const CustomOperatorStmt& stmt) {
    // Custom operator declarations are handled at compile time
    // For now, we just store the operator information
    // In a full implementation, this would register the operator with the parser
    std::cout << "Custom operator declared: " << stmt.operatorType.lexeme 
              << " " << stmt.operatorSymbol.lexeme << std::endl;
}

void Interpreter::visit(const OperatorPrecedenceStmt& stmt) {
    // Operator precedence declarations are handled at compile time
    // For now, we just print the precedence information
    std::cout << "Precedence group declared: " << stmt.precedenceGroupName.lexeme 
              << " (associativity: " << stmt.associativity.lexeme 
              << ", precedence: " << stmt.precedenceLevel;
    
    if (!stmt.higherThan.empty()) {
        std::cout << ", higherThan: ";
        for (size_t i = 0; i < stmt.higherThan.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << stmt.higherThan[i].lexeme;
        }
    }
    
    if (!stmt.lowerThan.empty()) {
        std::cout << ", lowerThan: ";
        for (size_t i = 0; i < stmt.lowerThan.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << stmt.lowerThan[i].lexeme;
        }
    }
    
    std::cout << ")" << std::endl;
}

void Interpreter::visit(const ResultBuilderStmt& stmt) {
    // Result builder declarations are handled at compile time
    // For now, we just print the builder information
    std::cout << "Result builder declared: " << stmt.name.lexeme << std::endl;
    
    // In a full implementation, this would register the result builder
    // and its transformation methods
}

// Concurrency statement implementation
void Interpreter::visit(const ActorStmt& stmt) {
    // Actor declarations are handled at compile time
    // For now, we just print the actor information
    std::cout << "Actor declared: " << stmt.name.lexeme << std::endl;
    
    // In a full implementation, this would:
    // 1. Create an actor type with isolated state
    // 2. Register actor methods with proper isolation
    // 3. Set up message passing mechanisms
    // 4. Handle actor initialization and lifecycle
}

// Concurrency expression implementations
void Interpreter::visit(const AwaitExpr& expr) {
    // For now, we simulate async behavior by evaluating the expression synchronously
    // In a full implementation, this would:
    // 1. Check if we're in an async context
    // 2. Suspend the current task
    // 3. Wait for the async operation to complete
    // 4. Resume with the result
    
    std::cout << "Awaiting expression..." << std::endl;
    result = evaluate(*expr.expression);
    std::cout << "Await completed." << std::endl;
}

void Interpreter::visit(const TaskExpr& expr) {
    // For now, we simulate task creation by executing the closure immediately
    // In a full implementation, this would:
    // 1. Create a new task with the given closure
    // 2. Schedule the task for execution
    // 3. Return a task handle that can be awaited
    
    std::cout << "Creating task..." << std::endl;
    
    // Create new environment for task execution
    auto previous = environment;
    environment = std::make_shared<Environment>(environment);
    
    try {
        // Cast the closure expression to Closure type
        const Closure* closure = dynamic_cast<const Closure*>(expr.closure.get());
        if (!closure) {
            throw std::runtime_error("Task closure is not a valid closure expression");
        }
        
        // Execute the task closure
        for (const auto& statement : closure->body) {
            statement->accept(*this);
        }
        
        // For now, return the last result
        // In a full implementation, this would return a Task<T> type
    } catch (const ReturnException& returnValue) {
        result = *returnValue.value;
    }
    
    // Restore previous environment
    environment = previous;
    
    std::cout << "Task created and executed." << std::endl;
}

void Interpreter::visit(const TaskGroupExpr& expr) {
    // For now, we simulate task group by executing the closure immediately
    // In a full implementation, this would:
    // 1. Create a task group for managing child tasks
    // 2. Execute the closure with the task group as parameter
    // 3. Wait for all child tasks to complete
    
    std::cout << "Creating task group..." << std::endl;
    
    // Create new environment for task group execution
    auto previous = environment;
    environment = std::make_shared<Environment>(environment);
    
    try {
        // Cast the closure expression to Closure type
        const Closure* closure = dynamic_cast<const Closure*>(expr.closure.get());
        if (!closure) {
            throw std::runtime_error("Task group closure is not a valid closure expression");
        }
        
        // Execute the task group closure
        for (const auto& statement : closure->body) {
            statement->accept(*this);
        }
        
    } catch (const ReturnException& returnValue) {
        result = *returnValue.value;
    }
    
    // Restore previous environment
    environment = previous;
    
    std::cout << "Task group completed." << std::endl;
}

void Interpreter::visit(const AsyncSequenceExpr& expr) {
    // For now, we simulate async sequence by evaluating the body expression
    // In a full implementation, this would:
    // 1. Create an async sequence that yields values over time
    // 2. Handle async iteration protocol
    // 3. Support cancellation and backpressure
    
    std::cout << "Creating async sequence..." << std::endl;
    
    // For now, just evaluate the body and return it as a regular value
    result = evaluate(*expr.body);
    
    std::cout << "Async sequence created." << std::endl;
}

void Interpreter::visit(const AsyncLetExpr& expr) {
    // For now, we simulate async let by evaluating the expression immediately
    // In a full implementation, this would:
    // 1. Start the async operation concurrently
    // 2. Allow other code to run while waiting
    // 3. Provide the result when awaited
    
    std::cout << "Creating async let binding..." << std::endl;
    
    // Evaluate the async expression
    Value asyncValue = evaluate(*expr.initializer);
    
    // Define the variable in current environment
    environment->define(expr.variable.lexeme, asyncValue, false, "Any");
    
    // Return the value
    result = asyncValue;
    
    std::cout << "Async let binding created." << std::endl;
}

// Opaque and Boxed Protocol Types implementations
void Interpreter::visit(const OpaqueTypeExpr& expr) {
    // For opaque types (some Protocol), we create a special type wrapper
    // that hides the concrete type while preserving protocol conformance
    
    std::string protocolName = expr.protocolName.lexeme;
    
    // In a full implementation, this would:
    // 1. Verify that the protocol exists
    // 2. Create an opaque type wrapper that enforces protocol conformance
    // 3. Hide the concrete type information from the caller
    
    // For now, we create a placeholder value that represents the opaque type
    result = Value("<opaque type conforming to " + protocolName + ">");
    
    std::cout << "Created opaque type for protocol: " << protocolName << std::endl;
}

void Interpreter::visit(const BoxedProtocolTypeExpr& expr) {
    // For boxed protocol types (any Protocol), we create a type-erased wrapper
    // that can hold any type conforming to the protocol
    
    std::string protocolName = expr.protocolName.lexeme;
    
    // In a full implementation, this would:
    // 1. Verify that the protocol exists
    // 2. Create a boxed protocol type that can hold any conforming type
    // 3. Provide dynamic dispatch for protocol methods
    // 4. Allow runtime type checking and casting
    
    // For now, we create a placeholder value that represents the boxed protocol type
    result = Value("<boxed protocol type: " + protocolName + ">");
    
    std::cout << "Created boxed protocol type: " << protocolName << std::endl;
}

// Macro statement implementations
void Interpreter::visit(const MacroStmt& stmt) {
    // Store macro definition in environment
    // For now, we'll store it as a placeholder value
    environment->define(stmt.name.lexeme, Value("<macro:" + stmt.name.lexeme + ">"), false, "Macro");
    std::cout << "Macro declared: " << stmt.name.lexeme << std::endl;
}

void Interpreter::visit(const ExternalMacroStmt& stmt) {
    // Store external macro definition in environment
    // External macros are placeholders that would be resolved at compile time
    environment->define(stmt.name.lexeme, Value("<external_macro:" + stmt.name.lexeme + ">"), false, "ExternalMacro");
    std::cout << "External macro declared: " << stmt.name.lexeme 
              << " (module: " << stmt.moduleName << ", type: " << stmt.typeName << ")" << std::endl;
}

void Interpreter::visit(const FreestandingMacroStmt& stmt) {
    // Store freestanding macro definition in environment
    environment->define(stmt.name.lexeme, Value("<freestanding_macro:" + stmt.name.lexeme + ">"), false, "FreestandingMacro");
    std::cout << "Freestanding macro declared: " << stmt.name.lexeme 
              << " (role: " << stmt.role.lexeme << ")" << std::endl;
}

void Interpreter::visit(const AttachedMacroStmt& stmt) {
    // Store attached macro definition in environment
    environment->define(stmt.name.lexeme, Value("<attached_macro:" + stmt.name.lexeme + ">"), false, "AttachedMacro");
    std::cout << "Attached macro declared: " << stmt.name.lexeme 
              << " (role: " << stmt.attachmentKind.lexeme << ")" << std::endl;
}

// Macro expression implementations
void Interpreter::visit(const MacroExpansionExpr& expr) {
    // For now, macro expansion will just return a placeholder value
    // In a real implementation, this would involve:
    // 1. Looking up the macro definition
    // 2. Expanding the macro with the given arguments
    // 3. Evaluating the expanded code
    
    std::cout << "Expanding macro: " << expr.macroName.lexeme << std::endl;
    
    // Simple placeholder implementation
    result = Value("<macro_expansion:" + expr.macroName.lexeme + ">");
}

void Interpreter::visit(const FreestandingMacroExpr& expr) {
    // Freestanding macro expressions are expanded at compile time
    // For interpreter mode, we'll provide a simple placeholder
    std::cout << "Expanding freestanding macro: " << expr.macroName.lexeme << std::endl;
    result = Value("<freestanding_macro:" + expr.macroName.lexeme + ">");
}

void Interpreter::visit(const AttachedMacroExpr& expr) {
    // Attached macro expressions modify the declaration they're attached to
    // For interpreter mode, we'll provide a simple placeholder
    std::cout << "Expanding attached macro: " << expr.macroName.lexeme << std::endl;
    result = Value("<attached_macro:" + expr.macroName.lexeme + ">");
}

} // namespace miniswift