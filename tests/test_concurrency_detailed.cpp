#include <iostream>
#include <memory>
#include "lexer/Lexer.h"
#include "parser/Parser.h"

using namespace miniswift;

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::Actor: return "Actor";
        case TokenType::Async: return "Async";
        case TokenType::Await: return "Await";
        case TokenType::Task: return "Task";
        case TokenType::Func: return "Func";
        case TokenType::Var: return "Var";
        case TokenType::Let: return "Let";
        case TokenType::Identifier: return "Identifier";
        case TokenType::LBrace: return "LBrace";
        case TokenType::RBrace: return "RBrace";
        case TokenType::LParen: return "LParen";
        case TokenType::RParen: return "RParen";
        case TokenType::StringLiteral: return "StringLiteral";
        case TokenType::IntegerLiteral: return "IntegerLiteral";
        case TokenType::FloatingLiteral: return "FloatingLiteral";
        case TokenType::Equal: return "Equal";
        case TokenType::Arrow: return "Arrow";
        case TokenType::Colon: return "Colon";
        case TokenType::EndOfFile: return "EndOfFile";
        default: return "Other";
    }
}

void testTokenization(const std::string& description, const std::string& code) {
    std::cout << "\n=== " << description << " ===" << std::endl;
    std::cout << "Code: " << code << std::endl;
    
    Lexer lexer(code);
    auto tokens = lexer.scanTokens();
    
    std::cout << "Tokens (" << tokens.size() << "):" << std::endl;
    for (const auto& token : tokens) {
        if (token.type != TokenType::EndOfFile) {
            std::cout << "  " << tokenTypeToString(token.type) << ": '" << token.lexeme << "'" << std::endl;
        }
    }
    
    // Test parsing
    try {
        Parser parser(tokens);
        auto statements = parser.parse();
        std::cout << "Parsing successful: " << statements.size() << " statements" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Parsing error: " << e.what() << std::endl;
    }
}

int main() {
    try {
        // Test 1: Actor keyword recognition
        testTokenization("Actor Declaration", "actor Counter { var value: Int = 0 }");
        
        // Test 2: Async function
        testTokenization("Async Function", "func fetchData() async -> String { return \"data\" }");
        
        // Test 3: Await expression
        testTokenization("Await Expression", "let result = await fetchData()");
        
        // Test 4: Task creation
        testTokenization("Task Creation", "Task { print(\"Hello\") }");
        
        // Test 5: Complex concurrency example
        std::string complexCode = R"(
            actor BankAccount {
                var balance: Double = 0.0
                
                func deposit(amount: Double) async {
                    balance += amount
                }
                
                func withdraw(amount: Double) async -> Bool {
                    if balance >= amount {
                        balance -= amount
                        return true
                    }
                    return false
                }
            }
            
            func testBankAccount() async {
                let account = BankAccount()
                await account.deposit(amount: 100.0)
                let success = await account.withdraw(amount: 50.0)
                print(success)
            }
            
            Task {
                await testBankAccount()
            }
        )";
        
        testTokenization("Complex Concurrency Example", complexCode);
        
        std::cout << "\n=== All Detailed Tests Completed Successfully! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}