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
  Set,
  WillSet,
  DidSet,
  Init,
  Deinit,
  Super,
  Override,
  Final,
  Static,
  Subscript,

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
  Arrow, // ->

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