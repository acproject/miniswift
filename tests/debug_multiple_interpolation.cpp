#include "src/lexer/Lexer.h"
#include <iostream>

using namespace miniswift;

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Let: return "Let";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Equal: return "Equal";
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::InterpolatedStringLiteral: return "InterpolatedStringLiteral";
        case TokenType::InterpolationStart: return "InterpolationStart";
        case TokenType::InterpolationEnd: return "InterpolationEnd";
        case TokenType::Print: return "Print";
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::EndOfFile: return "EndOfFile";
        case TokenType::Unknown: return "Unknown";
        default: return "Other";
    }
}

int main() {
    std::string source = "print(\"\\(name) is \\(age) years old\")";
    std::cout << "Source: " << source << std::endl;
    
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    std::cout << "\nTokens:" << std::endl;
    for (const auto& token : tokens) {
        std::cout << tokenTypeToString(token.type) << ": '" << token.lexeme << "'" << std::endl;
    }
    
    return 0;
}