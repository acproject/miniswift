#include "src/interpreter/Interpreter.h"
#include "src/lexer/Lexer.h"
#include "src/parser/Parser.h"
#include <iostream>

int main() {
  std::string source = "let a = 10; print(a)";

  std::cout << "=== Testing print statement ===" << std::endl;
  std::cout << "Source: " << source << std::endl;

  // Lexical analysis
  miniswift::Lexer lexer(source);
  std::vector<miniswift::Token> tokens = lexer.scanTokens();

  std::cout << "\n=== Tokens ===" << std::endl;
  for (const auto &token : tokens) {
    std::cout << "Type: " << static_cast<int>(token.type) << ", Lexeme: '"
              << token.lexeme << "'" << std::endl;
  }

  // Parsing
  std::cout << "\n=== Parsing ===" << std::endl;
  miniswift::Parser parser(tokens);
  try {
    auto statements = parser.parse();
    std::cout << "Parsed " << statements.size() << " statements successfully."
              << std::endl;

    // Interpretation
    std::cout << "\n=== Interpretation ===" << std::endl;
    miniswift::Interpreter interpreter;
    interpreter.interpret(statements);
  } catch (const std::exception &e) {
    std::cout << "Error: " << e.what() << std::endl;
  }

  return 0;
}