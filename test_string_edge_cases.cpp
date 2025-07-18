#include "src/lexer/Lexer.h"
#include "src/parser/Parser.h"
#include "src/interpreter/Interpreter.h"
#include <iostream>

int main() {
    std::vector<std::string> testCases = {
        // Basic string tests
        "let msg = \"Hello\"; print(msg)",
        
        // Empty string
        "let empty = \"\"; print(empty)",
        
        // String with spaces
        "let spaces = \"  Hello World  \"; print(spaces)",
        
        // String with special characters
        "let special = \"Hello\\nWorld\"; print(special)",
        
        // String interpolation (if supported)
        "let name = \"Swift\"; let greeting = \"Hello \\(name)\"; print(greeting)",
        
        // Multiple string operations
        "let a = \"Hello\"; let b = \"World\"; print(a); print(b)"
    };
    
    std::cout << "=== Testing String Edge Cases ===" << std::endl;
    
    for (size_t i = 0; i < testCases.size(); ++i) {
        const auto& source = testCases[i];
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Source: " << source << std::endl;
        
        try {
            // Lexical analysis
            miniswift::Lexer lexer(source);
            std::vector<miniswift::Token> tokens = lexer.scanTokens();
            
            // Parsing
            miniswift::Parser parser(tokens);
            auto statements = parser.parse();
            
            // Interpretation
            miniswift::Interpreter interpreter;
            std::cout << "Output: ";
            interpreter.interpret(statements);
            
        } catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << std::endl;
        }
    }
    
    return 0;
}