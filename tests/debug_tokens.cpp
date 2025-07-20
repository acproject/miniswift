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
        default: return "Other";
    }
}

int main() {
    std::string source = "let name = \"zhangli\"\nprint(\"Hello \\(name) !\")";
    std::cout << "Source code: " << source << std::endl << std::endl;
    
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    std::cout << "Tokens:" << std::endl;
    for (const auto& token : tokens) {
        std::cout << tokenTypeToString(token.type) << ": '" << token.lexeme << "'" << std::endl;
    }
    
    return 0;
}