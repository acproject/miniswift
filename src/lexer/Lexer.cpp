#include "Lexer.h"
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
  keywords["protocol"] = TokenType::Protocol;
  keywords["extension"] = TokenType::Extension;
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
  keywords["do"] = TokenType::Do;
  keywords["as"] = TokenType::As;
  keywords["is"] = TokenType::Is;
  keywords["try"] = TokenType::Try;
  keywords["catch"] = TokenType::Catch;
  keywords["throw"] = TokenType::Throw;
  keywords["throws"] = TokenType::Throws;
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
  keywords["static"] = TokenType::Static;
  keywords["subscript"] = TokenType::Subscript;
  keywords["where"] = TokenType::Where;
  keywords["inout"] = TokenType::Inout;
  keywords["mutating"] = TokenType::Mutating;

  // Access Control Keywords
  keywords["open"] = TokenType::Open;
  keywords["public"] = TokenType::Public;
  keywords["package"] = TokenType::Package;
  keywords["internal"] = TokenType::Internal;
  keywords["fileprivate"] = TokenType::Fileprivate;
  keywords["private"] = TokenType::Private;

  // Basic types
  keywords["String"] = TokenType::String;
  keywords["Int"] = TokenType::Int;
  keywords["Bool"] = TokenType::Bool;
  keywords["Double"] = TokenType::Double;

  // Extended Integer Types
  keywords["Int8"] = TokenType::Int8;
  keywords["Int16"] = TokenType::Int16;
  keywords["Int32"] = TokenType::Int32;
  keywords["Int64"] = TokenType::Int64;
  keywords["UInt"] = TokenType::UInt;
  keywords["UInt8"] = TokenType::UInt8;
  keywords["UInt16"] = TokenType::UInt16;

  keywords["UInt64"] = TokenType::UInt64;

  // Additional Basic Types
  keywords["Float"] = TokenType::Float;
  keywords["Character"] = TokenType::Character;

  // Special Types
  keywords["Any"] = TokenType::Any;
  keywords["Void"] = TokenType::Void;
  keywords["Set"] = TokenType::Set;

  // Custom Operator Keywords
  keywords["operator"] = TokenType::Operator;
  keywords["prefix"] = TokenType::Prefix;
  keywords["infix"] = TokenType::Infix;
  keywords["postfix"] = TokenType::Postfix;
  keywords["precedence"] = TokenType::Precedence;
  keywords["precedencegroup"] = TokenType::PrecedenceGroup;
  keywords["associativity"] = TokenType::Associativity;
  keywords["left"] = TokenType::Left;
  keywords["right"] = TokenType::Right;
  keywords["none"] = TokenType::None;

  // Result Builder Keywords
  keywords["resultBuilder"] = TokenType::ResultBuilder;

  // Concurrency Keywords
  keywords["async"] = TokenType::Async;
  keywords["await"] = TokenType::Await;
  keywords["actor"] = TokenType::Actor;
  keywords["Task"] = TokenType::Task;
  keywords["MainActor"] = TokenType::MainActor;
  keywords["globalActor"] = TokenType::GlobalActor;
  keywords["Sendable"] = TokenType::Sendable;
  keywords["isolated"] = TokenType::Isolated;
  keywords["nonisolated"] = TokenType::Nonisolated;
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
        // Return InterpolationEnd to properly close the interpolation
        return {TokenType::InterpolationEnd, ")", line};
      } else {
        return {TokenType::RParen, ")", line};
      }
    }

    // Put the character back and process normally
    current--;
  }

  // Handle string literal continuation after interpolation
  if (inStringLiteral) {
    return stringLiteral();
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
        return {TokenType::InterpolationEnd, ")", line};
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
      if (match('.'))
        return {TokenType::Ellipsis, "...", line};
      // If we have .. but not ..< or ..., it's an error
      return {TokenType::Unknown, "..", line};
    }
    return {TokenType::Dot, ".", line};
  case '-':
    if (match('='))
      return {TokenType::MinusEqual, "-=", line};
    if (match('>'))
      return {TokenType::Arrow, "->", line};
    return {TokenType::Minus, "-", line};
  case '+':
    if (match('='))
      return {TokenType::PlusEqual, "+=", line};
    return {TokenType::Plus, "+", line};
  case ';':
    return {TokenType::Semicolon, ";", line};
  case '*':
    if (match('='))
      return {TokenType::StarEqual, "*=", line};
    if (match('*'))
      return {TokenType::StarStar, "**", line};
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
    if (match('<'))
      return {TokenType::LeftShift, "<<", line};
    if (match('='))
      return {TokenType::LessEqual, "<=", line};
    if (match('>'))
      return {TokenType::LAngleRAngle, "<>", line};
    return {TokenType::LAngle, "<", line};
  case '>':
    if (match('>'))
      return {TokenType::RightShift, ">>", line};
    if (match('='))
      return {TokenType::GreaterEqual, ">=", line};
    return {TokenType::RAngle, ">", line};
  case '/':
    if (match('='))
      return {TokenType::SlashEqual, "/=", line};
    if (match('/')) {
      while (peek() != '\n' && current < source.length())
        advance();
      return scanToken(); // Skip comments
    }
    return {TokenType::Slash, "/", line};
  case '%':
    if (match('='))
      return {TokenType::PercentEqual, "%=", line};
    return {TokenType::Percent, "%", line};
  case '&':
    if (match('&'))
      return {TokenType::AmpAmp, "&&", line};
    if (match('+'))
      return {TokenType::OverflowPlus, "&+", line};
    if (match('-'))
      return {TokenType::OverflowMinus, "&-", line};
    if (match('*'))
      return {TokenType::OverflowStar, "&*", line};
    return {TokenType::BitwiseAnd, "&", line};
  case '|':
    if (match('|'))
      return {TokenType::PipePipe, "||", line};
    return {TokenType::BitwiseOr, "|", line};
  case '^':
    return {TokenType::BitwiseXor, "^", line};
  case '~':
    return {TokenType::BitwiseNot, "~", line};
  case '?':
    if (match('?'))
      return {TokenType::QuestionQuestion, "??", line};
    if (match('.'))
      return {TokenType::QuestionDot, "?.", line};
    if (match('['))
      return {TokenType::QuestionLSquare, "?[", line};
    return {TokenType::Question, "?", line};
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
      advance(); // consume backslash
      advance(); // consume (
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
  // std::cout << "Lexer found identifier: " << text << std::endl;

  auto it = keywords.find(text);
  if (it != keywords.end()) {
    // std::cout << "Recognized as keyword: " << static_cast<int>(it->second)
    //           << std::endl;
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