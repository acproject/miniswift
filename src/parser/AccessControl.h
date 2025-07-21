#ifndef MINISWIFT_ACCESS_CONTROL_H
#define MINISWIFT_ACCESS_CONTROL_H

#include "../lexer/Token.h"
#include <string>

namespace miniswift {

/**
 * Access levels in Swift, ordered from most restrictive to least restrictive
 */
enum class AccessLevel {
  PRIVATE = 0,      // Only accessible within the same declaration
  FILEPRIVATE = 1,  // Accessible within the same source file
  INTERNAL = 2,     // Accessible within the same module (default)
  PACKAGE = 3,      // Accessible within the same package
  PUBLIC = 4,       // Accessible from other modules, but not inheritable
  OPEN = 5          // Accessible from other modules and inheritable
};

/**
 * Access control context for checking access permissions
 */
struct AccessContext {
  std::string moduleName;
  std::string packageName;
  std::string fileName;
  std::string scopeName;  // For private access checking
  
  AccessContext(const std::string& module = "main", 
                const std::string& package = "main", 
                const std::string& file = "",
                const std::string& scope = "")
    : moduleName(module), packageName(package), fileName(file), scopeName(scope) {}
};

// Access level comparison functions
inline bool isMoreRestrictive(AccessLevel a, AccessLevel b) {
  return static_cast<int>(a) < static_cast<int>(b);
}

inline bool isLessRestrictive(AccessLevel a, AccessLevel b) {
  return static_cast<int>(a) > static_cast<int>(b);
}

inline bool isEquallyRestrictive(AccessLevel a, AccessLevel b) {
  return a == b;
}

// Utility functions
std::string accessLevelToString(AccessLevel level);
AccessLevel tokenToAccessLevel(TokenType token);
TokenType accessLevelToToken(AccessLevel level);

// Access control validation
bool canAccess(AccessLevel entityLevel, const AccessContext& entityContext,
               const AccessContext& accessContext);

bool isValidAccessLevelCombination(AccessLevel entityLevel, AccessLevel dependencyLevel);

// Default access levels
AccessLevel getDefaultAccessLevel();
AccessLevel getDefaultSetterAccessLevel(AccessLevel getterLevel);

} // namespace miniswift

#endif // MINISWIFT_ACCESS_CONTROL_H