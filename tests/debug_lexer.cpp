#include "src/lexer/Lexer.h"
#include <iostream>

using namespace miniswift;

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::Identifier: return "Identifier";
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::Comma: return "Comma";
        case TokenType::Colon: return "Colon";
        case TokenType::IntegerLiteral: return "IntegerLiteral";
        case TokenType::EndOfFile: return "EndOfFile";
        case TokenType::Unknown: return "Unknown";
        default: return "Other";
    }
}

int main() {
    std::string source = "greet(\"Alice\", age: 25)";
    std::cout << "Source: " << source << std::endl << std::endl;
    
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    std::cout << "Tokens:" << std::endl;
    for (const auto& token : tokens) {
        std::cout << tokenTypeToString(token.type) << ": '" << token.lexeme << "'" << std::endl;
    }
    
    return 0;
}