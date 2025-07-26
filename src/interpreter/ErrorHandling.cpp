#include "ErrorHandling.h"
#include "Value.h"

namespace miniswift {

// Implementation of ReturnException convenience constructor
ReturnException::ReturnException(const Value& val)
    : std::runtime_error("Return statement executed"),
      value(std::make_shared<Value>(val)) {}

} // namespace miniswift