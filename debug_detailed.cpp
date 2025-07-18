#include "src/lexer/Lexer.h"
#include <iostream>
#include <iomanip>

using namespace miniswift;

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::InterpolatedStringLiteral: return "InterpolatedStringLiteral";
        case TokenType::InterpolationStart: return "InterpolationStart";
        case TokenType::InterpolationEnd: return "InterpolationEnd";
        case TokenType::Identifier: return "Identifier";
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::Unknown: return "Unknown";
        case TokenType::EndOfFile: return "EndOfFile";
        default: return "Other";
    }
}

class DebugLexer : public Lexer {
public:
    DebugLexer(const std::string& source) : Lexer(source) {}
    
    void debugTokenization(const std::string& input) {
        std::cout << "\n=== Debugging tokenization of: \"" << input << "\" ===\n";
        
        // Create a new lexer for this input
        Lexer lexer(input);
        auto tokens = lexer.scanTokens();
        
        std::cout << "Total tokens generated: " << tokens.size() << "\n\n";
        
        for (size_t i = 0; i < tokens.size(); ++i) {
            const auto& token = tokens[i];
            std::cout << "Token " << i << ": " 
                      << std::setw(25) << std::left << tokenTypeToString(token.type)
                      << " Lexeme: '" << token.lexeme << "'\n";
        }
    }
};

int main() {
    DebugLexer debugger("");
    
    // Test cases
    debugger.debugTokenization("\"Hello World\"");
    debugger.debugTokenization("\"Hello \\(name)\"");
    debugger.debugTokenization("\"Start \\(var) End\"");
    
    return 0;
}