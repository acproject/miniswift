#include <iostream>
#include "src/lexer/Lexer.h"

std::string tokenTypeToString(miniswift::TokenType type) {
    switch (type) {
        case miniswift::TokenType::Print: return "Print";
        case miniswift::TokenType::LParen: return "LParen";
        case miniswift::TokenType::RParen: return "RParen";
        case miniswift::TokenType::InterpolatedStringLiteral: return "InterpolatedStringLiteral";
        case miniswift::TokenType::StringLiteral: return "StringLiteral";
        case miniswift::TokenType::InterpolationStart: return "InterpolationStart";
        case miniswift::TokenType::InterpolationEnd: return "InterpolationEnd";
        case miniswift::TokenType::Identifier: return "Identifier";
        case miniswift::TokenType::Less: return "Less";
        case miniswift::TokenType::IntegerLiteral: return "IntegerLiteral";
        case miniswift::TokenType::EndOfFile: return "EndOfFile";
        default: return "Unknown";
    }
}

int main() {
    std::string source = "print(\"\\(name)\\(age)\");";
    
    std::cout << "Source: " << source << std::endl << std::endl;
    
    miniswift::Lexer lexer(source);
    std::vector<miniswift::Token> tokens = lexer.scanTokens();
    
    std::cout << "Tokens:" << std::endl;
    for (const auto& token : tokens) {
        std::cout << tokenTypeToString(token.type) << ": '" << token.lexeme << "'" << std::endl;
    }
    
    return 0;
}