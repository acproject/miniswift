#ifndef MINISWIFT_PARSER_H
#define MINISWIFT_PARSER_H

#include "../lexer/Token.h"
#include "AST.h"
#include "AccessControl.h"
#include <vector>
#include <memory>

namespace miniswift {

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    std::vector<std::unique_ptr<Stmt>> parse();

private:
    std::vector<std::unique_ptr<Stmt>> declaration();
    std::unique_ptr<Stmt> functionDeclaration();
    std::unique_ptr<Stmt> enumDeclaration();
    std::unique_ptr<Stmt> structDeclaration();
    std::unique_ptr<Stmt> classDeclaration();
    std::unique_ptr<Stmt> protocolDeclaration();
    std::unique_ptr<Stmt> extensionDeclaration();
    std::unique_ptr<Stmt> actorDeclaration();
    std::unique_ptr<Stmt> initDeclaration();
    std::unique_ptr<Stmt> deinitDeclaration();
    std::unique_ptr<Stmt> subscriptDeclaration();
    std::unique_ptr<Stmt> statement();
    std::unique_ptr<Stmt> printStatement();
    std::unique_ptr<Stmt> expressionStatement();
    std::unique_ptr<Stmt> blockStatement();
    std::unique_ptr<Stmt> ifStatement();
    std::unique_ptr<Stmt> whileStatement();
    std::unique_ptr<Stmt> forStatement();
    std::unique_ptr<Stmt> returnStatement();
    std::unique_ptr<Stmt> throwStatement();
    std::unique_ptr<Stmt> doCatchStatement();
    std::unique_ptr<Stmt> deferStatement();
    std::unique_ptr<Stmt> guardStatement();
    std::unique_ptr<Stmt> switchStatement();
    // Advanced operator parsing methods
    std::unique_ptr<Stmt> customOperatorDeclaration();
    std::unique_ptr<Stmt> operatorPrecedenceDeclaration();
    std::unique_ptr<Stmt> resultBuilderDeclaration();
    // Macro parsing methods
    std::unique_ptr<Stmt> macroDeclaration();
    std::unique_ptr<Stmt> externalMacroDeclaration();
    std::unique_ptr<Stmt> freestandingMacroDeclaration();
    std::unique_ptr<Stmt> attachedMacroDeclaration();
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> ternary();
    std::unique_ptr<Expr> logicalOr();
    std::unique_ptr<Expr> logicalAnd();
    std::unique_ptr<Expr> nilCoalescing();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> typeCasting();
    std::unique_ptr<Expr> typeChecking();
    std::unique_ptr<Expr> range();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    // Bitwise and overflow operator parsing methods
    std::unique_ptr<Expr> bitwiseOr();
    std::unique_ptr<Expr> bitwiseXor();
    std::unique_ptr<Expr> bitwiseAnd();
    std::unique_ptr<Expr> bitwiseShift();
    std::unique_ptr<Expr> overflowArithmetic();
    std::unique_ptr<Expr> customOperatorExpression();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> tryExpression();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee);
    std::unique_ptr<Expr> primary();
    std::unique_ptr<Expr> arrayLiteral();
    std::unique_ptr<Expr> dictionaryLiteral();
    std::unique_ptr<Expr> indexAccess(std::unique_ptr<Expr> object);
    std::unique_ptr<Expr> subscriptAccess(std::unique_ptr<Expr> object);
    std::unique_ptr<Expr> memberAccess(std::unique_ptr<Expr> object);
    std::unique_ptr<Expr> structInit();
    std::unique_ptr<Expr> closure();
    std::unique_ptr<Expr> resultTypeExpression();
    std::unique_ptr<Expr> errorLiteral();
    std::unique_ptr<Expr> awaitExpression();
    std::unique_ptr<Expr> taskExpression();

    // Generic parsing methods
    GenericParameterClause parseGenericParameterClause();
    TypeParameter parseTypeParameter();
    WhereClause parseWhereClause();
    TypeConstraint parseTypeConstraint();
    GenericType parseGenericType();
    GenericTypeInstantiation parseGenericTypeInstantiation();

    // Access control parsing methods
    AccessLevel parseAccessLevel();
    std::pair<AccessLevel, AccessLevel> parseAccessLevelWithSetter();
    bool isAccessLevelToken(TokenType type);

    bool match(const std::vector<TokenType>& types);
    bool check(TokenType type);
    Token advance();
    bool isAtEnd();
    Token peek();
    Token previous();
    void consume(TokenType type, const std::string& message);
    Token parseType();
    bool isCustomOperator(const std::string& lexeme);
    bool isCustomPrefixOperator(const std::string& lexeme);

    const std::vector<Token>& tokens;
    int current = 0;
};

} // namespace miniswift

#endif // MINISWIFT_PARSER_H