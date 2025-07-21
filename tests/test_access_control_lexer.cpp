#include "../src/lexer/Lexer.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace miniswift;

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Open: return "Open";
        case TokenType::Public: return "Public";
        case TokenType::Package: return "Package";
        case TokenType::Internal: return "Internal";
        case TokenType::Fileprivate: return "Fileprivate";
        case TokenType::Private: return "Private";
        case TokenType::Class: return "Class";
        case TokenType::Struct: return "Struct";
        case TokenType::Func: return "Func";
        case TokenType::Var: return "Var";
        case TokenType::Identifier: return "Identifier";
        case TokenType::LBrace: return "LBrace";
        case TokenType::RBrace: return "RBrace";
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::Equal: return "Equal";
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::EndOfFile: return "EndOfFile";
        default: return "Other";
    }
}

int main() {
    std::cout << "Testing Access Control Lexer...\n\n";
    
    // Test basic access control keywords
    std::string testCode = R"(
public class TestClass {
    private var privateVar = "test"
    internal func internalFunc() {}
    fileprivate var fileprivateVar = 42
    open func openFunc() {}
    package var packageVar = true
}
)";
    
    std::cout << "Test code:\n" << testCode << "\n\n";
    
    Lexer lexer(testCode);
    auto tokens = lexer.scanTokens();
    
    std::cout << "Tokens found:\n";
    for (const auto& token : tokens) {
        std::cout << "Type: " << tokenTypeToString(token.type) 
                  << ", Lexeme: '" << token.lexeme 
                  << "', Line: " << token.line << std::endl;
        
        if (token.type == TokenType::EndOfFile) {
            break;
        }
    }
    
    // Test specific access control keywords
    std::cout << "\n\nTesting individual access control keywords:\n";
    std::vector<std::string> keywords = {
        "open", "public", "package", "internal", "fileprivate", "private"
    };
    
    for (const auto& keyword : keywords) {
        Lexer keywordLexer(keyword);
        auto keywordTokens = keywordLexer.scanTokens();
        if (!keywordTokens.empty()) {
            std::cout << "Keyword '" << keyword << "' -> Token: " 
                      << tokenTypeToString(keywordTokens[0].type) << std::endl;
        }
    }
    
    std::cout << "\nAccess Control Lexer test completed!\n";
    return 0;
}