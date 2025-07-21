#include "AccessControl.h"
#include <stdexcept>

namespace miniswift {

std::string accessLevelToString(AccessLevel level) {
  switch (level) {
    case AccessLevel::PRIVATE:
      return "private";
    case AccessLevel::FILEPRIVATE:
      return "fileprivate";
    case AccessLevel::INTERNAL:
      return "internal";
    case AccessLevel::PACKAGE:
      return "package";
    case AccessLevel::PUBLIC:
      return "public";
    case AccessLevel::OPEN:
      return "open";
    default:
      return "unknown";
  }
}

AccessLevel tokenToAccessLevel(TokenType token) {
  switch (token) {
    case TokenType::Private:
      return AccessLevel::PRIVATE;
    case TokenType::Fileprivate:
      return AccessLevel::FILEPRIVATE;
    case TokenType::Internal:
      return AccessLevel::INTERNAL;
    case TokenType::Package:
      return AccessLevel::PACKAGE;
    case TokenType::Public:
      return AccessLevel::PUBLIC;
    case TokenType::Open:
      return AccessLevel::OPEN;
    default:
      throw std::runtime_error("Invalid access level token");
  }
}

TokenType accessLevelToToken(AccessLevel level) {
  switch (level) {
    case AccessLevel::PRIVATE:
      return TokenType::Private;
    case AccessLevel::FILEPRIVATE:
      return TokenType::Fileprivate;
    case AccessLevel::INTERNAL:
      return TokenType::Internal;
    case AccessLevel::PACKAGE:
      return TokenType::Package;
    case AccessLevel::PUBLIC:
      return TokenType::Public;
    case AccessLevel::OPEN:
      return TokenType::Open;
    default:
      throw std::runtime_error("Invalid access level");
  }
}

bool canAccess(AccessLevel entityLevel, const AccessContext& entityContext,
               const AccessContext& accessContext) {
  switch (entityLevel) {
    case AccessLevel::OPEN:
    case AccessLevel::PUBLIC:
      // Always accessible from anywhere
      return true;
      
    case AccessLevel::PACKAGE:
      // Accessible within the same package
      return entityContext.packageName == accessContext.packageName;
      
    case AccessLevel::INTERNAL:
      // Accessible within the same module
      return entityContext.moduleName == accessContext.moduleName;
      
    case AccessLevel::FILEPRIVATE:
      // Accessible within the same source file
      return entityContext.fileName == accessContext.fileName;
      
    case AccessLevel::PRIVATE:
      // Accessible only within the same scope (class, struct, etc.)
      return entityContext.fileName == accessContext.fileName &&
             entityContext.scopeName == accessContext.scopeName;
      
    default:
      return false;
  }
}

bool isValidAccessLevelCombination(AccessLevel entityLevel, AccessLevel dependencyLevel) {
  // An entity cannot be more accessible than its dependencies
  // For example, a public function cannot use a private type
  return !isLessRestrictive(entityLevel, dependencyLevel);
}

AccessLevel getDefaultAccessLevel() {
  return AccessLevel::INTERNAL;
}

AccessLevel getDefaultSetterAccessLevel(AccessLevel getterLevel) {
  // By default, setter has the same access level as getter
  return getterLevel;
}

} // namespace miniswift