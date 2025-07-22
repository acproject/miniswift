#include <iostream>
#include <memory>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "interpreter/Interpreter.h"

using namespace miniswift;

int main() {
    try {
        // Test basic Task parsing
        std::string code = R"(
            Task {
                print("Hello from task")
            }
        )";
        
        std::cout << "Testing Task parsing..." << std::endl;
        
        Lexer lexer(code);
        auto tokens = lexer.scanTokens();
        
        std::cout << "Tokens scanned: " << tokens.size() << std::endl;
        
        Parser parser(tokens);
        auto statements = parser.parse();
        
        std::cout << "Statements parsed: " << statements.size() << std::endl;
        
        // Test basic await parsing
        std::string awaitCode = R"(
            let result = await someAsyncFunction()
        )";
        
        std::cout << "Testing await parsing..." << std::endl;
        
        Lexer awaitLexer(awaitCode);
        auto awaitTokens = awaitLexer.scanTokens();
        
        std::cout << "Await tokens scanned: " << awaitTokens.size() << std::endl;
        
        Parser awaitParser(awaitTokens);
        auto awaitStatements = awaitParser.parse();
        
        std::cout << "Await statements parsed: " << awaitStatements.size() << std::endl;
        
        std::cout << "Concurrency parsing tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}