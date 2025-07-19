#ifndef MINISWIFT_LEXER_H
#define MINISWIFT_LEXER_H

#include "Token.h"
#include <string>
#include <vector>

namespace miniswift {

class Lexer {
public:
  Lexer(const std::string &source);
  std::vector<Token> scanTokens();

private:
  Token scanToken();
  char advance();
  bool match(char expected);
  char peek();
  char peekNext();
  void skipWhitespace();
  Token stringLiteral();
  Token number();
  Token identifier();
  TokenType identifierType();

  // Unicode identifier support
  bool isUnicodeIdentifierStart(char c);
  bool isUnicodeIdentifierContinue(char c);

  std::string source;
  size_t start = 0;
  size_t current = 0;
  int line = 1;
  bool inInterpolation = false;
  bool inStringLiteral = false; // Track if we're inside a string literal
  int interpolationDepth = 0;   // Track nested interpolations
};

} // namespace miniswift

#endif // MINISWIFT_LEXER_H