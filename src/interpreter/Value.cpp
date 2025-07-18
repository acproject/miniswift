#include "Value.h"

namespace miniswift {

// StructValue comparison operators
bool StructValue::operator==(const StructValue& other) const {
    return structName == other.structName && *members == *other.members;
}

bool StructValue::operator!=(const StructValue& other) const {
    return !(*this == other);
}

// ClassValue comparison operators
bool ClassValue::operator==(const ClassValue& other) const {
    return className == other.className && *members == *other.members;
}

bool ClassValue::operator!=(const ClassValue& other) const {
    return !(*this == other);
}

} // namespace miniswift