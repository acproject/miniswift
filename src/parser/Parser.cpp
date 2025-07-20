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
  if (match({TokenType::Enum})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(enumDeclaration());
    return result;
  }
  if (match({TokenType::Struct})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(structDeclaration());
    return result;
  }
  if (match({TokenType::Class})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(classDeclaration());
    return result;
  }
  if (match({TokenType::Init})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(initDeclaration());
    return result;
  }
  if (match({TokenType::Deinit})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(deinitDeclaration());
    return result;
  }
  if (match({TokenType::Subscript})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(subscriptDeclaration());
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

    // Check if the left side is a valid assignment target
    if (dynamic_cast<VarExpr *>(expr.get()) || 
        dynamic_cast<MemberAccess *>(expr.get()) ||
        dynamic_cast<IndexAccess *>(expr.get()) ||
        dynamic_cast<SubscriptAccess *>(expr.get())) {
      return std::make_unique<Assign>(std::move(expr), std::move(value));
    }

    throw std::runtime_error("Invalid assignment target.");
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
  
  // Handle 'self' keyword as a special identifier
  if (match({TokenType::Self})) {
    Token selfToken = previous();
    return std::make_unique<VarExpr>(selfToken);
  }

  // Handle 'super' keyword
  if (match({TokenType::Super})) {
    Token keyword = previous();
    consume(TokenType::Dot, "Expect '.' after 'super'.");
    consume(TokenType::Identifier, "Expect superclass method name.");
    Token method = previous();
    return std::make_unique<Super>(keyword, method);
  }

  // Handle string interpolation - can start with either InterpolatedStringLiteral or InterpolationStart
  if (match({TokenType::InterpolatedStringLiteral, TokenType::InterpolationStart})) {
    std::vector<StringInterpolation::InterpolationPart> parts;
    
    // Check what type of token we matched
    if (previous().type == TokenType::InterpolatedStringLiteral) {
      // String starts with text
      std::string initialText = previous().lexeme;
      if (!initialText.empty()) {
        parts.emplace_back(initialText);
      }
    } else {
      // String starts with interpolation - put the token back
      current--;
    }
    
    // Parse the complete interpolation sequence
    while (true) {
      // Look for interpolation start
      if (match({TokenType::InterpolationStart})) {
        auto expr = expression();
        consume(TokenType::InterpolationEnd, "Expect ')' after interpolation expression.");
        parts.emplace_back(std::move(expr));
        
        // After interpolation, check for more string content
        if (match({TokenType::InterpolatedStringLiteral, TokenType::StringLiteral})) {
          std::string nextText = previous().lexeme;
          if (!nextText.empty()) {
            parts.emplace_back(nextText);
          }
          // Continue the loop to look for more interpolations
        } else {
          // Check if there's another interpolation directly following
          if (check(TokenType::InterpolationStart)) {
            // Continue the loop for consecutive interpolations
            continue;
          } else {
            // No more string content or interpolations, we're done
            break;
          }
        }
      } else {
        // No more interpolations, we're done
        break;
      }
    }
    
    return std::make_unique<StringInterpolation>(std::move(parts));
  }

  if (match({TokenType::InterpolationStart})) {
    // Handle standalone interpolation (shouldn't normally happen)
    auto expr = expression();
    consume(TokenType::InterpolationEnd, "Expect ')' after interpolation expression.");
    return expr;
  }

  // Array or dictionary literal
  if (match({TokenType::LSquare})) {
    return arrayLiteral();
  }

  if (match({TokenType::Identifier})) {
    Token identifier = previous();
    
    std::unique_ptr<Expr> expr = std::make_unique<VarExpr>(identifier);
    
    // Check for index/subscript access: identifier[index] or identifier[arg1, arg2, ...]
    while (check(TokenType::LSquare)) {
      // Check if this is a multi-parameter subscript by looking ahead
      int savedCurrent = current;
      bool isMultiParam = false;
      
      advance(); // consume '['
      
      // Look for comma to detect multi-parameter subscript
      int depth = 1;
      while (depth > 0 && !isAtEnd()) {
        if (peek().type == TokenType::LSquare) depth++;
        else if (peek().type == TokenType::RSquare) depth--;
        else if (peek().type == TokenType::Comma && depth == 1) {
          isMultiParam = true;
          break;
        }
        advance();
      }
      
      // Restore position and consume '[' for the access methods
      current = savedCurrent;
      advance(); // consume '[' for indexAccess/subscriptAccess
      
      if (isMultiParam) {
        expr = subscriptAccess(std::move(expr));
      } else {
        expr = indexAccess(std::move(expr));
      }
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

// Parse subscript access: object[arg1, arg2, ...]
std::unique_ptr<Expr> Parser::subscriptAccess(std::unique_ptr<Expr> object) {
  // We've already consumed the '['
  std::vector<std::unique_ptr<Expr>> arguments;
  
  if (!check(TokenType::RSquare)) {
    do {
      arguments.push_back(expression());
    } while (match({TokenType::Comma}));
  }
  
  consume(TokenType::RSquare, "Expect ']' after subscript arguments.");
  return std::make_unique<SubscriptAccess>(std::move(object), std::move(arguments));
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
    } else if (check(TokenType::LSquare)) {
      // Check if this is a multi-parameter subscript by looking ahead
      int savedCurrent = current;
      bool isMultiParam = false;
      
      advance(); // consume '['
      
      // Look for comma to detect multi-parameter subscript
      int depth = 1;
      while (depth > 0 && !isAtEnd()) {
        if (peek().type == TokenType::LSquare) depth++;
        else if (peek().type == TokenType::RSquare) depth--;
        else if (peek().type == TokenType::Comma && depth == 1) {
          isMultiParam = true;
          break;
        }
        advance();
      }
      
      // Restore position and consume '[' for the access methods
      current = savedCurrent;
      advance(); // consume '[' for indexAccess/subscriptAccess
      
      if (isMultiParam) {
        expr = subscriptAccess(std::move(expr));
      } else {
        expr = indexAccess(std::move(expr));
      }
    } else if (match({TokenType::Dot})) {
      consume(TokenType::Identifier, "Expect member name after '.'.");
      Token memberName = previous();
      
      // Check if this is an enum access with arguments
      if (match({TokenType::LParen})) {
        // Enum member access with associated values: EnumType.caseName(arg1, arg2)
        std::vector<std::unique_ptr<Expr>> arguments;
        if (!check(TokenType::RParen)) {
          do {
            arguments.push_back(expression());
          } while (match({TokenType::Comma}));
        }
        consume(TokenType::RParen, "Expect ')' after enum case arguments.");
        expr = std::make_unique<EnumAccess>(std::move(expr), memberName, std::move(arguments));
      } else {
        // Regular member access: object.member
        expr = std::make_unique<MemberAccess>(std::move(expr), memberName);
      }
    } else {
      break;
    }
  }
  
  return expr;
}

// Parse function call arguments
std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) {
  // Check if this might be a struct initialization by looking ahead
  // Struct init has pattern: StructName(memberName: value, ...)
  // Function call has pattern: functionName(arg1, arg2, ...)
  
  if (!check(TokenType::RParen)) {
    // Look ahead to see if the first argument has the pattern "identifier:"
    int savedCurrent = current;
    bool isStructInit = false;
    
    if (check(TokenType::Identifier)) {
      advance(); // consume identifier
      if (check(TokenType::Colon)) {
        isStructInit = true;
      }
    }
    
    // Restore position
    current = savedCurrent;
    
    if (isStructInit) {
      // Parse as struct initialization
      std::vector<std::pair<Token, std::unique_ptr<Expr>>> members;
      
      do {
        consume(TokenType::Identifier, "Expect member name.");
        Token memberName = previous();
        
        consume(TokenType::Colon, "Expect ':' after member name.");
        auto value = expression();
        
        members.emplace_back(memberName, std::move(value));
      } while (match({TokenType::Comma}));
      
      consume(TokenType::RParen, "Expect ')' after struct members.");
      
      // Extract struct name from callee (should be a VarExpr)
      if (auto varExpr = dynamic_cast<VarExpr*>(callee.get())) {
        Token structName = varExpr->name;
        return std::make_unique<StructInit>(structName, std::move(members));
      } else {
        throw std::runtime_error("Invalid struct initialization.");
      }
    }
  }
  
  // Parse as regular function call
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

// Parse enum declaration: enum Name: RawType { case1, case2(AssociatedType), case3 = rawValue }
std::unique_ptr<Stmt> Parser::enumDeclaration() {
  consume(TokenType::Identifier, "Expect enum name.");
  Token name = previous();
  
  Token rawType = Token(TokenType::Nil, "", name.line);
  if (match({TokenType::Colon})) {
    rawType = parseType();
  }
  
  consume(TokenType::LBrace, "Expect '{' after enum name.");
  
  std::vector<EnumCase> cases;
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts;
  
  while (!check(TokenType::RBrace) && !isAtEnd()) {
    if (match({TokenType::Case})) {
      // Parse enum cases
      do {
        consume(TokenType::Identifier, "Expect case name.");
        Token caseName = previous();
        
        std::vector<Token> associatedTypes;
        std::unique_ptr<Expr> rawValue = nullptr;
        
        // Check for associated values: case name(Type1, Type2)
        if (match({TokenType::LParen})) {
          if (!check(TokenType::RParen)) {
            do {
              Token associatedType = parseType();
              associatedTypes.push_back(associatedType);
            } while (match({TokenType::Comma}));
          }
          consume(TokenType::RParen, "Expect ')' after associated types.");
        }
        // Check for raw value: case name = value
        else if (match({TokenType::Equal})) {
          rawValue = expression();
        }
        
        cases.emplace_back(caseName, std::move(associatedTypes), std::move(rawValue));
      } while (match({TokenType::Comma}));
      
      // Optional semicolon or newline
      match({TokenType::Semicolon});
    } else if (match({TokenType::Static})) {
      // Parse static members (like static subscript)
      if (match({TokenType::Subscript})) {
        // Parse static subscript declaration
        auto subscript = std::unique_ptr<SubscriptStmt>(static_cast<SubscriptStmt*>(subscriptDeclaration().release()));
        subscripts.push_back(std::move(subscript));
      } else {
        throw std::runtime_error("Only static subscripts are currently supported in enums.");
      }
    } else {
      throw std::runtime_error("Expect 'case' or 'static' in enum declaration.");
    }
  }
  
  consume(TokenType::RBrace, "Expect '}' after enum cases.");
  return std::make_unique<EnumStmt>(name, rawType, std::move(cases), std::move(subscripts));
}

// Parse struct declaration: struct Name { var member1: Type, let member2: Type = defaultValue }
std::unique_ptr<Stmt> Parser::structDeclaration() {
  consume(TokenType::Identifier, "Expect struct name.");
  Token name = previous();
  
  std::cout << "Parsing struct: " << name.lexeme << std::endl;
  
  consume(TokenType::LBrace, "Expect '{' after struct name.");
  
  std::vector<StructMember> members;
  std::vector<std::unique_ptr<FunctionStmt>> methods;
  std::vector<std::unique_ptr<InitStmt>> initializers;
  std::unique_ptr<DeinitStmt> deinitializer;
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts;
  
  while (!check(TokenType::RBrace) && !isAtEnd()) {
    if (match({TokenType::Func})) {
      // Parse method declaration
      auto method = std::unique_ptr<FunctionStmt>(static_cast<FunctionStmt*>(functionDeclaration().release()));
      methods.push_back(std::move(method));
    } else if (match({TokenType::Init})) {
      // Parse initializer declaration
      auto init = std::unique_ptr<InitStmt>(static_cast<InitStmt*>(initDeclaration().release()));
      initializers.push_back(std::move(init));
    } else if (match({TokenType::Deinit})) {
      // Parse deinitializer declaration
      if (deinitializer) {
        throw std::runtime_error("Multiple deinitializers not allowed.");
      }
      deinitializer = std::unique_ptr<DeinitStmt>(static_cast<DeinitStmt*>(deinitDeclaration().release()));
    } else if (match({TokenType::Subscript})) {
      // Parse subscript declaration
      auto subscript = std::unique_ptr<SubscriptStmt>(static_cast<SubscriptStmt*>(subscriptDeclaration().release()));
      subscripts.push_back(std::move(subscript));
    } else if (match({TokenType::Var, TokenType::Let})) {
      // Parse member declaration
      bool isVar = previous().type == TokenType::Var;
      
      consume(TokenType::Identifier, "Expect member name.");
      Token memberName = previous();
      
      consume(TokenType::Colon, "Expect ':' after member name.");
      Token memberType = parseType();
      
      std::unique_ptr<Expr> defaultValue = nullptr;
      std::vector<PropertyAccessor> accessors;
      
      if (match({TokenType::Equal})) {
        defaultValue = expression();
      } else if (match({TokenType::LBrace})) {
        // Parse property accessors
        while (!check(TokenType::RBrace) && !isAtEnd()) {
          if (match({TokenType::Get})) {
            consume(TokenType::LBrace, "Expect '{' after 'get'.");
            auto body = blockStatement();
            accessors.emplace_back(AccessorType::GET, std::move(body));
          } else if (match({TokenType::Set})) {
            std::string paramName = "newValue";
            if (match({TokenType::LParen})) {
              consume(TokenType::Identifier, "Expect parameter name.");
              paramName = previous().lexeme;
              consume(TokenType::RParen, "Expect ')' after parameter name.");
            }
            consume(TokenType::LBrace, "Expect '{' after 'set'.");
            auto body = blockStatement();
            accessors.emplace_back(AccessorType::SET, std::move(body), paramName);
          } else if (match({TokenType::WillSet})) {
            std::string paramName = "newValue";
            if (match({TokenType::LParen})) {
              consume(TokenType::Identifier, "Expect parameter name.");
              paramName = previous().lexeme;
              consume(TokenType::RParen, "Expect ')' after parameter name.");
            }
            consume(TokenType::LBrace, "Expect '{' after 'willSet'.");
            auto body = blockStatement();
            accessors.emplace_back(AccessorType::WILL_SET, std::move(body), paramName);
          } else if (match({TokenType::DidSet})) {
            std::string paramName = "oldValue";
            if (match({TokenType::LParen})) {
              consume(TokenType::Identifier, "Expect parameter name.");
              paramName = previous().lexeme;
              consume(TokenType::RParen, "Expect ')' after parameter name.");
            }
            consume(TokenType::LBrace, "Expect '{' after 'didSet'.");
            auto body = blockStatement();
            accessors.emplace_back(AccessorType::DID_SET, std::move(body), paramName);
          } else {
            throw std::runtime_error("Expect 'get', 'set', 'willSet', or 'didSet' in property accessor.");
          }
        }
        consume(TokenType::RBrace, "Expect '}' after property accessors.");
      }
      
      StructMember member(memberName, memberType, std::move(defaultValue), isVar);
      member.accessors = std::move(accessors);
      members.push_back(std::move(member));
      
      match({TokenType::Semicolon}); // Optional semicolon
    } else {
      throw std::runtime_error("Expect 'var', 'let', 'func', 'init', 'deinit', or 'subscript' in struct body.");
    }
  }
  
  consume(TokenType::RBrace, "Expect '}' after struct body.");
  return std::make_unique<StructStmt>(name, std::move(members), std::move(methods), 
                                      std::move(initializers), std::move(deinitializer), std::move(subscripts));
}

// Parse class declaration: class Name: Superclass { var member1: Type, func method() {} }
std::unique_ptr<Stmt> Parser::classDeclaration() {
  consume(TokenType::Identifier, "Expect class name.");
  Token name = previous();
  
  Token superclass = Token(TokenType::Nil, "", name.line);
  if (match({TokenType::Colon})) {
    consume(TokenType::Identifier, "Expect superclass name.");
    superclass = previous();
  }
  
  consume(TokenType::LBrace, "Expect '{' after class name.");
  
  std::vector<StructMember> members;
  std::vector<std::unique_ptr<FunctionStmt>> methods;
  std::vector<std::unique_ptr<InitStmt>> initializers;
  std::unique_ptr<DeinitStmt> deinitializer;
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts;
  
  while (!check(TokenType::RBrace) && !isAtEnd()) {
    if (match({TokenType::Func})) {
      // Parse method declaration
      auto method = std::unique_ptr<FunctionStmt>(static_cast<FunctionStmt*>(functionDeclaration().release()));
      methods.push_back(std::move(method));
    } else if (match({TokenType::Init})) {
      // Parse initializer declaration
      auto init = std::unique_ptr<InitStmt>(static_cast<InitStmt*>(initDeclaration().release()));
      initializers.push_back(std::move(init));
    } else if (match({TokenType::Deinit})) {
      // Parse deinitializer declaration
      if (deinitializer) {
        throw std::runtime_error("Multiple deinitializers not allowed.");
      }
      deinitializer = std::unique_ptr<DeinitStmt>(static_cast<DeinitStmt*>(deinitDeclaration().release()));
    } else if (match({TokenType::Subscript})) {
      // Parse subscript declaration
      auto subscript = std::unique_ptr<SubscriptStmt>(static_cast<SubscriptStmt*>(subscriptDeclaration().release()));
      subscripts.push_back(std::move(subscript));
    } else if (match({TokenType::Var, TokenType::Let})) {
      // Parse member declaration
      bool isVar = previous().type == TokenType::Var;
      
      consume(TokenType::Identifier, "Expect member name.");
      Token memberName = previous();
      
      consume(TokenType::Colon, "Expect ':' after member name.");
      Token memberType = parseType();
      
      std::unique_ptr<Expr> defaultValue = nullptr;
      if (match({TokenType::Equal})) {
        defaultValue = expression();
      }
      
      members.emplace_back(memberName, memberType, std::move(defaultValue), isVar);
      
      match({TokenType::Semicolon}); // Optional semicolon
    } else {
      throw std::runtime_error("Expect 'var', 'let', 'func', 'init', 'deinit', or 'subscript' in class body.");
    }
  }
  
  consume(TokenType::RBrace, "Expect '}' after class body.");
  return std::make_unique<ClassStmt>(name, superclass, std::move(members), std::move(methods),
                                     std::move(initializers), std::move(deinitializer), std::move(subscripts));
}

// Parse member access: object.member
std::unique_ptr<Expr> Parser::memberAccess(std::unique_ptr<Expr> object) {
  consume(TokenType::Identifier, "Expect member name after '.'.");
  Token member = previous();
  return std::make_unique<MemberAccess>(std::move(object), member);
}

// Parse struct initialization: StructName(member1: value1, member2: value2)
std::unique_ptr<Expr> Parser::structInit() {
  Token structName = previous(); // We've already consumed the identifier
  
  consume(TokenType::LParen, "Expect '(' after struct name.");
  
  std::vector<std::pair<Token, std::unique_ptr<Expr>>> members;
  
  if (!check(TokenType::RParen)) {
    do {
      consume(TokenType::Identifier, "Expect member name.");
      Token memberName = previous();
      
      consume(TokenType::Colon, "Expect ':' after member name.");
      auto value = expression();
      
      members.emplace_back(memberName, std::move(value));
    } while (match({TokenType::Comma}));
  }
  
  consume(TokenType::RParen, "Expect ')' after struct members.");
  return std::make_unique<StructInit>(structName, std::move(members));
}

// Parse init declaration: init(parameters) { body } or init?(parameters) { body }
std::unique_ptr<Stmt> Parser::initDeclaration() {
  // Check for failable initializer
  bool isFailable = false;
  if (match({TokenType::Unknown}) && previous().lexeme == "?") {
    isFailable = true;
  }
  
  consume(TokenType::LParen, "Expect '(' after 'init'.");
  
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
  
  consume(TokenType::LBrace, "Expect '{' before initializer body.");
  auto body = blockStatement();
  
  InitType initType = isFailable ? InitType::FAILABLE : InitType::DESIGNATED;
  
  return std::make_unique<InitStmt>(initType, std::move(parameters), std::move(body), false);
}

// Parse deinit declaration: deinit { body }
std::unique_ptr<Stmt> Parser::deinitDeclaration() {
  consume(TokenType::LBrace, "Expect '{' before deinitializer body.");
  auto body = blockStatement();
  
  return std::make_unique<DeinitStmt>(std::move(body));
}

// Parse subscript declaration: subscript(parameters) -> ReturnType { get { } set { } }
std::unique_ptr<Stmt> Parser::subscriptDeclaration() {
  bool isStatic = false;
  // Check if this is a static subscript (would be handled by caller)
  
  consume(TokenType::LParen, "Expect '(' after 'subscript'.");
  
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
  consume(TokenType::Arrow, "Expect '->' after subscript parameters.");
  Token returnType = parseType();
  
  consume(TokenType::LBrace, "Expect '{' before subscript body.");
  
  std::vector<PropertyAccessor> accessors;
  
  while (!check(TokenType::RBrace) && !isAtEnd()) {
    if (match({TokenType::Get})) {
      consume(TokenType::LBrace, "Expect '{' after 'get'.");
      auto body = blockStatement();
      accessors.emplace_back(AccessorType::GET, std::move(body));
    } else if (match({TokenType::Set})) {
      std::string paramName = "newValue";
      if (match({TokenType::LParen})) {
        consume(TokenType::Identifier, "Expect parameter name.");
        paramName = previous().lexeme;
        consume(TokenType::RParen, "Expect ')' after parameter name.");
      }
      consume(TokenType::LBrace, "Expect '{' after 'set'.");
      auto body = blockStatement();
      accessors.emplace_back(AccessorType::SET, std::move(body), paramName);
    } else {
      throw std::runtime_error("Expect 'get' or 'set' in subscript accessor.");
    }
  }
  
  consume(TokenType::RBrace, "Expect '}' after subscript body.");
  
  return std::make_unique<SubscriptStmt>(std::move(parameters), returnType, std::move(accessors), isStatic);
}

} // namespace miniswift