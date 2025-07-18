#ifndef MINISWIFT_VALUE_H
#define MINISWIFT_VALUE_H

#include <string>
#include <variant>

namespace miniswift {

enum class ValueType {
    Nil,
    Bool,
    Int,
    Double,
    String
};

struct Value {
    ValueType type;
    std::variant<std::monostate, bool, int, double, std::string> value;

    Value() : type(ValueType::Nil), value(std::monostate{}) {}
    Value(bool v) : type(ValueType::Bool), value(v) {}
    Value(int v) : type(ValueType::Int), value(v) {}
    Value(double v) : type(ValueType::Double), value(v) {}
    Value(std::string v) : type(ValueType::String), value(v) {}
};

} // namespace miniswift

#endif // MINISWIFT_VALUE_H