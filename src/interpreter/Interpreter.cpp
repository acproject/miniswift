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
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
    std::cout << "Starting interpretation with " << statements.size() << " statements" << std::endl;
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
    }
}

void Interpreter::visit(const VarStmt& stmt) {
    Value value; // Default-initialized (nil)
    if (stmt.initializer) {
        value = evaluate(*stmt.initializer);
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
    } else {
        throw std::runtime_error("Invalid assignment target");
    }
    
    result = value;
}

void Interpreter::visit(const Binary& expr) {
    Value left = evaluate(*expr.left);
    Value right = evaluate(*expr.right);

    if ((left.type == ValueType::Int || left.type == ValueType::Double) && (right.type == ValueType::Int || right.type == ValueType::Double)) {
        // Numeric operations
    } else if (left.type == ValueType::String && right.type == ValueType::String) {
        // String concatenation
    } else {
        throw std::runtime_error("Operands must be two numbers or two strings.");
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
        case TokenType::Greater: {
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
        case TokenType::Less: {
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
    } else {
        throw std::runtime_error("Only arrays and dictionaries can be indexed.");
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
    // Register struct properties
    registerStructProperties(stmt.name.lexeme, stmt.members);
    
    // Debug: Check if property manager was created
    auto* propManager = getStructPropertyManager(stmt.name.lexeme);
    if (propManager) {
        std::cout << "Property manager created for struct: " << stmt.name.lexeme << std::endl;
    } else {
        std::cout << "Failed to create property manager for struct: " << stmt.name.lexeme << std::endl;
    }
    
    // Store struct definition in environment for later use
    environment->define(stmt.name.lexeme, Value(), false, "Struct");
}

// Execute class declaration: class Name { members }
void Interpreter::visit(const ClassStmt& stmt) {
    // Register class properties
    registerClassProperties(stmt.name.lexeme, stmt.members);
    
    // Store class definition in environment for later use
    environment->define(stmt.name.lexeme, Value(), false, "Class");
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

// Execute member access: object.member
void Interpreter::visit(const MemberAccess& expr) {
    Value object = evaluate(*expr.object);
    result = getMemberValue(object, expr.member.lexeme);
}

// Execute struct initialization: StructName(member1: value1, member2: value2)
void Interpreter::visit(const StructInit& expr) {
    // Get property manager for this struct
    auto* propManager = getStructPropertyManager(expr.structName.lexeme);
    
    // Debug: Check property manager status
    std::cout << "StructInit for: " << expr.structName.lexeme << std::endl;
    if (propManager) {
        std::cout << "Property manager found, properties count: " << propManager->getAllProperties().size() << std::endl;
    } else {
        std::cout << "No property manager found" << std::endl;
    }
    
    // Create struct with property system if available
    
    if (propManager) {
        // Create struct with property support
        auto propContainer = std::make_unique<InstancePropertyContainer>(*propManager, environment);
        
        // Initialize provided members
        for (const auto& member : expr.members) {
            Value memberValue = evaluate(*member.second);
            propContainer->setProperty(*this, member.first.lexeme, memberValue);
        }
        
        // Initialize default values for unspecified properties
        propContainer->initializeDefaults(*this);
        
        StructValue structValue(expr.structName.lexeme, std::move(propContainer));
        result = Value(structValue);
    } else {
        // Fallback to legacy struct creation
        StructValue structValue(expr.structName.lexeme);
        
        // Initialize members with provided values
        for (const auto& member : expr.members) {
            Value memberValue = evaluate(*member.second);
            (*structValue.members)[member.first.lexeme] = memberValue;
        }
        
        result = Value(structValue);
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

} // namespace miniswift