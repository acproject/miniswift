#include "interpreter/Interpreter.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include <fstream>
#include <iostream>
#include <sstream>

static miniswift::Interpreter interpreter;
bool hadError = false;

void run(const std::string &source) {
  bool currentError = false;

  miniswift::Lexer lexer(source);
  std::vector<miniswift::Token> tokens = lexer.scanTokens();

  miniswift::Parser parser(tokens);
  std::vector<std::unique_ptr<miniswift::Stmt>> statements;
  try {
    statements = parser.parse();
  } catch (const std::runtime_error &e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    currentError = true;
  }

  // Stop if there was a syntax error for this statement only
  if (currentError)
    return;

  // Try to interpret, catch runtime errors
  try {
    interpreter.interpret(statements);
  } catch (const std::exception &e) {
    std::cerr << "RUNTIME ERROR: " << e.what() << std::endl;
  }
}

void runFile(const std::string &path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Could not open file: " << path << std::endl;
    return;
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string content = buffer.str();
  run(content);
}

int main(int argc, char *argv[]) {
  if (argc > 2) {
    std::cout << "Usage: miniswift [script]" << std::endl;
    return 64;
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    std::string line;
    std::cout << "MiniSwift Interactive Mode (输入 'exit' 或 'quit' 退出)"
              << std::endl;
    while (true) {
      std::cout << "> ";
      if (!std::getline(std::cin, line))
        break;

      // 检查退出命令
      if (line == "exit" || line == "quit" || line == "q") {
        std::cout << "再见" << std::endl;
        break;
      }

      // 跳过空行
      if (line.empty())
        continue;

      run(line);
    }
  }
  return 0;
}