#ifndef MINISWIFT_TOKEN_H
#define MINISWIFT_TOKEN_H

#include <string>

namespace miniswift {

enum class TokenType {
  // Keywords
  Var,
  Let,
  Func,
  Class,
  Struct,
  Enum,
  If,
  Else,
  For,
  In,
  While,
  Switch,
  Case,
  Default,
  Guard,
  Defer,
  As,
  Is,
  Try,
  Catch,
  Throw,
  Nil,
  Self,
  True,
  False,
  Print,
  Return,
  Get,
  WillSet,
  DidSet,
  Init,
  Deinit,
  Super,
  Override,
  Final,
  Static,
  Subscript,
  Protocol,
  Extension,
  Inout,
  Mutating,
  
  // Error Handling Keywords
  Throws,
  Rethrows,
  Do,
  Result,
  Error,

  // Access Control Keywords
  Open,
  Public,
  Package,
  Internal,
  Fileprivate,
  Private,

  // Basic Types
  String,
  Int,
  Bool,
  Double,
  
  // Extended Integer Types
  Int8,
  Int16,
  Int32,
  Int64,
  UInt,
  UInt8,
  UInt16,
  UInt64,
  
  // Additional Basic Types
  Float,
  Character,
  
  // Special Types
  Any,
  Void,
  Set,

  // Operators
  Plus,
  Minus,
  Star,
  Slash,
  Percent,
  Equal,
  EqualEqual,
  BangEqual,
  Less,
  LessEqual,
  Greater,
  GreaterEqual,
  AmpAmp,
  PipePipe,
  Bang,
  Ellipsis,
  LessEllipsis,
  QuestionQuestion,
  QuestionDot,
  QuestionLSquare, // ?[
  Question, // ?
  Arrow, // ->
  
  // Compound Assignment Operators
  PlusEqual,    // +=
  MinusEqual,   // -=
  StarEqual,    // *=
  SlashEqual,   // /=
  PercentEqual, // %=
  
  // Bitwise Operators
  BitwiseAnd,      // &
  BitwiseOr,       // |
  BitwiseXor,      // ^
  BitwiseNot,      // ~
  LeftShift,       // <<
  RightShift,      // >>
  
  // Overflow Operators
  OverflowPlus,    // &+
  OverflowMinus,   // &-
  OverflowStar,    // &*
  
  // Custom Operator Keywords
  Operator,        // operator keyword
  Prefix,          // prefix keyword
  Infix,           // infix keyword
  Postfix,         // postfix keyword
  Precedence,      // precedence keyword
  PrecedenceGroup, // precedencegroup keyword
  Associativity,   // associativity keyword
  Left,            // left keyword
  Right,           // right keyword
  None,            // none keyword
  
  // Result Builder Keywords
  ResultBuilder,   // @resultBuilder

  // Punctuation
  LParen,
  RParen,
  LBrace,
  RBrace,
  LSquare,
  RSquare,
  Comma,
  Colon,
  Semicolon,
  Dot,
  At,
  Amp,

  // Literals
  Identifier,
  IntegerLiteral,
  FloatingLiteral,
  StringLiteral,
  InterpolatedStringLiteral,

  // String Interpolation
  InterpolationStart, // \(
  InterpolationEnd,   // )

  // Generics
  LAngle,    // <
  RAngle,    // >
  Where,     // where keyword for generic constraints
  
  // Other
  EndOfFile,
  Unknown
};

struct Token {
  Token(TokenType type, const std::string &lexeme, int line)
      : type(type), lexeme(lexeme), line(line) {}
  TokenType type;
  std::string lexeme;
  int line;
};

} // namespace miniswift

#endif // MINISWIFT_TOKEN_H