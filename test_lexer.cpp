#include "src/lexer/Lexer.h"
#include <iomanip>
#include <iostream>

using namespace miniswift;

std::string tokenTypeToString(TokenType type) {
  switch (type) {
  case TokenType::Var:
    return "Var";
  case TokenType::Let:
    return "Let";
  case TokenType::Func:
    return "Func";
  case TokenType::Class:
    return "Class";
  case TokenType::Struct:
    return "Struct";
  case TokenType::Enum:
    return "Enum";
  case TokenType::If:
    return "If";
  case TokenType::Else:
    return "Else";
  case TokenType::For:
    return "For";
  case TokenType::In:
    return "In";
  case TokenType::While:
    return "While";
  case TokenType::Switch:
    return "Switch";
  case TokenType::Case:
    return "Case";
  case TokenType::Default:
    return "Default";
  case TokenType::Guard:
    return "Guard";
  case TokenType::Defer:
    return "Defer";
  case TokenType::As:
    return "As";
  case TokenType::Try:
    return "Try";
  case TokenType::Catch:
    return "Catch";
  case TokenType::Throw:
    return "Throw";
  case TokenType::Nil:
    return "Nil";
  case TokenType::Self:
    return "Self";
  case TokenType::True:
    return "True";
  case TokenType::False:
    return "False";
  case TokenType::Print:
    return "Print";
  case TokenType::Plus:
    return "Plus";
  case TokenType::Minus:
    return "Minus";
  case TokenType::Star:
    return "Star";
  case TokenType::Slash:
    return "Slash";
  case TokenType::Percent:
    return "Percent";
  case TokenType::Equal:
    return "Equal";
  case TokenType::EqualEqual:
    return "EqualEqual";
  case TokenType::BangEqual:
    return "BangEqual";
  case TokenType::Less:
    return "Less";
  case TokenType::LessEqual:
    return "LessEqual";
  case TokenType::Greater:
    return "Greater";
  case TokenType::GreaterEqual:
    return "GreaterEqual";
  case TokenType::AmpAmp:
    return "AmpAmp";
  case TokenType::PipePipe:
    return "PipePipe";
  case TokenType::Bang:
    return "Bang";
  case TokenType::Ellipsis:
    return "Ellipsis";
  case TokenType::LessEllipsis:
    return "LessEllipsis";
  case TokenType::QuestionQuestion:
    return "QuestionQuestion";
  case TokenType::QuestionDot:
    return "QuestionDot";
  case TokenType::LParen:
    return "LParen";
  case TokenType::RParen:
    return "RParen";
  case TokenType::LBrace:
    return "LBrace";
  case TokenType::RBrace:
    return "RBrace";
  case TokenType::LSquare:
    return "LSquare";
  case TokenType::RSquare:
    return "RSquare";
  case TokenType::Comma:
    return "Comma";
  case TokenType::Colon:
    return "Colon";
  case TokenType::Semicolon:
    return "Semicolon";
  case TokenType::Dot:
    return "Dot";
  case TokenType::At:
    return "At";
  case TokenType::Amp:
    return "Amp";
  case TokenType::Identifier:
    return "Identifier";
  case TokenType::IntegerLiteral:
    return "IntegerLiteral";
  case TokenType::FloatingLiteral:
    return "FloatingLiteral";
  case TokenType::StringLiteral:
    return "StringLiteral";
  case TokenType::InterpolatedStringLiteral:
    return "InterpolatedStringLiteral";
  case TokenType::InterpolationStart:
    return "InterpolationStart";
  case TokenType::InterpolationEnd:
    return "InterpolationEnd";
  case TokenType::EndOfFile:
    return "EndOfFile";
  case TokenType::Unknown:
    return "Unknown";
  default:
    return "Unknown";
  }
}

void testLexer(const std::string &source, const std::string &testName) {
  std::cout << "\n=== Testing: " << testName << " ===\n";
  std::cout << "Source: " << source << "\n\n";

  Lexer lexer(source);
  auto tokens = lexer.scanTokens();

  std::cout << std::left << std::setw(20) << "Token Type" << std::setw(20)
            << "Lexeme"
            << "Line" << std::endl;
  std::cout << std::string(50, '-') << std::endl;

  for (const auto &token : tokens) {
    std::cout << std::left << std::setw(20) << tokenTypeToString(token.type)
              << std::setw(20) << ("\"" + token.lexeme + "\"") << token.line
              << std::endl;
  }
}

int main() {
  std::cout << "MiniSwift Lexer Test Suite\n";
  std::cout << "==========================\n";

  // Test 1: Basic variable declarations
  testLexer("var environment = \"development\"", "Basic variable declaration");

  // Test 2: Constant with type annotation
  testLexer("let maximumNumberOfLoginAttempts: Int",
            "Constant with type annotation");

  // Test 3: Multiple variable declaration
  testLexer("var x = 0.0, y = 0.0, z = 0.0", "Multiple variable declaration");

  // Test 4: Multiple variables with type annotation
  testLexer("var red, green, blue: Double", "Multiple variables with type");

  // Test 5: Unicode identifiers
  testLexer("let π = 3.14159", "Unicode identifier (π)");

  // Test 6: Chinese characters
  testLexer("let 你好 = \"你好世界\"", "Chinese identifier");

  // Test 7: Emoji identifiers
  testLexer("let 🐶🐮 = \"dogcow\"", "Emoji identifier");

  // Test 8: String interpolation
  testLexer("print(\"The current value is \\(friendlyWelcome)\")",
            "String interpolation");

  // Test 9: Conditional assignment
  testLexer(
      "if environment == \"development\" {\n    maximumNumberOfLoginAttempts = "
      "100\n} else {\n    maximumNumberOfLoginAttempts = 10\n}",
      "Conditional assignment");

  // Test 10: Complex expression
  testLexer("let message = \"\\(multiplier) times 2.5 is \\(Double(multiplier) "
            "* 2.5)\"",
            "Complex string interpolation");

  return 0;
}