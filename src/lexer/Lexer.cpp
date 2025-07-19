#include "Lexer.h"
#include <iostream>
#include <unordered_map>

namespace miniswift {

static std::unordered_map<std::string, TokenType> keywords;

static void initializeKeywords() {
  keywords["var"] = TokenType::Var;
  keywords["let"] = TokenType::Let;
  keywords["func"] = TokenType::Func;
  keywords["class"] = TokenType::Class;
  keywords["struct"] = TokenType::Struct;
  keywords["enum"] = TokenType::Enum;
  keywords["if"] = TokenType::If;
  keywords["else"] = TokenType::Else;
  keywords["for"] = TokenType::For;
  keywords["in"] = TokenType::In;
  keywords["while"] = TokenType::While;
  keywords["switch"] = TokenType::Switch;
  keywords["case"] = TokenType::Case;
  keywords["default"] = TokenType::Default;
  keywords["guard"] = TokenType::Guard;
  keywords["defer"] = TokenType::Defer;
  keywords["as"] = TokenType::As;
  keywords["try"] = TokenType::Try;
  keywords["catch"] = TokenType::Catch;
  keywords["throw"] = TokenType::Throw;
  keywords["nil"] = TokenType::Nil;
  keywords["self"] = TokenType::Self;
  keywords["true"] = TokenType::True;
  keywords["false"] = TokenType::False;
  keywords["print"] = TokenType::Print;
  keywords["return"] = TokenType::Return;
  keywords["get"] = TokenType::Get;
  keywords["set"] = TokenType::Set;
  keywords["willSet"] = TokenType::WillSet;
  keywords["didSet"] = TokenType::DidSet;
  keywords["init"] = TokenType::Init;
  keywords["deinit"] = TokenType::Deinit;
  keywords["super"] = TokenType::Super;
  keywords["override"] = TokenType::Override;
  keywords["final"] = TokenType::Final;
}

Lexer::Lexer(const std::string &source)
    : source(source), inInterpolation(false), inStringLiteral(false),
      interpolationDepth(0) {
  initializeKeywords();
}

std::vector<Token> Lexer::scanTokens() {
  std::vector<Token> tokens;
  while (current < source.length()) {
    start = current;
    tokens.push_back(scanToken());
  }
  tokens.push_back({TokenType::EndOfFile, "", line});
  return tokens;
}

Token Lexer::scanToken() {
  // Handle interpolation mode
  if (inInterpolation) {
    skipWhitespace();
    start = current;
    if (current >= source.length())
      return {TokenType::EndOfFile, "", line};

    char c = advance();

    if (c == ')') {
      interpolationDepth--;
      if (interpolationDepth == 0) {
        inInterpolation = false;
        // After interpolation ends, we're back in string context
        inStringLiteral = true;
        // Return RParen first, then handle string continuation in next call
        return {TokenType::RParen, ")", line};
      } else {
        return {TokenType::RParen, ")", line};
      }
    }

    // Put the character back and process normally
    current--;
  }

  skipWhitespace();
  start = current;
  if (current >= source.length())
    return {TokenType::EndOfFile, "", line};

  char c = advance();

  // Support Unicode identifiers
  if (isalpha(c) || c == '_' || isUnicodeIdentifierStart(c))
    return identifier();
  if (isdigit(c))
    return number();

  switch (c) {
  case '(':
    if (inInterpolation) {
      interpolationDepth++;
    }
    return {TokenType::LParen, "(", line};
  case ')':
    if (inInterpolation) {
      interpolationDepth--;
      if (interpolationDepth == 0) {
        inInterpolation = false;
        inStringLiteral = true;
        return {TokenType::RParen, ")", line};
      }
    }
    return {TokenType::RParen, ")", line};
  case '{':
    return {TokenType::LBrace, "{", line};
  case '}':
    return {TokenType::RBrace, "}", line};
  case '[':
    return {TokenType::LSquare, "[", line};
  case ']':
    return {TokenType::RSquare, "]", line};
  case ',':
    return {TokenType::Comma, ",", line};
  case '.':
    if (match('.')) {
      if (match('<'))
        return {TokenType::LessEllipsis, "..<", line};
      return {TokenType::Ellipsis, "...", line};
    }
    return {TokenType::Dot, ".", line};
  case '-':
    if (match('>'))
      return {TokenType::Arrow, "->", line};
    return {TokenType::Minus, "-", line};
  case '+':
    return {TokenType::Plus, "+", line};
  case ';':
    return {TokenType::Semicolon, ";", line};
  case '*':
    return {TokenType::Star, "*", line};
  case '!':
    if (match('='))
      return {TokenType::BangEqual, "!=", line};
    return {TokenType::Bang, "!", line};
  case '=':
    if (match('='))
      return {TokenType::EqualEqual, "==", line};
    return {TokenType::Equal, "=", line};
  case '<':
    if (match('='))
      return {TokenType::LessEqual, "<=", line};
    return {TokenType::Less, "<", line};
  case '>':
    if (match('='))
      return {TokenType::GreaterEqual, ">=", line};
    return {TokenType::Greater, ">", line};
  case '/':
    if (match('/')) {
      while (peek() != '\n' && current < source.length())
        advance();
      return scanToken(); // Skip comments
    }
    return {TokenType::Slash, "/", line};
  case '%':
    return {TokenType::Percent, "%", line};
  case '&':
    if (match('&'))
      return {TokenType::AmpAmp, "&&", line};
    return {TokenType::Amp, "&", line};
  case '|':
    if (match('|'))
      return {TokenType::PipePipe, "||", line};
    return {TokenType::Unknown, "|", line};
  case '?':
    if (match('?'))
      return {TokenType::QuestionQuestion, "??", line};
    if (match('.'))
      return {TokenType::QuestionDot, "?.", line};
    return {TokenType::Unknown, "?", line};
  case '@':
    return {TokenType::At, "@", line};
  case ':':
    return {TokenType::Colon, ":", line};
  case '"':
    if (inStringLiteral) {
      // End of string
      inStringLiteral = false;
      return {TokenType::StringLiteral, "", line};
    } else {
      // Start of string
      inStringLiteral = true;
      return stringLiteral();
    }
  case '\\':
    if (inStringLiteral && peek() == '(') {
      advance(); // consume (
      inInterpolation = true;
      interpolationDepth = 1;
      inStringLiteral = false;
      return {TokenType::InterpolationStart, "\\(", line};
    }
    // Backslash outside of string context
    return {TokenType::Unknown, "\\", line};
  }

  return {TokenType::Unknown, std::string(1, c), line};
}

char Lexer::advance() { return source[current++]; }

bool Lexer::match(char expected) {
  if (current >= source.length())
    return false;
  if (source[current] != expected)
    return false;
  current++;
  return true;
}

char Lexer::peek() {
  if (current >= source.length())
    return '\0';
  return source[current];
}

char Lexer::peekNext() {
  if (current + 1 >= source.length())
    return '\0';
  return source[current + 1];
}

void Lexer::skipWhitespace() {
  while (true) {
    char c = peek();
    switch (c) {
    case ' ':
    case '\r':
    case '\t':
      advance();
      break;
    case '\n':
      line++;
      advance();
      break;
    default:
      return;
    }
  }
}

Token Lexer::stringLiteral() {
  std::string value;

  while (peek() != '"' && current < source.length()) {
    if (peek() == '\\' && peekNext() == '(') {
      if (!value.empty()) {
        // Return the string part before interpolation
        // Keep inStringLiteral true, we'll handle interpolation in next call
        return {TokenType::InterpolatedStringLiteral, value, line};
      }
      advance(); // consume \\ advance(); // consume (
      inInterpolation = true;
      interpolationDepth = 1;
      inStringLiteral = false; // We're now in interpolation mode
      return {TokenType::InterpolationStart, "\\(", line};
    }

    // Handle escape sequences
    if (peek() == '\\') {
      advance(); // consume backslash
      char escaped = peek();
      switch (escaped) {
      case 'n':
        value += '\n';
        advance();
        break;
      case 't':
        value += '\t';
        advance();
        break;
      case 'r':
        value += '\r';
        advance();
        break;
      case '\\':
        value += '\\';
        advance();
        break;
      case '"':
        value += '"';
        advance();
        break;
      case '0':
        value += '\0';
        advance();
        break;
      default:
        // Unknown escape sequence, keep the backslash
        value += '\\';
        break;
      }
    } else {
      if (peek() == '\n')
        line++;
      value += advance();
    }
  }

  if (current >= source.length()) {
    // Unterminated string.
    inStringLiteral = false;
    return {TokenType::Unknown, "Unterminated string.", line};
  }

  // The closing ".
  advance();
  inStringLiteral = false;

  // If this is an empty string after interpolation, it might be the end of an
  // interpolated string
  if (value.empty()) {
    return {TokenType::StringLiteral, "", line};
  }

  // Return the string content
  return {TokenType::StringLiteral, value, line};
}

Token Lexer::number() {
  while (isdigit(peek()))
    advance();

  if (peek() == '.' && isdigit(peekNext())) {
    advance();
    while (isdigit(peek()))
      advance();
    return {TokenType::FloatingLiteral, source.substr(start, current - start),
            line};
  }

  return {TokenType::IntegerLiteral, source.substr(start, current - start),
          line};
}

Token Lexer::identifier() {
  while (isalnum(peek()) || peek() == '_' ||
         isUnicodeIdentifierContinue(peek())) {
    advance();
  }

  std::string text = source.substr(start, current - start);
  std::cout << "Lexer found identifier: " << text << std::endl;

  auto it = keywords.find(text);
  if (it != keywords.end()) {
    std::cout << "Recognized as keyword: " << static_cast<int>(it->second)
              << std::endl;
    return {it->second, text, line};
  }

  return {TokenType::Identifier, text, line};
}

// Unicode identifier support functions
bool Lexer::isUnicodeIdentifierStart(char c) {
  // Basic Unicode support for common cases
  // This is a simplified implementation - full Unicode support would require
  // more complex logic
  unsigned char uc = static_cast<unsigned char>(c);

  // Support for Latin Extended, Greek, Cyrillic, CJK, and other common Unicode
  // ranges
  if (uc >= 0x80) {
    // This is a multi-byte UTF-8 character
    // For simplicity, we'll accept most high-bit characters as identifier
    // starts
    return true;
  }

  return false;
}

bool Lexer::isUnicodeIdentifierContinue(char c) {
  // Similar to isUnicodeIdentifierStart but also allows digits and some
  // additional characters
  return isUnicodeIdentifierStart(c);
}

} // namespace miniswift