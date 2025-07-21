#include "Interpreter.h"
#include "OOP/Property.h"
#include "OOP/Constructor.h"
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
    Value val = evaluate(*stmt.expression);
    switch (val.type) {
        case ValueType::Int:
            std::cout << std::get<int>(val.value) << std::endl;
            break;
        case ValueType::Double:
            std::cout << std::get<double>(val.value) << std::endl;
            break;
        case ValueType::Bool:
            std::cout << (std::get<bool>(val.value) ? "true" : "false") << std::endl;
            break;
        case ValueType::String:
            std::cout << std::get<std::string>(val.value) << std::endl;
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
            std::cout << "nil" << std::endl;
            break;
        case ValueType::Function:
            if (val.isClosure()) {
                std::cout << "<closure>" << std::endl;
            } else {
                std::cout << "<function>" << std::endl;
            }
            break;
        case ValueType::Enum: {
            const auto& enumVal = val.asEnum();
            std::cout << enumVal.enumName << "." << enumVal.caseName;
            if (!enumVal.associatedValues.empty()) {
                std::cout << "(";
                for (size_t i = 0; i < enumVal.associatedValues.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    printValue(enumVal.associatedValues[i]);
                }
                std::cout << ")";
            }
            std::cout << std::endl;
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
            std::cout << ")" << std::endl;
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
            std::cout << ")" << std::endl;
            break;
        }
        case ValueType::Constructor:
            std::cout << "<constructor>" << std::endl;
            break;
        case ValueType::Destructor:
            std::cout << "<destructor>" << std::endl;
            break;
        case ValueType::Optional: {
            const auto& optionalVal = val.asOptional();
            if (optionalVal.hasValue && optionalVal.wrappedValue) {
                std::cout << "Optional(";
                printValue(*optionalVal.wrappedValue);
                std::cout << ")" << std::endl;
            } else {
                std::cout << "nil" << std::endl;
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
                printValue(resultPtr->getValue());
                std::cout << ")";
            } else {
                std::cout << "Result.failure(" << resultPtr->getError().getDescription() << ")";
            }
            break;
        }
    }
}

void Interpreter::visit(const VarStmt& stmt) {
    Value value; // Default-initialized (nil)
    if (stmt.initializer) {
        value = evaluate(*stmt.initializer);
        
        // Check if the variable type is optional and wrap the value if needed
        if (stmt.type.lexeme.back() == '?' && value.type != ValueType::Optional) {
            value = OptionalManager::createOptional(value);
        }
    }
    environment->define(stmt.name.lexeme, value, stmt.isConst, stmt.type.lexeme);
}

void Interpreter::visit(const VarExpr& expr) {
    result = environment->get(expr.name);
}

