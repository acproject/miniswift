#include "src/lexer/Lexer.h"
#include "src/lexer/Token.h"
#include <iostream>
#include <string>
#include <iomanip>

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
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::EndOfFile: return "EndOfFile";
        case TokenType::Unknown: return "Unknown";
        default: return "Other";
    }
}

void testSimpleInterpolation() {
    std::cout << "=== Testing: Simple string interpolation ===" << std::endl;
    std::string source = "let x = \"Hello \\(name)\"";
    std::cout << "Source: " << source << std::endl << std::endl;
    
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    std::cout << "Token Type          Lexeme              Line" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    
    for (const auto& token : tokens) {
        std::cout << std::left << std::setw(20) << tokenTypeToString(token.type)
                  << std::setw(20) << ("\"" + token.lexeme + "\"")
                  << token.line << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    testSimpleInterpolation();
    return 0;
}