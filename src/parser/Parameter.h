#ifndef MINISWIFT_PARAMETER_H
#define MINISWIFT_PARAMETER_H

#include "../lexer/Token.h"

namespace miniswift {

// Function parameter
struct Parameter {
  Token name; // Internal parameter name
  Token externalName; // External parameter name (for function calls)
  Token type; // Can be empty for type inference
  bool isInout; // true if parameter is inout
  bool isVariadic; // true if parameter is variadic (...)

  Parameter(Token n, Token t, bool inout = false, bool variadic = false) 
    : name(n), externalName(n), type(t), isInout(inout), isVariadic(variadic) {}
    
  Parameter(Token n, Token ext, Token t, bool inout = false, bool variadic = false) 
    : name(n), externalName(ext), type(t), isInout(inout), isVariadic(variadic) {}
};

} // namespace miniswift

#endif // MINISWIFT_PARAMETER_H