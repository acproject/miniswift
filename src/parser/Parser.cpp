#include "Parser.h"
#include <stdexcept>
#include <iostream>

namespace miniswift {

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!isAtEnd()) {
    auto decls = declaration();
    statements.insert(statements.end(), std::make_move_iterator(decls.begin()),
                      std::make_move_iterator(decls.end()));
  }
  return statements;
}

std::vector<std::unique_ptr<Stmt>> Parser::declaration() {
  if (match({TokenType::Func})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(functionDeclaration());
    return result;
  }
  if (match({TokenType::Var, TokenType::Let})) {
    bool isConst = previous().type == TokenType::Let;
    std::vector<Token> names;
    std::vector<std::unique_ptr<Stmt>> statements;

    do {
      consume(TokenType::Identifier, "Expect variable name.");
      names.push_back(previous());
    } while (match({TokenType::Comma}));

    Token type = Token(TokenType::Nil, "", 0);
    std::unique_ptr<Expr> initializer = nullptr;

    if (match({TokenType::Colon})) {
      type = parseType();
    }

    if (match({TokenType::Equal})) {
      initializer = expression();
    }

    if (names.size() == 1) {
      // For single variable declaration, use the original initializer
      statements.push_back(std::make_unique<VarStmt>(
          names[0], std::move(initializer), isConst, type));
    } else {
      // For multiple variable declarations, clone the initializer
      for (const auto &name : names) {
        statements.push_back(std::make_unique<VarStmt>(
            name, initializer ? initializer->clone() : nullptr, isConst, type));
      }
    }

    match({TokenType::Semicolon}); // Optional semicolon
    return statements;
  }

  std::vector<std::unique_ptr<Stmt>> result;
  result.push_back(statement());
  return result;
}

std::unique_ptr<Stmt> Parser::statement() {
  if (peek().type == TokenType::Print) {
    advance(); // consume 'print'
    return printStatement();
  }
  if (match({TokenType::LBrace})) {
    return blockStatement();
  }
  if (match({TokenType::If})) {
    return ifStatement();
  }
  if (match({TokenType::While})) {
    return whileStatement();
  }
  if (match({TokenType::For})) {
    return forStatement();
  }
  if (match({TokenType::Return})) {
    return returnStatement();
  }
  return expressionStatement();
}

