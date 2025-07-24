#include "codegen/LLVMCodeGenerator.h"
#include "interpreter/Interpreter.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semantic/SemanticAnalyzer.h"
#include <fstream>
#include <iostream>
#include <llvm/Support/raw_ostream.h>
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

      // Generate LLVM IR if enabled
      if (enableLLVMCodeGen && analysisResult.typedAST) {
        try {
          auto codeGenerator = std::make_shared<miniswift::LLVMCodeGenerator>();
          auto codeGenResult =
              codeGenerator->generateCode(*analysisResult.typedAST);

          if (!codeGenResult.errors.empty()) {
            std::cerr << "CODE GENERATION ERRORS:" << std::endl;
            for (const auto &error : codeGenResult.errors) {
              std::cerr << "  " << error.message << std::endl;
            }
            return;
          }

          std::cout << "LLVM IR generation completed successfully."
                    << std::endl;

          // Compile to executable if requested
          if (compileToExecutable) {
            std::string outputFile = "output";
            if (codeGenerator->compileToExecutable(outputFile)) {
              std::cout << "Compilation to executable completed successfully." << std::endl;
              std::cout << "Output file: " << outputFile << ".exe" << std::endl;
            } else {
              std::cerr << "Failed to compile to executable." << std::endl;
              // 显示详细的错误信息
              auto newResult = codeGenerator->generateCode(*analysisResult.typedAST);
              if (!newResult.errors.empty()) {
                std::cerr << "COMPILATION ERRORS:" << std::endl;
                for (const auto &error : newResult.errors) {
                  std::cerr << "  " << error.message << std::endl;
                }
              }
              if (!newResult.warnings.empty()) {
                std::cerr << "COMPILATION WARNINGS:" << std::endl;
                for (const auto &warning : newResult.warnings) {
                  std::cerr << "  " << warning << std::endl;
                }
              }
            }
          } else {
            // Print generated IR only if not compiling
            if (codeGenResult.module) {
              std::cout << "Generated LLVM IR:" << std::endl;
              std::string irString;
              llvm::raw_string_ostream irStream(irString);
              codeGenResult.module->print(irStream, nullptr);
              std::cout << irString << std::endl;
            }
          }

        } catch (const std::exception &e) {
          std::cerr << "CODE GENERATION ERROR: " << e.what() << std::endl;
        }
      }

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
  std::cout << "  -l, --llvm        Enable LLVM code generation (requires -s)"
            << std::endl;
  std::cout << "  -c, --compile     Compile to executable (requires -l)" << std::endl;
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
      enableLLVMCodeGen = true;
      enableSemanticAnalysis = true; // LLVM code gen requires semantic analysis
    } else if (arg == "-c" || arg == "--compile") {
      compileToExecutable = true;
      enableLLVMCodeGen = true;
      enableSemanticAnalysis = true; // Compilation requires LLVM and semantic analysis
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
      std::cout << " (Semantic Analysis Enabled";
      if (enableLLVMCodeGen) {
        std::cout << " + LLVM Code Generation";
      }
      std::cout << ")";
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