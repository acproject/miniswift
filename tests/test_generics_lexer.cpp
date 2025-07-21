#include "src/lexer/Lexer.h"
#include <iostream>

using namespace miniswift;

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Let: return "Let";
        case TokenType::Identifier: return "Identifier";
        case TokenType::Equal: return "Equal";
        case TokenType::LAngle: return "LAngle";
        case TokenType::RAngle: return "RAngle";
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::Colon: return "Colon";
        case TokenType::IntegerLiteral: return "IntegerLiteral";
        case TokenType::EndOfFile: return "EndOfFile";
        default: return "Other";
    }
}

int main() {
    std::string source = "let intContainer = Container<Int>(item: 42)";
    std::cout << "Source: " << source << std::endl << std::endl;
    
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    std::cout << "Tokens:" << std::endl;
    for (const auto& token : tokens) {
        std::cout << tokenTypeToString(token.type) << ": '" << token.lexeme << "'" << std::endl;
    }
    
    return 0;
}