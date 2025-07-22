#include "src/lexer/Lexer.h"
#include "src/parser/Parser.h"
#include <iostream>

using namespace miniswift;

int main() {
    std::string source = "greet(\"Alice\", age: 25)";
    std::cout << "Testing: " << source << std::endl;
    
    try {
        Lexer lexer(source);
        auto tokens = lexer.scanTokens();
        
        std::cout << "Tokens generated successfully." << std::endl;
        
        Parser parser(tokens);
        auto statements = parser.parse();
        
        std::cout << "Parsing completed successfully!" << std::endl;
        std::cout << "Number of statements: " << statements.size() << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}