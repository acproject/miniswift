#ifndef MINISWIFT_ERROR_HANDLING_H
#define MINISWIFT_ERROR_HANDLING_H

#include "../lexer/Token.h"
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace miniswift {

// Forward declarations
struct Value;
class Expr;
class Stmt;
class Environment;

// Error protocol - base interface for all error types
struct ErrorProtocol {
  virtual ~ErrorProtocol() = default;
  virtual std::string getDescription() const = 0;
  virtual std::string getLocalizedDescription() const {
    return getDescription();
  }
  virtual std::unique_ptr<ErrorProtocol> clone() const = 0;
};

// Built-in error types
struct RuntimeError : public ErrorProtocol {
  std::string message;
  int line;
  int column;

  RuntimeError(const std::string &msg, int l = -1, int c = -1)
      : message(msg), line(l), column(c) {}

  std::string getDescription() const override {
    if (line >= 0) {
      return "Runtime Error at line " + std::to_string(line) + ": " + message;
    }
    return "Runtime Error: " + message;
  }

  std::unique_ptr<ErrorProtocol> clone() const override {
    return std::make_unique<RuntimeError>(message, line, column);
  }
};

struct TypeError : public ErrorProtocol {
  std::string expectedType;
  std::string actualType;
  std::string context;

  TypeError(const std::string &expected, const std::string &actual,
            const std::string &ctx = "")
      : expectedType(expected), actualType(actual), context(ctx) {}

  std::string getDescription() const override {
    std::string desc =
        "Type Error: Expected " + expectedType + ", got " + actualType;
    if (!context.empty()) {
      desc += " in " + context;
    }
    return desc;
  }

  std::unique_ptr<ErrorProtocol> clone() const override {
    return std::make_unique<TypeError>(expectedType, actualType, context);
  }
};

struct IndexOutOfBoundsError : public ErrorProtocol {
  int index;
  int size;

  IndexOutOfBoundsError(int idx, int sz) : index(idx), size(sz) {}

  std::string getDescription() const override {
    return "Index Out of Bounds: Index " + std::to_string(index) +
           " is out of range for collection of size " + std::to_string(size);
  }

  std::unique_ptr<ErrorProtocol> clone() const override {
    return std::make_unique<IndexOutOfBoundsError>(index, size);
  }
};

struct KeyNotFoundError : public ErrorProtocol {
  std::string key;

  KeyNotFoundError(const std::string &k) : key(k) {}

  std::string getDescription() const override {
    return "Key Not Found: Key '" + key + "' not found in dictionary";
  }

  std::unique_ptr<ErrorProtocol> clone() const override {
    return std::make_unique<KeyNotFoundError>(key);
  }
};

// Error value type for runtime representation
struct ErrorValue {
  std::unique_ptr<ErrorProtocol> error;

  ErrorValue(std::unique_ptr<ErrorProtocol> err) : error(std::move(err)) {}

  // Copy constructor
  ErrorValue(const ErrorValue &other) : error(other.error->clone()) {}

  // Move constructor
  ErrorValue(ErrorValue &&other) noexcept : error(std::move(other.error)) {}

  // Copy assignment
  ErrorValue &operator=(const ErrorValue &other) {
    if (this != &other) {
      error = other.error->clone();
    }
    return *this;
  }

  // Move assignment
  ErrorValue &operator=(ErrorValue &&other) noexcept {
    if (this != &other) {
      error = std::move(other.error);
    }
    return *this;
  }

  std::string getDescription() const {
    return error ? error->getDescription() : "Unknown Error";
  }

  bool operator==(const ErrorValue &other) const {
    // Simple comparison based on description
    return getDescription() == other.getDescription();
  }

  bool operator!=(const ErrorValue &other) const { return !(*this == other); }
};

// Result type for error handling
template <typename T> struct Result {
  bool isSuccess;
  std::variant<T, ErrorValue> value;

  // Success constructor
  static Result<T> success(const T &val) {
    Result<T> result;
    result.isSuccess = true;
    result.value = val;
    return result;
  }

  // Failure constructor
  static Result<T> failure(ErrorValue err) {
    Result<T> result;
    result.isSuccess = false;
    result.value = std::move(err);
    return result;
  }

  // Convenience failure constructor
  static Result<T> failure(std::unique_ptr<ErrorProtocol> err) {
    return failure(ErrorValue(std::move(err)));
  }

  // Get success value (throws if failure)
  const T &getValue() const {
    if (!isSuccess) {
      throw std::runtime_error("Attempted to get value from failed Result");
    }
    return std::get<T>(value);
  }

  // Get error value (throws if success)
  const ErrorValue &getError() const {
    if (isSuccess) {
      throw std::runtime_error("Attempted to get error from successful Result");
    }
    return std::get<ErrorValue>(value);
  }

  // Check if result is success
  bool isOk() const { return isSuccess; }

  // Check if result is failure
  bool isErr() const { return !isSuccess; }

  // Comparison operators
  bool operator==(const Result<T> &other) const {
    if (isSuccess != other.isSuccess)
      return false;
    if (isSuccess) {
      return std::get<T>(value) == std::get<T>(other.value);
    } else {
      return std::get<ErrorValue>(value) == std::get<ErrorValue>(other.value);
    }
  }

  bool operator!=(const Result<T> &other) const { return !(*this == other); }
};

// Specialized Result for Value type
using ValueResult = Result<Value>;

// Exception class for throw statements
class ThrowException : public std::runtime_error {
public:
  ErrorValue error;

  ThrowException(ErrorValue err)
      : std::runtime_error("Thrown error: " + err.getDescription()),
        error(std::move(err)) {}

  ThrowException(std::unique_ptr<ErrorProtocol> err)
      : ThrowException(ErrorValue(std::move(err))) {}
};

// Error handling context for tracking try-catch blocks
struct ErrorContext {
  bool inTryBlock = false;
  bool inCatchBlock = false;
  std::vector<std::string> catchableErrorTypes;

  void enterTryBlock() {
    inTryBlock = true;
    inCatchBlock = false;
  }

  void enterCatchBlock(const std::vector<std::string> &errorTypes = {}) {
    inTryBlock = false;
    inCatchBlock = true;
    catchableErrorTypes = errorTypes;
  }

  void exitErrorHandling() {
    inTryBlock = false;
    inCatchBlock = false;
    catchableErrorTypes.clear();
  }

  bool canCatch(const std::string &errorType) const {
    if (catchableErrorTypes.empty()) {
      return true; // Catch all errors
    }

    for (const auto &catchableType : catchableErrorTypes) {
      if (errorType == catchableType) {
        return true;
      }
    }
    return false;
  }
};

// Error propagation helper
class ErrorPropagator {
public:
  static bool shouldPropagate(const ErrorValue & /* error */,
                              const ErrorContext &context) {
    if (!context.inTryBlock && !context.inCatchBlock) {
      return true; // No error handling context, propagate
    }

    if (context.inCatchBlock) {
      // In catch block, check if this error type can be caught
      return !context.canCatch("Error"); // Simplified type checking
    }

    return false; // In try block, don't propagate yet
  }
};

} // namespace miniswift

#endif // MINISWIFT_ERROR_HANDLING_H