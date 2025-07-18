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
        default: return "Other(" + std::to_string(static_cast<int>(type)) + ")";
    }
}

void debugStepByStep() {
    std::cout << "=== Step by step tokenization ===" << std::endl;
    std::string source = "\"Hello \\(name)\"";
    std::cout << "Source: " << source << std::endl;
    
    Lexer lexer(source);
    
    std::cout << "\nTokens generated:" << std::endl;
    std::cout << "Token Type          Lexeme              Line" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;
    
    int tokenCount = 0;
    while (true) {
        auto tokens = lexer.scanTokens();
        for (const auto& token : tokens) {
            std::cout << std::left << std::setw(20) << tokenTypeToString(token.type)
                      << std::setw(20) << ("\"" + token.lexeme + "\"")
                      << token.line << std::endl;
            if (token.type == TokenType::EndOfFile) {
                return;
            }
        }
        break;
    }
}

void testSimpleString() {
    std::cout << "\n=== Testing simple string (no interpolation) ===" << std::endl;
    std::string source = "\"Hello World\"";
    std::cout << "Source: " << source << std::endl;
    
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    std::cout << "\nTokens:" << std::endl;
    for (const auto& token : tokens) {
        std::cout << tokenTypeToString(token.type) << ": \"" << token.lexeme << "\"" << std::endl;
    }
}

void testInterpolationString() {
    std::cout << "\n=== Testing interpolation string ===" << std::endl;
    std::string source = "\"Hello \\(name)\"";
    std::cout << "Source: " << source << std::endl;
    
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    std::cout << "\nTokens:" << std::endl;
    for (const auto& token : tokens) {
        std::cout << tokenTypeToString(token.type) << ": \"" << token.lexeme << "\"" << std::endl;
    }
}

int main() {
    testSimpleString();
    testInterpolationString();
    debugStepByStep();
    return 0;
}