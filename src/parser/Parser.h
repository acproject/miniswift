#ifndef MINISWIFT_PARSER_H
#define MINISWIFT_PARSER_H

#include "../lexer/Token.h"
#include "AST.h"
#include <vector>
#include <memory>

namespace miniswift {

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<Stmt>> parse();

private:
    std::vector<std::unique_ptr<Stmt>> declaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> printStatement();
    std::unique_ptr<Stmt> expressionStatement();
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();

    bool match(const std::vector<TokenType>& types);
    Token advance();
    bool isAtEnd();
    Token peek();
    Token previous();
    void consume(TokenType type, const std::string& message);

    const std::vector<Token>& tokens;
    int current = 0;
};

} // namespace miniswift

#endif // MINISWIFT_PARSER_H