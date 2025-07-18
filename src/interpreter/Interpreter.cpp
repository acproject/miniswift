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
        case TokenType::Minus:
            result = Value(std::get<double>(left.value) - std::get<double>(right.value));
            return;
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
        case TokenType::Slash:
            result = Value(std::get<double>(left.value) / std::get<double>(right.value));
            return;
        case TokenType::Star:
            result = Value(std::get<double>(left.value) * std::get<double>(right.value));
            return;
        case TokenType::Greater:
            result = Value(std::get<double>(left.value) > std::get<double>(right.value));
            return;
        case TokenType::GreaterEqual:
            result = Value(std::get<double>(left.value) >= std::get<double>(right.value));
            return;
        case TokenType::Less:
            result = Value(std::get<double>(left.value) < std::get<double>(right.value));
            return;
        case TokenType::LessEqual:
            result = Value(std::get<double>(left.value) <= std::get<double>(right.value));
            return;
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
        case TokenType::Minus:
            result = Value(-std::get<double>(right.value));
            return;
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

} // namespace miniswift