#include "src/lexer/Lexer.h"
#include "src/parser/Parser.h"
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
        case TokenType::Let: return "Let";
        case TokenType::Equal: return "Equal";
        case TokenType::Semicolon: return "Semicolon";
        default: return "Other";
    }
}

void debugTokensAndParsing(const std::string& source) {
    std::cout << "\n=== Debugging: \"" << source << "\" ===\n";
    
    // Tokenize
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    
    std::cout << "\nTokens generated:\n";
    for (size_t i = 0; i < tokens.size(); ++i) {
        const auto& token = tokens[i];
        std::cout << "  " << i << ": " 
                  << std::setw(25) << std::left << tokenTypeToString(token.type)
                  << " '" << token.lexeme << "'\n";
    }
    
    // Try to parse
    std::cout << "\nParsing result:\n";
    try {
        Parser parser(tokens);
        auto statements = parser.parse();
        std::cout << "  SUCCESS: Parsed " << statements.size() << " statements\n";
    } catch (const std::exception& e) {
        std::cout << "  ERROR: " << e.what() << "\n";
    }
}

int main() {
    debugTokensAndParsing("let name = \"Swift\"");
    debugTokensAndParsing("let greeting = \"Hello \\(name)\"");
    
    return 0;
}