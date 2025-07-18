#include "Interpreter.h"
#include <stdexcept>
#include <iostream>

namespace miniswift {

Interpreter::Interpreter() {
    environment = std::make_shared<Environment>();
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>>& statements) {
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
    environment->assign(expr.name, value);
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
    Array elements;
    for (const auto& element : expr.elements) {
        elements.push_back(evaluate(*element));
    }
    result = Value(elements);
}

void Interpreter::visit(const DictionaryLiteral& expr) {
    Dictionary dict;
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
        const Array& arr = object.asArray();
        
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
        
        const Dictionary& dict = object.asDictionary();
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
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i > 0) std::cout << ", ";
        printValue(arr[i]);
    }
    std::cout << "]" << std::endl;
}

void Interpreter::printDictionary(const Dictionary& dict) {
    std::cout << "{";
    bool first = true;
    for (const auto& pair : dict) {
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

} // namespace miniswift