void Interpreter::visit(const Assign& expr) {
    Value value = evaluate(*expr.value);
    
    // Check if target is a simple variable or member access
    if (auto varExpr = dynamic_cast<const VarExpr*>(expr.target.get())) {
        // Simple variable assignment
        environment->assign(varExpr->name, value);
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

    // Type checking for non-logical operators
    if (expr.op.type != TokenType::BangEqual && expr.op.type != TokenType::EqualEqual) {
        if ((left.type == ValueType::Int || left.type == ValueType::Double) && (right.type == ValueType::Int || right.type == ValueType::Double)) {
            // Numeric operations
        } else if (left.type == ValueType::String && right.type == ValueType::String && expr.op.type == TokenType::Plus) {
            // String concatenation
        } else {
            throw std::runtime_error("Operands must be two numbers or two strings.");
        }
    }

    switch (expr.op.type) {
        case TokenType::Minus: {
            double leftVal = (left.type == ValueType::Int) ? std::get<int>(left.value) : std::get<double>(left.value);
            double rightVal = (right.type == ValueType::Int) ? std::get<int>(right.value) : std::get<double>(right.value);
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
                double leftVal = (left.type == ValueType::Int) ? std::get<int>(left.value) : std::get<double>(left.value);
                double rightVal = (right.type == ValueType::Int) ? std::get<int>(right.value) : std::get<double>(right.value);
                if (left.type == ValueType::Int && right.type == ValueType::Int) {
                    result = Value(static_cast<int>(leftVal + rightVal));
                } else {
                    result = Value(leftVal + rightVal);
                }
            }
            return;
        case TokenType::Slash: {
            double leftVal = (left.type == ValueType::Int) ? std::get<int>(left.value) : std::get<double>(left.value);
            double rightVal = (right.type == ValueType::Int) ? std::get<int>(right.value) : std::get<double>(right.value);
            result = Value(leftVal / rightVal);
            return;
        }
        case TokenType::Star: {
            double leftVal = (left.type == ValueType::Int) ? std::get<int>(left.value) : std::get<double>(left.value);
            double rightVal = (right.type == ValueType::Int) ? std::get<int>(right.value) : std::get<double>(right.value);
            if (left.type == ValueType::Int && right.type == ValueType::Int) {
                result = Value(static_cast<int>(leftVal * rightVal));
            } else {
                result = Value(leftVal * rightVal);
            }
            return;
        }
        case TokenType::RAngle: {
            double leftVal = (left.type == ValueType::Int) ? std::get<int>(left.value) : std::get<double>(left.value);
            double rightVal = (right.type == ValueType::Int) ? std::get<int>(right.value) : std::get<double>(right.value);
            result = Value(leftVal > rightVal);
            return;
        }
        case TokenType::GreaterEqual: {
            double leftVal = (left.type == ValueType::Int) ? std::get<int>(left.value) : std::get<double>(left.value);
            double rightVal = (right.type == ValueType::Int) ? std::get<int>(right.value) : std::get<double>(right.value);
            result = Value(leftVal >= rightVal);
            return;
        }
        case TokenType::LAngle: {
            double leftVal = (left.type == ValueType::Int) ? std::get<int>(left.value) : std::get<double>(left.value);
            double rightVal = (right.type == ValueType::Int) ? std::get<int>(right.value) : std::get<double>(right.value);
            result = Value(leftVal < rightVal);
            return;
        }
        case TokenType::LessEqual: {
            double leftVal = (left.type == ValueType::Int) ? std::get<int>(left.value) : std::get<double>(left.value);
            double rightVal = (right.type == ValueType::Int) ? std::get<int>(right.value) : std::get<double>(right.value);
            result = Value(leftVal <= rightVal);
            return;
        }
        case TokenType::BangEqual:
            result = Value(left.value != right.value);
            return;
        case TokenType::EqualEqual:
            result = Value(left.value == right.value);
            return;
        default: break; // Should not be reached
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
                default:
                    result_str += "<object>";
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
        if (index.type != ValueType::Int) {
            throw std::runtime_error("Array index must be an integer.");
        }
        
        int idx = std::get<int>(index.value);
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

void Interpreter::printTuple(const Tuple& tuple) {
    std::cout << "(";
    for (size_t i = 0; i < tuple.elements->size(); ++i) {
        if (i > 0) std::cout << ", ";
        printValue((*tuple.elements)[i]);
    }
    std::cout << ")" << std::endl;
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
        case ValueType::Array:
            return "<array>";
        case ValueType::Dictionary:
            return "<dictionary>";
        case ValueType::Tuple:
            return "<tuple>";
        case ValueType::Function:
            return val.isClosure() ? "<closure>" : "<function>";
        case ValueType::Enum: {
            const auto& enumVal = val.asEnum();
            return enumVal.enumName + "." + enumVal.caseName;
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
            std::cout << enumVal.enumName << "." << enumVal.caseName;
            if (!enumVal.associatedValues.empty()) {
                std::cout << "(";
                for (size_t i = 0; i < enumVal.associatedValues.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    printValue(enumVal.associatedValues[i]);
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
            const auto& dict = *collection.asDictionary();
            
            if (stmt.variables.size() == 2) {
                // Dictionary iteration: for (key, value) in dictionary
                for (const auto& pair : dict) {
                    environment->define(stmt.variables[0].lexeme, Value(pair.first), false, "String");
                    environment->define(stmt.variables[1].lexeme, pair.second, false, "Any");
                    stmt.body->accept(*this);
                }
            } else {
                throw std::runtime_error("Dictionary iteration requires exactly 2 variables");
            }
        } else {
            throw std::runtime_error("Can only iterate over arrays and dictionaries");
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

// Execute function call: callee(arguments)
void Interpreter::visit(const Call& expr) {
    // Check if callee is MemberAccess
    if (auto memberAccess = dynamic_cast<const MemberAccess*>(expr.callee.get())) {
        
        // Handle method calls on objects (including extension methods)
        Value object = evaluate(*memberAccess->object);
        Value method = getMemberValue(object, memberAccess->member.lexeme);
        
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
                if (arguments.size() != callable->functionDecl->parameters.size()) {
                    throw std::runtime_error("Expected " + 
                        std::to_string(callable->functionDecl->parameters.size()) + 
                        " arguments but got " + std::to_string(arguments.size()) + ".");
                }
                
                // Create new environment for function execution
                auto previous = environment;
                environment = std::make_shared<Environment>(callable->closure);
                
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
                    
                    // If no return statement was executed, return nil
                    result = Value();
                } catch (const ReturnException& returnValue) {
                    // Function returned a value
                    result = returnValue.value;
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
        // Handle function call
        if (arguments.size() != callable->functionDecl->parameters.size()) {
            throw std::runtime_error("Expected " + 
                std::to_string(callable->functionDecl->parameters.size()) + 
                " arguments but got " + std::to_string(arguments.size()) + ".");
        }
        
        // Create new environment for function execution
        auto previous = environment;
        environment = std::make_shared<Environment>(callable->closure);
        
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
            
            // If no return statement was executed, return nil
            result = Value();
        } catch (const ReturnException& returnValue) {
            // Function returned a value
            result = returnValue.value;
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
            
            // If no return statement was executed, return nil
            result = Value();
        } catch (const ReturnException& returnValue) {
            // Closure returned a value
            result = returnValue.value;
        }
        
        // Restore previous environment
        environment = previous;
    }
}

// Execute closure expression: { (parameters) -> ReturnType in body }
void Interpreter::visit(const Closure& expr) {
    auto closure = std::make_shared<Function>(&expr, environment);
    result = Value(closure);
}

// Execute enum declaration: enum Name: RawType { cases }
void Interpreter::visit(const EnumStmt& stmt) {
    // Store enum definition in environment for later use
    // For now, we'll store the enum name as a special marker
    environment->define(stmt.name.lexeme, Value(), false, "Enum");
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
    result = Value(enumValue);
}

// Execute struct declaration: struct Name { members }
void Interpreter::visit(const StructStmt& stmt) {
    // Set current type context for subscript registration
    environment->define("__current_type__", Value(stmt.name.lexeme), false, "String");
    
    // Register struct properties
    registerStructProperties(stmt.name.lexeme, stmt.members);
    
    // Process subscripts
    for (const auto& subscript : stmt.subscripts) {
        subscript->accept(*this);
    }
    
    // Debug: Check if property manager was created
    auto* propManager = getStructPropertyManager(stmt.name.lexeme);
    if (propManager) {
        // std::cout << "Property manager created for struct: " << stmt.name.lexeme << std::endl;
    } else {
        std::cout << "Failed to create property manager for struct: " << stmt.name.lexeme << std::endl;
    }
    
    // Store struct definition in environment for later use
    environment->define(stmt.name.lexeme, Value(), false, "Struct");
    
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
    
    // Register class properties
    registerClassProperties(stmt.name.lexeme, stmt.members);
    
    // Process subscripts
    for (const auto& subscript : stmt.subscripts) {
        subscript->accept(*this);
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
        
        // Create a special callable that creates class instances
        auto callable = std::make_shared<Function>(constructorFunc.get(), environment);
        
        // Store the constructor in the environment with the class name
        environment->define(stmt.name.lexeme, Value(callable), false, "Constructor");
        
        std::cout << "Created default constructor for class: " << stmt.name.lexeme << std::endl;
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
    Value object = evaluate(*expr.object);
    result = getMemberValue(object, expr.member.lexeme);
}

// Execute struct initialization: StructName(member1: value1, member2: value2)
void Interpreter::visit(const StructInit& expr) {
    // Check if this is a class or struct
    auto* structPropManager = getStructPropertyManager(expr.structName.lexeme);
    auto* classPropManager = getClassPropertyManager(expr.structName.lexeme);
    

    
    if (classPropManager) {
        // Create class instance

        
        auto propContainer = std::make_unique<InstancePropertyContainer>(*classPropManager, environment);
        
        // Initialize provided members
        for (const auto& member : expr.members) {
            Value memberValue = evaluate(*member.second);

            propContainer->setProperty(*this, member.first.lexeme, memberValue);
        }
        
        // Initialize default values for unspecified properties
        propContainer->initializeDefaults(*this);
        
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
        env->define("return", returnValue.value);
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
        } catch (const std::runtime_error&) {
            // Extension method not found, continue to error
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

// Execute super expression: super.method() or super.property
void Interpreter::visit(const Super& expr) {
    // Get current class context from environment
    // This should be set when we're inside a class method
    std::string currentClass;
    try {
        Value classContext = environment->get(Token(TokenType::Identifier, "__current_class__", 0));
        if (classContext.type == ValueType::String) {
            currentClass = std::get<std::string>(classContext.value);
        } else {
            throw std::runtime_error("super can only be used within class methods");
        }
    } catch (const std::runtime_error&) {
        throw std::runtime_error("super can only be used within class methods");
    }
    
    // For now, we'll handle super.method as a property access
    // In a full implementation, this would need to handle method calls differently
    try {
        result = superHandler->getSuperProperty(currentClass, expr.method.lexeme, environment);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Failed to access super property '" + expr.method.lexeme + "': " + e.what());
    }
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

} // namespace miniswift