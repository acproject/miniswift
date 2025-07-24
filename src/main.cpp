#include "interpreter/Interpreter.h"
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "semantic/SemanticAnalyzer.h"
#include "codegen/LLVMCodeGenerator.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <filesystem>

static miniswift::Interpreter interpreter;
bool hadError = false;
bool enableSemanticAnalysis = false;
bool enableLLVMCodeGen = false;
bool compileToExecutable = false;
bool compileToObject = false;
bool compileToAssembly = false;
bool showVersion = false;
bool verboseOutput = false;
std::string outputFile = "";

// 版本信息
const std::string MINISWIFT_VERSION = "1.0.0";
const std::string MINISWIFT_BUILD_DATE = __DATE__;
const std::string MINISWIFT_BUILD_TIME = __TIME__;

void printVersion() {
    std::cout << "MiniSwift Compiler " << MINISWIFT_VERSION << std::endl;
    std::cout << "Built on " << MINISWIFT_BUILD_DATE << " at " << MINISWIFT_BUILD_TIME << std::endl;
    std::cout << "LLVM Code Generation: Enabled" << std::endl;
    std::cout << "Copyright (c) 2024 MiniSwift Project" << std::endl;
}

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
        hadError = true;
        return;
      }

      if (!analysisResult.warnings.empty()) {
        std::cerr << "SEMANTIC WARNINGS:" << std::endl;
        for (const auto &warning : analysisResult.warnings) {
          std::cerr << "  " << warning << std::endl;
        }
      }

      if (verboseOutput) {
        std::cout << "Semantic analysis completed successfully." << std::endl;
      }

      // LLVM code generation if enabled
      if (enableLLVMCodeGen) {
        try {
          auto codeGenerator = std::make_unique<miniswift::LLVMCodeGenerator>();
          auto codeGenResult = codeGenerator->generateCode(*analysisResult.typedAST);
          
          if (codeGenResult.hasErrors()) {
            std::cerr << "CODE GENERATION ERRORS:" << std::endl;
            for (const auto &error : codeGenResult.errors) {
              std::cerr << "  " << error.message;
              if (error.line != -1) {
                std::cerr << " at line " << error.line;
              }
              std::cerr << std::endl;
            }
            hadError = true;
            return;
          }
          
          if (!codeGenResult.warnings.empty()) {
            std::cerr << "CODE GENERATION WARNINGS:" << std::endl;
            for (const auto &warning : codeGenResult.warnings) {
              std::cerr << "  " << warning << std::endl;
            }
          }
          
          if (verboseOutput) {
            std::cout << "LLVM IR generation completed successfully." << std::endl;
          }
          
          // 根据编译选项生成不同的输出
          if (compileToExecutable) {
            std::string executableName = outputFile.empty() ? "output" : outputFile;
            if (verboseOutput) {
              std::cout << "Compiling to executable: " << executableName << std::endl;
            }
            if (!codeGenerator->compileToExecutable(executableName)) {
              std::cerr << "Failed to compile to executable" << std::endl;
              hadError = true;
              return;
            }
            std::cout << "Successfully compiled to executable: " << executableName << std::endl;
          } else if (compileToObject) {
            std::string objectName = outputFile.empty() ? "output.o" : outputFile;
            if (verboseOutput) {
              std::cout << "Compiling to object file: " << objectName << std::endl;
            }
            if (!codeGenerator->compileToObjectFile(objectName)) {
              std::cerr << "Failed to compile to object file" << std::endl;
              hadError = true;
              return;
            }
            std::cout << "Successfully compiled to object file: " << objectName << std::endl;
          } else if (compileToAssembly) {
            std::string assemblyName = outputFile.empty() ? "output.s" : outputFile;
            if (verboseOutput) {
              std::cout << "Compiling to assembly: " << assemblyName << std::endl;
            }
            if (!codeGenerator->compileToAssembly(assemblyName)) {
              std::cerr << "Failed to compile to assembly" << std::endl;
              hadError = true;
              return;
            }
            std::cout << "Successfully compiled to assembly: " << assemblyName << std::endl;
          } else {
            // 默认JIT执行
            if (verboseOutput) {
              std::cout << "Initializing JIT execution..." << std::endl;
            }
            if (codeGenerator->initializeJIT()) {
              // 尝试执行main函数
              try {
                auto result = codeGenerator->executeFunction("main", {});
                if (verboseOutput) {
                  std::cout << "JIT execution completed." << std::endl;
                }
              } catch (const std::exception &e) {
                std::cerr << "JIT EXECUTION ERROR: " << e.what() << std::endl;
              }
            } else {
              std::cerr << "Failed to initialize JIT execution" << std::endl;
            }
          }
          
        } catch (const std::exception &e) {
          std::cerr << "CODE GENERATION ERROR: " << e.what() << std::endl;
          hadError = true;
        }
      }

    } catch (const std::exception &e) {
      std::cerr << "SEMANTIC ANALYSIS ERROR: " << e.what() << std::endl;
      hadError = true;
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
  std::cout << "  -l, --llvm        Enable LLVM code generation (requires -s)" << std::endl;
  std::cout << "  -c, --compile     Compile to executable (requires -l)" << std::endl;
  std::cout << "  -o, --object      Compile to object file (requires -l)" << std::endl;
  std::cout << "  -S, --assembly    Compile to assembly (requires -l)" << std::endl;
  std::cout << "  -O, --output FILE Specify output file name" << std::endl;
  std::cout << "  -v, --verbose     Enable verbose output" << std::endl;
  std::cout << "  --version         Show version information" << std::endl;
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
      enableSemanticAnalysis = true; // LLVM requires semantic analysis
    } else if (arg == "-c" || arg == "--compile") {
      compileToExecutable = true;
      enableLLVMCodeGen = true;
      enableSemanticAnalysis = true;
    } else if (arg == "-o" || arg == "--object") {
      compileToObject = true;
      enableLLVMCodeGen = true;
      enableSemanticAnalysis = true;
    } else if (arg == "-S" || arg == "--assembly") {
      compileToAssembly = true;
      enableLLVMCodeGen = true;
      enableSemanticAnalysis = true;
    } else if (arg == "-O" || arg == "--output") {
      if (i + 1 >= argc) {
        std::cerr << "Error: --output requires a filename" << std::endl;
        printUsage();
        return 64;
      }
      outputFile = argv[++i];
    } else if (arg == "-v" || arg == "--verbose") {
      verboseOutput = true;
    } else if (arg == "--version") {
      showVersion = true;
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

  // Handle version option
  if (showVersion) {
    printVersion();
    return 0;
  }

  // Validate option combinations
  if (enableLLVMCodeGen && !enableSemanticAnalysis) {
    std::cerr << "Error: LLVM code generation requires semantic analysis (-s)" << std::endl;
    return 64;
  }

  if ((compileToExecutable || compileToObject || compileToAssembly) && !enableLLVMCodeGen) {
    std::cerr << "Error: Compilation options require LLVM code generation (-l)" << std::endl;
    return 64;
  }

  // Check for conflicting compilation options
  int compilationModes = (compileToExecutable ? 1 : 0) + (compileToObject ? 1 : 0) + (compileToAssembly ? 1 : 0);
  if (compilationModes > 1) {
    std::cerr << "Error: Cannot specify multiple compilation modes" << std::endl;
    return 64;
  }

  if (!scriptFile.empty()) {
    runFile(scriptFile);
    // 如果处理文件时出现错误，返回非零退出码
    if (hadError) {
      if (verboseOutput) {
        std::cerr << "Compilation failed with errors." << std::endl;
      }
      return 1;
    }
  } else {
    std::string line;
    std::cout << "MiniSwift Interactive Mode";
    if (enableSemanticAnalysis) {
      std::cout << " (Semantic Analysis Enabled)";
    }
    if (enableLLVMCodeGen) {
      std::cout << " (LLVM Code Generation Enabled)";
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

      // 重置错误状态
      hadError = false;
      run(line);
    }
  }
  
  return hadError ? 1 : 0;
}