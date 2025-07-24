#include "interpreter/Interpreter.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semantic/SemanticAnalyzer.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

static miniswift::Interpreter interpreter;
bool hadError = false;
bool enableSemanticAnalysis = false;
bool enableLLVMCodeGen = false;
bool compileToExecutable = false;

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

  // Perform semantic analysis if enabled
  if (enableSemanticAnalysis) {
    try {
      auto semanticAnalyzer = std::make_shared<miniswift::SemanticAnalyzer>();
      auto analysisResult = semanticAnalyzer->analyze(statements);

      if (!analysisResult.errors.empty()) {
        std::cerr << "SEMANTIC ERRORS:" << std::endl;
        for (const auto &error : analysisResult.errors) {
          std::cerr << "  " << error.message << " at line " << error.line
                    << std::endl;
        }
        return;
      }

      if (!analysisResult.warnings.empty()) {
        std::cerr << "SEMANTIC WARNINGS:" << std::endl;
        for (const auto &warning : analysisResult.warnings) {
          std::cerr << "  " << warning << std::endl;
        }
      }

      std::cout << "Semantic analysis completed successfully." << std::endl;

      // LLVM code generation disabled - semantic analysis only

    } catch (const std::exception &e) {
      std::cerr << "SEMANTIC ANALYSIS ERROR: " << e.what() << std::endl;
    }
  } else {
    // Traditional interpretation
    try {
      interpreter.interpret(statements);
    } catch (const std::exception &e) {
      std::cerr << "RUNTIME ERROR: " << e.what() << std::endl;
    }
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

void printUsage() {
  std::cout << "Usage: miniswift [options] [script]" << std::endl;
  std::cout << "Options:" << std::endl;
  std::cout << "  -s, --semantic    Enable semantic analysis" << std::endl;
  // LLVM options disabled
  // std::cout << "  -l, --llvm        Enable LLVM code generation (requires
  // -s)"
  //           << std::endl;
  // std::cout << "  -c, --compile     Compile to executable (requires -l)" <<
  // std::endl;
  std::cout << "  -h, --help        Show this help message" << std::endl;
}

int main(int argc, char *argv[]) {
  std::string scriptFile;

  // Parse command line arguments
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];

    if (arg == "-s" || arg == "--semantic") {
      enableSemanticAnalysis = true;
    } else if (arg == "-l" || arg == "--llvm") {
      std::cerr << "Error: LLVM code generation is disabled in this build"
                << std::endl;
      return 64;
    } else if (arg == "-c" || arg == "--compile") {
      std::cerr << "Error: Compilation is disabled in this build" << std::endl;
      return 64;
    } else if (arg == "-h" || arg == "--help") {
      printUsage();
      return 0;
    } else if (arg[0] != '-') {
      if (!scriptFile.empty()) {
        std::cerr << "Error: Multiple script files specified" << std::endl;
        printUsage();
        return 64;
      }
      scriptFile = arg;
    } else {
      std::cerr << "Error: Unknown option " << arg << std::endl;
      printUsage();
      return 64;
    }
  }

  if (!scriptFile.empty()) {
    runFile(scriptFile);
  } else {
    std::string line;
    std::cout << "MiniSwift Interactive Mode";
    if (enableSemanticAnalysis) {
      std::cout << " (Semantic Analysis Enabled)";
    }
    std::cout << std::endl;
    std::cout << "input 'exit' or 'quit' to exit" << std::endl;

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