std::unique_ptr<Stmt> Parser::printStatement() {
  consume(TokenType::LParen, "Expect '(' after 'print'.");
  auto value = expression();
  consume(TokenType::RParen, "Expect ')' after expression.");
  match({TokenType::Semicolon}); // Optional semicolon
  return std::make_unique<PrintStmt>(std::move(value));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
  auto expr = expression();
  match({TokenType::Semicolon}); // Optional semicolon
  return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Expr> Parser::expression() { return assignment(); }

std::unique_ptr<Expr> Parser::assignment() {
  auto expr = equality();

  if (match({TokenType::Equal})) {
    Token equals = previous();
    auto value = assignment();

    if (auto *var = dynamic_cast<VarExpr *>(expr.get())) {
      Token name = var->name;
      return std::make_unique<Assign>(name, std::move(value));
    }

    // In a real compiler, you'd report an error.
  }

  return expr;
}

std::unique_ptr<Expr> Parser::equality() {
  auto expr = comparison();
  while (match({TokenType::BangEqual, TokenType::EqualEqual})) {
    Token op = previous();
    auto right = comparison();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::comparison() {
  auto expr = term();
  while (match({TokenType::Greater, TokenType::GreaterEqual, TokenType::Less,
                TokenType::LessEqual})) {
    Token op = previous();
    auto right = term();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::term() {
  auto expr = factor();
  while (match({TokenType::Minus, TokenType::Plus})) {
    Token op = previous();
    auto right = factor();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  auto expr = unary();
  while (match({TokenType::Slash, TokenType::Star})) {
    Token op = previous();
    auto right = unary();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  if (match({TokenType::Bang, TokenType::Minus})) {
    Token op = previous();
    auto right = unary();
    return std::make_unique<Unary>(op, std::move(right));
  }
  return call();
}

std::unique_ptr<Expr> Parser::primary() {
  if (match({TokenType::True, TokenType::False, TokenType::Nil,
             TokenType::IntegerLiteral, TokenType::FloatingLiteral,
             TokenType::StringLiteral})) {
    return std::make_unique<Literal>(previous());
  }

  // Handle string interpolation
  if (match({TokenType::InterpolatedStringLiteral})) {
    // For now, treat interpolated string literals as regular string literals
    // TODO: Implement proper string interpolation support
    return std::make_unique<Literal>(previous());
  }

  if (match({TokenType::InterpolationStart})) {
    // Skip interpolation for now - this is a simplified implementation
    // In a full implementation, we would parse the expression inside the interpolation
    auto expr = expression();
    consume(TokenType::RParen, "Expect ')' after interpolation expression.");
    // For now, just return the expression (this won't work properly with mixed string/interpolation)
    return expr;
  }

  // Array or dictionary literal
  if (match({TokenType::LSquare})) {
    return arrayLiteral();
  }

  // Closure literal: { (parameters) -> ReturnType in body }
  if (match({TokenType::LBrace})) {
    return closure();
  }

  if (match({TokenType::Identifier})) {
    std::unique_ptr<Expr> expr = std::make_unique<VarExpr>(previous());
    
    // Check for index access: identifier[index]
    while (match({TokenType::LSquare})) {
      expr = indexAccess(std::move(expr));
    }
    
    return expr;
  }

  if (match({TokenType::LParen})) {
    auto expr = expression();
    consume(TokenType::RParen, "Expect ')' after expression.");
    return std::make_unique<Grouping>(std::move(expr));
  }

  throw std::runtime_error("Expect expression.");
}

bool Parser::match(const std::vector<TokenType> &types) {
  for (TokenType type : types) {
    if (!isAtEnd() && peek().type == type) {
      advance();
      return true;
    }
  }
  return false;
}

Token Parser::advance() {
  if (!isAtEnd())
    current++;
  return previous();
}

bool Parser::isAtEnd() { return peek().type == TokenType::EndOfFile; }

Token Parser::peek() { return tokens[current]; }

Token Parser::previous() { return tokens[current - 1]; }

void Parser::consume(TokenType type, const std::string &message) {
  if (!isAtEnd() && peek().type == type) {
    advance();
    return;
  }
  throw std::runtime_error(message);
}

// Parse type annotations including collection types
Token Parser::parseType() {
  if (match({TokenType::LSquare})) {
    // Could be array type [ElementType] or dictionary type [KeyType: ValueType]
    Token firstType(TokenType::Identifier, "", 0);
    if (check(TokenType::LSquare)) {
      // Nested array type
      firstType = parseType();
    } else {
      consume(TokenType::Identifier, "Expect type name.");
      firstType = previous();
    }
    
    // Check if this is a dictionary type (has colon)
    if (match({TokenType::Colon})) {
      // Dictionary type: [KeyType: ValueType]
      Token valueType(TokenType::Identifier, "", 0);
      if (check(TokenType::LSquare)) {
        // Nested type for value
        valueType = parseType();
      } else {
        consume(TokenType::Identifier, "Expect value type name.");
        valueType = previous();
      }
      consume(TokenType::RSquare, "Expect ']' after dictionary type.");
      // Create a synthetic token for dictionary type
      return Token(TokenType::Identifier, "[" + firstType.lexeme + ":" + valueType.lexeme + "]", firstType.line);
    } else {
      // Array type: [ElementType]
      consume(TokenType::RSquare, "Expect ']' after array element type.");
      // Create a synthetic token for array type
      return Token(TokenType::Identifier, "[" + firstType.lexeme + "]", firstType.line);
    }
  }
  
  consume(TokenType::Identifier, "Expect type name.");
  return previous();
}

// Parse array literal: [1, 2, 3] or dictionary literal: ["key": value]
std::unique_ptr<Expr> Parser::arrayLiteral() {
  // We've already consumed the '['
  std::vector<std::unique_ptr<Expr>> elements;
  
  if (!check(TokenType::RSquare)) {
    do {
      auto expr = expression();
      
      // Check if this is a dictionary literal (key: value)
      if (match({TokenType::Colon})) {
        // This is a dictionary literal, parse as key-value pairs
        auto value = expression();
        
        std::vector<DictionaryLiteral::KeyValuePair> pairs;
        pairs.emplace_back(std::move(expr), std::move(value));
        
        // Parse remaining key-value pairs
        while (match({TokenType::Comma})) {
          if (check(TokenType::RSquare)) break; // Trailing comma
          auto key = expression();
          consume(TokenType::Colon, "Expect ':' after dictionary key.");
          auto val = expression();
          pairs.emplace_back(std::move(key), std::move(val));
        }
        
        consume(TokenType::RSquare, "Expect ']' after dictionary literal.");
        return std::make_unique<DictionaryLiteral>(std::move(pairs));
      }
      
      // This is an array literal
      elements.push_back(std::move(expr));
    } while (match({TokenType::Comma}));
  }
  
  consume(TokenType::RSquare, "Expect ']' after array literal.");
  return std::make_unique<ArrayLiteral>(std::move(elements));
}

// Parse dictionary literal (called when we know it's a dictionary)
std::unique_ptr<Expr> Parser::dictionaryLiteral() {
  // This method is for future use if we need separate dictionary parsing
  std::vector<DictionaryLiteral::KeyValuePair> pairs;
  
  if (!check(TokenType::RSquare)) {
    do {
      auto key = expression();
      consume(TokenType::Colon, "Expect ':' after dictionary key.");
      auto value = expression();
      pairs.emplace_back(std::move(key), std::move(value));
    } while (match({TokenType::Comma}));
  }
  
  consume(TokenType::RSquare, "Expect ']' after dictionary literal.");
  return std::make_unique<DictionaryLiteral>(std::move(pairs));
}

// Parse index access: object[index]
std::unique_ptr<Expr> Parser::indexAccess(std::unique_ptr<Expr> object) {
  // We've already consumed the '['
  auto index = expression();
  consume(TokenType::RSquare, "Expect ']' after index.");
  return std::make_unique<IndexAccess>(std::move(object), std::move(index));
}

// Helper method to check current token type without consuming
bool Parser::check(TokenType type) {
  if (isAtEnd()) return false;
  return peek().type == type;
}

// Parse block statement: { statements }
std::unique_ptr<Stmt> Parser::blockStatement() {
  std::vector<std::unique_ptr<Stmt>> statements;
  
  while (!check(TokenType::RBrace) && !isAtEnd()) {
    auto decls = declaration();
    statements.insert(statements.end(), std::make_move_iterator(decls.begin()),
                      std::make_move_iterator(decls.end()));
  }
  
  consume(TokenType::RBrace, "Expect '}' after block.");
  return std::make_unique<BlockStmt>(std::move(statements));
}

// Parse if statement: if condition { thenBranch } else { elseBranch }
std::unique_ptr<Stmt> Parser::ifStatement() {
  auto condition = expression();
  
  consume(TokenType::LBrace, "Expect '{' after if condition.");
  auto thenBranch = blockStatement();
  
  std::unique_ptr<Stmt> elseBranch = nullptr;
  if (match({TokenType::Else})) {
    if (match({TokenType::If})) {
      // else if
      elseBranch = ifStatement();
    } else {
      consume(TokenType::LBrace, "Expect '{' after else.");
      elseBranch = blockStatement();
    }
  }
  
  return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch), std::move(elseBranch));
}

// Parse while statement: while condition { body }
std::unique_ptr<Stmt> Parser::whileStatement() {
  auto condition = expression();
  
  consume(TokenType::LBrace, "Expect '{' after while condition.");
  auto body = blockStatement();
  
  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

// Parse for statement: for initializer; condition; increment { body }
std::unique_ptr<Stmt> Parser::forStatement() {
  consume(TokenType::LParen, "Expect '(' after 'for'.");
  
  // Initializer
  std::unique_ptr<Stmt> initializer = nullptr;
  if (match({TokenType::Semicolon})) {
    initializer = nullptr;
  } else if (match({TokenType::Var, TokenType::Let})) {
    // Variable declaration
    bool isConst = previous().type == TokenType::Let;
    consume(TokenType::Identifier, "Expect variable name.");
    Token name = previous();
    
    Token type = Token(TokenType::Nil, "", 0);
    std::unique_ptr<Expr> init = nullptr;
    
    if (match({TokenType::Colon})) {
      type = parseType();
    }
    
    if (match({TokenType::Equal})) {
      init = expression();
    }
    
    initializer = std::make_unique<VarStmt>(name, std::move(init), isConst, type);
    consume(TokenType::Semicolon, "Expect ';' after for loop initializer.");
  } else {
    initializer = expressionStatement();
  }
  
  // Condition
  std::unique_ptr<Expr> condition = nullptr;
  if (!check(TokenType::Semicolon)) {
    condition = expression();
  }
  consume(TokenType::Semicolon, "Expect ';' after for loop condition.");
  
  // Increment
  std::unique_ptr<Expr> increment = nullptr;
  if (!check(TokenType::RParen)) {
    increment = expression();
  }
  consume(TokenType::RParen, "Expect ')' after for clauses.");
  
  // Body
  consume(TokenType::LBrace, "Expect '{' after for clauses.");
  auto body = blockStatement();
  
  return std::make_unique<ForStmt>(std::move(initializer), std::move(condition), 
                                   std::move(increment), std::move(body));
}

// Parse function declaration: func name(parameters) -> ReturnType { body }
std::unique_ptr<Stmt> Parser::functionDeclaration() {
  consume(TokenType::Identifier, "Expect function name.");
  Token name = previous();
  
  consume(TokenType::LParen, "Expect '(' after function name.");
  
  std::vector<Parameter> parameters;
  if (!check(TokenType::RParen)) {
    do {
      consume(TokenType::Identifier, "Expect parameter name.");
      Token paramName = previous();
      consume(TokenType::Colon, "Expect ':' after parameter name.");
      Token paramType = parseType();
      parameters.emplace_back(paramName, paramType);
    } while (match({TokenType::Comma}));
  }
  
  consume(TokenType::RParen, "Expect ')' after parameters.");
  
  Token returnType = Token(TokenType::Identifier, "Void", name.line);
  if (match({TokenType::Arrow})) {
    returnType = parseType();
  }
  
  consume(TokenType::LBrace, "Expect '{' before function body.");
  auto body = blockStatement();
  
  return std::make_unique<FunctionStmt>(name, std::move(parameters), returnType, std::move(body));
}

// Parse return statement: return expression?
std::unique_ptr<Stmt> Parser::returnStatement() {
  Token keyword = previous();
  std::unique_ptr<Expr> value = nullptr;
  
  if (!check(TokenType::Semicolon) && !check(TokenType::RBrace)) {
    value = expression();
  }
  
  match({TokenType::Semicolon}); // Optional semicolon
  return std::make_unique<ReturnStmt>(std::move(value));
}

// Parse function call: primary ( arguments )
std::unique_ptr<Expr> Parser::call() {
  auto expr = primary();
  
  while (true) {
    if (match({TokenType::LParen})) {
      expr = finishCall(std::move(expr));
    } else if (match({TokenType::LSquare})) {
      expr = indexAccess(std::move(expr));
    } else {
      break;
    }
  }
  
  return expr;
}

// Parse function call arguments
std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) {
  std::vector<std::unique_ptr<Expr>> arguments;
  
  if (!check(TokenType::RParen)) {
    do {
      arguments.push_back(expression());
    } while (match({TokenType::Comma}));
  }
  
  consume(TokenType::RParen, "Expect ')' after arguments.");
  return std::make_unique<Call>(std::move(callee), std::move(arguments));
}

// Parse closure: { (parameters) -> ReturnType in body }
std::unique_ptr<Expr> Parser::closure() {
  // We've already consumed the '{'
  std::vector<Parameter> parameters;
  Token returnType = Token(TokenType::Identifier, "Void", peek().line);
  
  // Check if this starts with parameters: (param1: Type, param2: Type)
  if (match({TokenType::LParen})) {
    if (!check(TokenType::RParen)) {
      do {
        consume(TokenType::Identifier, "Expect parameter name.");
        Token paramName = previous();
        consume(TokenType::Colon, "Expect ':' after parameter name.");
        Token paramType = parseType();
        parameters.emplace_back(paramName, paramType);
      } while (match({TokenType::Comma}));
    }
    consume(TokenType::RParen, "Expect ')' after parameters.");
    
    // Optional return type: -> ReturnType
    if (match({TokenType::Arrow})) {
      returnType = parseType();
    }
    
    // Expect 'in' keyword
    consume(TokenType::In, "Expect 'in' after closure signature.");
  }
  
  // Parse closure body (statements until '}')
  std::vector<std::unique_ptr<Stmt>> body;
  while (!check(TokenType::RBrace) && !isAtEnd()) {
    body.push_back(statement());
  }
  
  consume(TokenType::RBrace, "Expect '}' after closure body.");
  return std::make_unique<Closure>(std::move(parameters), returnType, std::move(body));
}

} // namespace miniswift