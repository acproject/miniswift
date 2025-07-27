#include "Parser.h"
#include <iostream>
#include <optional>
#include <set>
#include <stdexcept>

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
  // Parse optional access level modifier
  AccessLevel accessLevel = AccessLevel::INTERNAL;
  AccessLevel setterAccessLevel = AccessLevel::INTERNAL;

  if (isAccessLevelToken(peek().type)) {
    auto accessPair = parseAccessLevelWithSetter();
    accessLevel = accessPair.first;
    setterAccessLevel = accessPair.second;
  }

  // Check for mutating keyword before func
  bool isMutating = false;
  if (match({TokenType::Mutating})) {
    isMutating = true;
  }

  if (match({TokenType::Func})) {
    std::vector<std::unique_ptr<Stmt>> result;
    auto func = functionDeclaration();
    // Set access level and mutating flag for function
    auto funcStmt = static_cast<FunctionStmt *>(func.get());
    funcStmt->accessLevel = accessLevel;
    funcStmt->isMutating = isMutating;
    result.push_back(std::move(func));
    return result;
  }

  // If we found mutating but no func, that's an error
  if (isMutating) {
    throw std::runtime_error("Expect 'func' after 'mutating'.");
  }
  if (match({TokenType::Enum})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(enumDeclaration());
    return result;
  }
  if (match({TokenType::Struct})) {
    std::vector<std::unique_ptr<Stmt>> result;
    auto structStmt = structDeclaration();
    // Set access level for struct
    static_cast<StructStmt *>(structStmt.get())->accessLevel = accessLevel;
    result.push_back(std::move(structStmt));
    return result;
  }
  if (match({TokenType::Class})) {
    std::vector<std::unique_ptr<Stmt>> result;
    auto classStmt = classDeclaration();
    // Set access level for class
    static_cast<ClassStmt *>(classStmt.get())->accessLevel = accessLevel;
    result.push_back(std::move(classStmt));
    return result;
  }
  if (match({TokenType::Protocol})) {
    std::vector<std::unique_ptr<Stmt>> result;
    auto protocolStmt = protocolDeclaration();
    // Set access level for protocol
    static_cast<ProtocolStmt *>(protocolStmt.get())->accessLevel = accessLevel;
    result.push_back(std::move(protocolStmt));
    return result;
  }
  if (match({TokenType::Extension})) {
    std::vector<std::unique_ptr<Stmt>> result;
    auto extensionStmt = extensionDeclaration();
    // Set access level for extension
    static_cast<ExtensionStmt *>(extensionStmt.get())->accessLevel =
        accessLevel;
    result.push_back(std::move(extensionStmt));
    return result;
  }
  if (match({TokenType::Actor})) {
    std::vector<std::unique_ptr<Stmt>> result;
    auto actorStmt = actorDeclaration();
    // Set access level for actor
    static_cast<ActorStmt *>(actorStmt.get())->accessLevel = accessLevel;
    result.push_back(std::move(actorStmt));
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
  // Advanced operator declarations
  if (match({TokenType::Operator})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(customOperatorDeclaration());
    return result;
  }
  if (match({TokenType::Infix, TokenType::Prefix, TokenType::Postfix})) {
    // Check if this is an operator function (prefix func) or operator
    // declaration (prefix operator)
    if (check(TokenType::Func)) {
      // This is an operator function declaration like "prefix func ±"
      advance(); // consume 'func'
      std::vector<std::unique_ptr<Stmt>> result;
      result.push_back(functionDeclaration());
      return result;
    } else {
      // This is an operator declaration like "prefix operator ±"
      std::vector<std::unique_ptr<Stmt>> result;
      result.push_back(customOperatorDeclaration());
      return result;
    }
  }
  if (match({TokenType::PrecedenceGroup})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(operatorPrecedenceDeclaration());
    return result;
  }
  // Result Builder declaration (with @resultBuilder attribute)
  if (match({TokenType::At})) {
    if (match({TokenType::ResultBuilder})) {
      std::vector<std::unique_ptr<Stmt>> result;
      result.push_back(resultBuilderDeclaration());
      return result;
    }
    if (match({TokenType::FreestandingMacro})) {
      std::vector<std::unique_ptr<Stmt>> result;
      result.push_back(freestandingMacroDeclaration());
      return result;
    }
    if (match({TokenType::AttachedMacro})) {
      std::vector<std::unique_ptr<Stmt>> result;
      result.push_back(attachedMacroDeclaration());
      return result;
    }
    // Handle @main attribute
    if (check(TokenType::Identifier) && peek().lexeme == "main") {
      advance(); // consume 'main'
      std::vector<std::unique_ptr<Stmt>> result;
      
      // Check what follows @main
      if (match({TokenType::Func})) {
        auto func = functionDeclaration();
        static_cast<FunctionStmt *>(func.get())->isMain = true;
        result.push_back(std::move(func));
      } else if (match({TokenType::Struct})) {
        auto structStmt = structDeclaration();
        static_cast<StructStmt *>(structStmt.get())->isMain = true;
        static_cast<StructStmt *>(structStmt.get())->accessLevel = accessLevel;
        result.push_back(std::move(structStmt));
      } else if (match({TokenType::Class})) {
        auto classStmt = classDeclaration();
        static_cast<ClassStmt *>(classStmt.get())->isMain = true;
        static_cast<ClassStmt *>(classStmt.get())->accessLevel = accessLevel;
        result.push_back(std::move(classStmt));
      } else {
        throw std::runtime_error("@main attribute can only be applied to functions, structs, or classes.");
      }
      return result;
    }
  }
  // Macro declarations
  if (match({TokenType::Macro})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(macroDeclaration());
    return result;
  }
  if (match({TokenType::ExternalMacro})) {
    std::vector<std::unique_ptr<Stmt>> result;
    result.push_back(externalMacroDeclaration());
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
      statements.push_back(
          std::make_unique<VarStmt>(names[0], std::move(initializer), isConst,
                                    type, accessLevel, setterAccessLevel));
    } else {
      // For multiple variable declarations, clone the initializer
      for (const auto &name : names) {
        statements.push_back(std::make_unique<VarStmt>(
            name, initializer ? initializer->clone() : nullptr, isConst, type,
            accessLevel, setterAccessLevel));
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
  if (match({TokenType::Continue})) {
    return continueStatement();
  }
  if (match({TokenType::Break})) {
    return breakStatement();
  }
  if (match({TokenType::Fallthrough})) {
    return fallthroughStatement();
  }
  if (match({TokenType::Throw})) {
    return throwStatement();
  }
  if (match({TokenType::Do})) {
    return doCatchStatement();
  }
  if (match({TokenType::Defer})) {
    return deferStatement();
  }
  if (match({TokenType::Guard})) {
    return guardStatement();
  }
  if (match({TokenType::Switch})) {
    return switchStatement();
  }
  return expressionStatement();
}

std::unique_ptr<Stmt> Parser::printStatement() {
  consume(TokenType::LParen, "Expect '(' after 'print'.");

  std::vector<std::unique_ptr<Expr>> expressions;

  // Parse first expression
  if (!check(TokenType::RParen)) {
    expressions.push_back(expression());

    // Parse additional expressions separated by commas
    while (match({TokenType::Comma})) {
      expressions.push_back(expression());
    }
  }

  consume(TokenType::RParen, "Expect ')' after expression.");
  match({TokenType::Semicolon}); // Optional semicolon
  return std::make_unique<PrintStmt>(std::move(expressions));
}

std::unique_ptr<Stmt> Parser::expressionStatement() {
  auto expr = expression();
  match({TokenType::Semicolon}); // Optional semicolon
  return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<Expr> Parser::expression() { return assignment(); }

std::unique_ptr<Expr> Parser::assignment() {
  auto expr = ternary();

  if (match({TokenType::Equal, TokenType::PlusEqual, TokenType::MinusEqual,
             TokenType::StarEqual, TokenType::SlashEqual,
             TokenType::PercentEqual})) {
    Token equals = previous();
    auto value = assignment();

    // Check if the left side is a valid assignment target
    if (dynamic_cast<VarExpr *>(expr.get()) ||
        dynamic_cast<MemberAccess *>(expr.get()) ||
        dynamic_cast<IndexAccess *>(expr.get()) ||
        dynamic_cast<SubscriptAccess *>(expr.get())) {

      // For compound assignment operators, create equivalent binary operation
      if (equals.type != TokenType::Equal) {
        Token binaryOp = equals;
        // Convert compound assignment to binary operator
        switch (equals.type) {
        case TokenType::PlusEqual:
          binaryOp.type = TokenType::Plus;
          binaryOp.lexeme = "+";
          break;
        case TokenType::MinusEqual:
          binaryOp.type = TokenType::Minus;
          binaryOp.lexeme = "-";
          break;
        case TokenType::StarEqual:
          binaryOp.type = TokenType::Star;
          binaryOp.lexeme = "*";
          break;
        case TokenType::SlashEqual:
          binaryOp.type = TokenType::Slash;
          binaryOp.lexeme = "/";
          break;
        case TokenType::PercentEqual:
          binaryOp.type = TokenType::Percent;
          binaryOp.lexeme = "%";
          break;
        default:
          break;
        }

        // Create a copy of the left expression for the binary operation
        std::unique_ptr<Expr> leftCopy;
        if (auto varExpr = dynamic_cast<VarExpr *>(expr.get())) {
          leftCopy = std::make_unique<VarExpr>(varExpr->name);
        } else if (auto memberExpr = dynamic_cast<MemberAccess *>(expr.get())) {
          // For member access, we need to clone the object and member
          // This is a simplified approach - in a full implementation,
          // you'd need proper expression cloning
          throw std::runtime_error(
              "Compound assignment to member access not yet supported");
        } else {
          throw std::runtime_error(
              "Compound assignment to this expression type not yet supported");
        }

        // Create binary expression: left op right
        auto binaryExpr = std::make_unique<Binary>(std::move(leftCopy),
                                                   binaryOp, std::move(value));
        return std::make_unique<Assign>(std::move(expr), std::move(binaryExpr));
      }

      return std::make_unique<Assign>(std::move(expr), std::move(value));
    }

    throw std::runtime_error("Invalid assignment target.");
  }

  return expr;
}

std::unique_ptr<Expr> Parser::ternary() {
  auto expr = logicalOr();

  if (match({TokenType::Question})) {
    auto thenBranch = expression();
    consume(TokenType::Colon,
            "Expect ':' after then branch of ternary expression.");
    auto elseBranch = ternary();
    expr = std::make_unique<Ternary>(std::move(expr), std::move(thenBranch),
                                     std::move(elseBranch));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::logicalOr() {
  auto expr = logicalAnd();

  while (match({TokenType::PipePipe})) {
    Token op = previous();
    auto right = logicalAnd();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::logicalAnd() {
  auto expr = nilCoalescing();

  while (match({TokenType::AmpAmp})) {
    Token op = previous();
    auto right = nilCoalescing();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::nilCoalescing() {
  auto expr = bitwiseOr();

  while (match({TokenType::QuestionQuestion})) {
    Token op = previous();
    auto right = bitwiseOr();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

// Bitwise OR: expr | expr
std::unique_ptr<Expr> Parser::bitwiseOr() {
  auto expr = bitwiseXor();

  while (match({TokenType::BitwiseOr})) {
    Token op = previous();
    auto right = bitwiseXor();
    expr = std::make_unique<BitwiseExpr>(std::move(expr), op, std::move(right));
  }

  return expr;
}

// Bitwise XOR: expr ^ expr
std::unique_ptr<Expr> Parser::bitwiseXor() {
  auto expr = bitwiseAnd();

  while (match({TokenType::BitwiseXor})) {
    Token op = previous();
    auto right = bitwiseAnd();
    expr = std::make_unique<BitwiseExpr>(std::move(expr), op, std::move(right));
  }

  return expr;
}

// Bitwise AND: expr & expr
std::unique_ptr<Expr> Parser::bitwiseAnd() {
  auto expr = equality();

  while (match({TokenType::BitwiseAnd})) {
    Token op = previous();
    auto right = equality();
    expr = std::make_unique<BitwiseExpr>(std::move(expr), op, std::move(right));
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
  auto expr = bitwiseShift();
  while (match({TokenType::RAngle, TokenType::GreaterEqual, TokenType::LAngle,
                TokenType::LessEqual})) {
    Token op = previous();
    auto right = bitwiseShift();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

// Bitwise shift: expr << expr, expr >> expr
std::unique_ptr<Expr> Parser::bitwiseShift() {
  auto expr = typeCasting();

  while (match({TokenType::LeftShift, TokenType::RightShift})) {
    Token op = previous();
    auto right = typeCasting();
    expr = std::make_unique<BitwiseExpr>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::typeCasting() {
  auto expr = typeChecking();

  while (match({TokenType::As})) {
    Token op = previous();

    // Check for as?, as! variants
    TypeCast::CastType castType = TypeCast::CastType::Safe;
    if (match({TokenType::Unknown})) {
      if (previous().lexeme == "?") {
        castType = TypeCast::CastType::Optional;
      } else {
        // Put the token back
        current--;
      }
    } else if (match({TokenType::Bang})) {
      castType = TypeCast::CastType::Forced;
    }

    Token targetType = parseType();
    expr = std::make_unique<TypeCast>(std::move(expr), targetType, castType);
  }

  return expr;
}

std::unique_ptr<Expr> Parser::typeChecking() {
  auto expr = range();

  while (match({TokenType::Is})) {
    Token targetType = parseType();
    expr = std::make_unique<TypeCheck>(std::move(expr), targetType);
  }

  return expr;
}

std::unique_ptr<Expr> Parser::range() {
  auto expr = term();

  if (match({TokenType::LessEllipsis, TokenType::Ellipsis})) {
    Token op = previous();
    auto right = term();

    Range::RangeType rangeType = (op.type == TokenType::LessEllipsis)
                                     ? Range::RangeType::HalfOpen
                                     : Range::RangeType::Closed;

    return std::make_unique<Range>(std::move(expr), std::move(right),
                                   rangeType);
  }

  return expr;
}

std::unique_ptr<Expr> Parser::term() {
  auto expr = overflowArithmetic();

  while (match({TokenType::Minus, TokenType::Plus})) {
    Token op = previous();
    auto right = overflowArithmetic();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }

  return expr;
}

// Overflow arithmetic: expr &+ expr, expr &- expr, expr &* expr
std::unique_ptr<Expr> Parser::overflowArithmetic() {
  auto expr = factor();

  while (match({TokenType::OverflowPlus, TokenType::OverflowMinus,
                TokenType::OverflowStar})) {
    Token op = previous();
    auto right = factor();
    expr =
        std::make_unique<OverflowExpr>(std::move(expr), op, std::move(right));
  }

  return expr;
}

std::unique_ptr<Expr> Parser::factor() {
  auto expr = customOperatorExpression();
  while (match({TokenType::Slash, TokenType::Star})) {
    Token op = previous();
    auto right = customOperatorExpression();
    expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
  }
  return expr;
}

// Custom operator expression
std::unique_ptr<Expr> Parser::customOperatorExpression() {
  auto expr = unary();

  // Check for custom infix operators (both predefined and identifier-based)
  while ((check(TokenType::Identifier) && isCustomOperator(peek().lexeme)) ||
         check(TokenType::StarStar) || check(TokenType::LAngleRAngle) ||
         check(TokenType::PlusMinus)) {
    Token op = peek();
    // Only process as infix if it's not a prefix operator
    if (!isCustomPrefixOperator(op.lexeme)) {
      advance(); // consume the operator
      auto right = unary();
      expr = std::make_unique<CustomOperatorExpr>(std::move(expr), op,
                                                  std::move(right));
    } else {
      break; // This is a prefix operator, don't consume it here
    }
  }

  return expr;
}

std::unique_ptr<Expr> Parser::unary() {
  if (match({TokenType::Bang, TokenType::Minus, TokenType::Amp})) {
    Token op = previous();
    auto right = unary();
    return std::make_unique<Unary>(op, std::move(right));
  }

  // Handle custom prefix operators like ±
  if ((check(TokenType::Identifier) && isCustomOperator(peek().lexeme)) ||
      check(TokenType::PlusMinus)) {
    // Check if this is a prefix operator by looking at the operator registry
    Token op = peek();
    if (isCustomPrefixOperator(op.lexeme)) {
      advance(); // consume the operator
      auto right = unary();
      // Create a CustomOperatorExpr with null left operand for prefix operators
      return std::make_unique<CustomOperatorExpr>(nullptr, op,
                                                  std::move(right));
    }
  }

  // Handle try expressions: try, try?, try!
  if (match({TokenType::Try})) {
    return tryExpression();
  }

  // Handle await expressions: await expression
  if (match({TokenType::Await})) {
    return awaitExpression();
  }

  // Handle task expressions: Task { ... }
  if (match({TokenType::Task})) {
    return taskExpression();
  }

  return call();
}

std::unique_ptr<Expr> Parser::primary() {
  if (match({TokenType::True, TokenType::False, TokenType::Nil,
             TokenType::IntegerLiteral, TokenType::FloatingLiteral,
             TokenType::StringLiteral, TokenType::CharacterLiteral})) {
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
    // Don't consume the dot and method name here - let call() handle it
    // Create a Super expression with an empty method token
    Token emptyMethod = Token(TokenType::Identifier, "", keyword.line);
    return std::make_unique<Super>(keyword, emptyMethod);
  }

  // Handle opaque types: some Protocol
  if (match({TokenType::Some})) {
    Token someKeyword = previous();
    consume(TokenType::Identifier, "Expect protocol name after 'some'.");
    Token protocolName = previous();
    return std::make_unique<OpaqueTypeExpr>(someKeyword, protocolName);
  }

  // Handle boxed protocol types: any Protocol
  if (match({TokenType::Any})) {
    Token anyKeyword = previous();
    consume(TokenType::Identifier, "Expect protocol name after 'any'.");
    Token protocolName = previous();
    return std::make_unique<BoxedProtocolTypeExpr>(anyKeyword, protocolName);
  }

  // Handle string interpolation - can start with either
  // InterpolatedStringLiteral or InterpolationStart
  if (match({TokenType::InterpolatedStringLiteral,
             TokenType::InterpolationStart})) {
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
        consume(TokenType::InterpolationEnd,
                "Expect ')' after interpolation expression.");
        parts.emplace_back(std::move(expr));

        // After interpolation, check for more string content
        if (match({TokenType::InterpolatedStringLiteral,
                   TokenType::StringLiteral})) {
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
    consume(TokenType::InterpolationEnd,
            "Expect ')' after interpolation expression.");
    return expr;
  }

  // Array or dictionary literal
  if (match({TokenType::LSquare})) {
    return arrayLiteral();
  }

  if (match({TokenType::Identifier, TokenType::String, TokenType::Int,
             TokenType::Bool,       TokenType::Double, TokenType::Int8,
             TokenType::Int16,      TokenType::Int32,  TokenType::Int64,
             TokenType::UInt,       TokenType::UInt8,  TokenType::UInt16,
             TokenType::UInt64,     TokenType::Float,  TokenType::Character,
             TokenType::Any,        TokenType::Void,   TokenType::Set,
             TokenType::Left,       TokenType::Right})) {
    Token identifier = previous();

    std::unique_ptr<Expr> expr;

    // Check for Result type: Result<Success, Error>
    if (identifier.type == TokenType::Identifier &&
        identifier.lexeme == "Result" && check(TokenType::LAngle)) {
      return resultTypeExpression();
    }

    // Check for Error literal: ErrorType("message")
    // Removed incorrect error literal detection logic
    // This was causing normal function calls with string arguments to be
    // misinterpreted

    // Check for generic type instantiation: Identifier<Type1, Type2>
    // Only parse as generic if the next token after '<' looks like a type
    if (identifier.type == TokenType::Identifier && check(TokenType::LAngle)) {
      // Look ahead to see if this is really a generic type instantiation
      int savedCurrent = current;
      advance(); // consume '<'
      
      bool isGeneric = false;
      if (!check(TokenType::RAngle)) {
        // Check if the next token could be a type name (not a literal or variable)
        // Only consider built-in type keywords, not literals or variables
        if (check(TokenType::String) || check(TokenType::Int) || check(TokenType::Bool) ||
            check(TokenType::Double) || check(TokenType::Int8) || check(TokenType::Int16) ||
            check(TokenType::Int32) || check(TokenType::Int64) || check(TokenType::UInt) ||
            check(TokenType::UInt8) || check(TokenType::UInt16) || check(TokenType::UInt64) ||
            check(TokenType::Float) || check(TokenType::Character) || check(TokenType::Any) ||
            check(TokenType::Void) || check(TokenType::Set)) {
          isGeneric = true;
        }
        // For identifiers, we need to be more careful - only treat as generic
        // if it's likely a type name (starts with uppercase)
        else if (check(TokenType::Identifier)) {
          std::string tokenText = peek().lexeme;
          if (!tokenText.empty() && std::isupper(tokenText[0])) {
            isGeneric = true;
          }
        }
      }
      
      if (isGeneric) {
        // Parse generic type instantiation
        std::vector<Token> typeArguments;
        if (!check(TokenType::RAngle)) {
          do {
            Token typeArg = parseType();
            typeArguments.push_back(typeArg);
          } while (match({TokenType::Comma}));
        }

        consume(TokenType::RAngle, "Expect '>' after generic type arguments.");

        // Create a GenericTypeInstantiation expression
        expr = std::make_unique<GenericTypeInstantiationExpr>(
            identifier, std::move(typeArguments));
      } else {
        // Not a generic, restore position and treat as regular variable
        current = savedCurrent;
        expr = std::make_unique<VarExpr>(identifier);
      }
    } else {
      // Regular variable expression
      expr = std::make_unique<VarExpr>(identifier);
    }

    // Check for index/subscript access: identifier[index] or identifier[arg1,
    // arg2, ...]
    while (check(TokenType::LSquare)) {
      // Check if this is a multi-parameter subscript by looking ahead
      int savedCurrent = current;
      bool isMultiParam = false;

      advance(); // consume '['

      // Look for comma to detect multi-parameter subscript
      int depth = 1;
      while (depth > 0 && !isAtEnd()) {
        if (peek().type == TokenType::LSquare)
          depth++;
        else if (peek().type == TokenType::RSquare)
          depth--;
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
    // Check if this is a tuple literal or a grouping expression
    if (check(TokenType::RParen)) {
      // Empty tuple: ()
      consume(TokenType::RParen, "Expect ')' after empty tuple.");
      return std::make_unique<TupleLiteral>(
          std::vector<std::unique_ptr<Expr>>{});
    }

    auto firstExpr = expression();

    if (match({TokenType::Comma})) {
      // This is a tuple literal: (expr1, expr2, ...)
      std::vector<std::unique_ptr<Expr>> elements;
      elements.push_back(std::move(firstExpr));

      if (!check(TokenType::RParen)) {
        do {
          elements.push_back(expression());
        } while (match({TokenType::Comma}));
      }

      consume(TokenType::RParen, "Expect ')' after tuple elements.");
      return std::make_unique<TupleLiteral>(std::move(elements));
    } else {
      // This is a grouping expression: (expr)
      consume(TokenType::RParen, "Expect ')' after expression.");
      return std::make_unique<Grouping>(std::move(firstExpr));
    }
  }

  // Handle macro expressions: #macroName(arguments)
  if (match({TokenType::Hash})) {
    consume(TokenType::Identifier, "Expect macro name after '#'.");
    Token macroName = previous();

    consume(TokenType::LParen, "Expect '(' after macro name.");
    std::vector<std::unique_ptr<Expr>> arguments;

    if (!check(TokenType::RParen)) {
      do {
        arguments.push_back(expression());
      } while (match({TokenType::Comma}));
    }

    consume(TokenType::RParen, "Expect ')' after macro arguments.");

    return std::make_unique<MacroExpansionExpr>(macroName,
                                                std::move(arguments));
  }

  // Handle range operators: start..<end or start...end
  if (check(TokenType::LessEllipsis) || check(TokenType::Ellipsis)) {
    // This is a range starting from an implicit 0
    auto start = std::make_unique<Literal>(
        Token(TokenType::FloatingLiteral, "0.0", peek().line));

    TokenType rangeOp = advance().type;
    auto end = unary();

    Range::RangeType rangeType = (rangeOp == TokenType::LessEllipsis)
                                     ? Range::RangeType::HalfOpen
                                     : Range::RangeType::Closed;

    return std::make_unique<Range>(std::move(start), std::move(end), rangeType);
  }

  std::cout << "DEBUG: primary() - unexpected token: " << peek().lexeme
            << " (type: " << static_cast<int>(peek().type) << ")" << std::endl;

  // Include line number in error message
  std::string errorMsg = "Expect expression.";
  if (!isAtEnd()) {
    errorMsg += " (Line " + std::to_string(peek().line) + ")";
  }
  throw std::runtime_error(errorMsg);
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

  // Include line number in error message
  std::string errorMsg = message;
  if (!isAtEnd()) {
    errorMsg += " (Line " + std::to_string(peek().line) + ")";
  }
  throw std::runtime_error(errorMsg);
}

// Parse type annotations including collection types
Token Parser::parseType() {
  if (match({TokenType::LParen})) {
    // Tuple type: (Type1, Type2, ...)
    std::string tupleType = "(";

    if (!check(TokenType::RParen)) {
      do {
        Token elementType = parseType();
        tupleType += elementType.lexeme;
        if (match({TokenType::Comma})) {
          tupleType += ", ";
        }
      } while (previous().type == TokenType::Comma);
    }

    consume(TokenType::RParen, "Expect ')' after tuple type.");
    tupleType += ")";

    Token tupleToken = Token(TokenType::Identifier, tupleType, peek().line);

    // Check for optional type suffix (?)
    if (match({TokenType::Question})) {
      return Token(TokenType::Identifier, tupleToken.lexeme + "?",
                   tupleToken.line);
    }

    return tupleToken;
  }

  if (match({TokenType::LSquare})) {
    // Could be array type [ElementType] or dictionary type [KeyType: ValueType]
    Token firstType(TokenType::Identifier, "", 0);
    if (check(TokenType::LSquare)) {
      // Nested array type
      firstType = parseType();
    } else if (check(TokenType::LParen)) {
      // Tuple type as array element: [(Type1, Type2)]
      firstType = parseType();
    } else {
      // Accept basic types or identifiers
      if (match({TokenType::String, TokenType::Int, TokenType::Bool,
                 TokenType::Double, TokenType::Int8, TokenType::Int16,
                 TokenType::Int32, TokenType::Int64, TokenType::UInt,
                 TokenType::UInt8, TokenType::UInt16, TokenType::UInt64,
                 TokenType::Float, TokenType::Character, TokenType::Any,
                 TokenType::Void, TokenType::Set, TokenType::Identifier})) {
        firstType = previous();
      } else {
        std::cout << "DEBUG: parseType failed, current token: " << peek().lexeme
                  << " (type: " << static_cast<int>(peek().type) << ")"
                  << std::endl;

        // Include line number in error message
        std::string errorMsg = "Expect type name.";
        if (!isAtEnd()) {
          errorMsg += " (Line " + std::to_string(peek().line) + ")";
        }
        throw std::runtime_error(errorMsg);
      }
    }

    // Check if this is a dictionary type (has colon)
    if (match({TokenType::Colon})) {
      // Dictionary type: [KeyType: ValueType]
      Token valueType(TokenType::Identifier, "", 0);
      if (check(TokenType::LSquare)) {
        // Nested type for value
        valueType = parseType();
      } else {
        // Accept basic types or identifiers
        if (match({TokenType::String, TokenType::Int, TokenType::Bool,
                   TokenType::Double, TokenType::Int8, TokenType::Int16,
                   TokenType::Int32, TokenType::Int64, TokenType::UInt,
                   TokenType::UInt8, TokenType::UInt16, TokenType::UInt64,
                   TokenType::Float, TokenType::Character, TokenType::Any,
                   TokenType::Void, TokenType::Set, TokenType::Identifier})) {
          valueType = previous();
        } else {
          // Include line number in error message
          std::string errorMsg = "Expect value type name.";
          if (!isAtEnd()) {
            errorMsg += " (Line " + std::to_string(peek().line) + ")";
          }
          throw std::runtime_error(errorMsg);
        }
      }
      consume(TokenType::RSquare, "Expect ']' after dictionary type.");
      // Create a synthetic token for dictionary type
      Token dictType =
          Token(TokenType::Identifier,
                "[" + firstType.lexeme + ":" + valueType.lexeme + "]",
                firstType.line);

      // Check for optional type suffix (?) after dictionary type
      if (match({TokenType::Question})) {
        // Create a synthetic token for optional dictionary type
        return Token(TokenType::Identifier, dictType.lexeme + "?",
                     dictType.line);
      }

      return dictType;
    } else {
      // Array type: [ElementType]
      consume(TokenType::RSquare, "Expect ']' after array element type.");
      // Create a synthetic token for array type
      Token arrayType = Token(TokenType::Identifier,
                              "[" + firstType.lexeme + "]", firstType.line);

      // Check for optional type suffix (?) after array type
      if (match({TokenType::Question})) {
        // Create a synthetic token for optional array type
        return Token(TokenType::Identifier, arrayType.lexeme + "?",
                     arrayType.line);
      }

      return arrayType;
    }
  }

  // Handle opaque types: some Protocol
  if (match({TokenType::Some})) {
    Token someKeyword = previous();
    consume(TokenType::Identifier, "Expect protocol name after 'some'.");
    Token protocolName = previous();
    // Create a synthetic token for opaque type
    return Token(TokenType::Identifier, "some " + protocolName.lexeme,
                 someKeyword.line);
  }

  // Handle boxed protocol types: any Protocol
  if (match({TokenType::Any})) {
    Token anyKeyword = previous();
    consume(TokenType::Identifier, "Expect protocol name after 'any'.");
    Token protocolName = previous();
    // Create a synthetic token for boxed protocol type
    return Token(TokenType::Identifier, "any " + protocolName.lexeme,
                 anyKeyword.line);
  }

  // Accept basic types or identifiers
  if (match({TokenType::String, TokenType::Int, TokenType::Bool,
             TokenType::Double, TokenType::Int8, TokenType::Int16,
             TokenType::Int32, TokenType::Int64, TokenType::UInt,
             TokenType::UInt8, TokenType::UInt16, TokenType::UInt64,
             TokenType::Float, TokenType::Character, TokenType::Void,
             TokenType::Set, TokenType::Identifier})) {
    Token baseType = previous();

    // Check for generic type syntax: Type<T, U>
    if (match({TokenType::LAngle})) {
      std::string genericType = baseType.lexeme + "<";

      if (!check(TokenType::RAngle)) {
        do {
          Token typeArg = parseType();
          genericType += typeArg.lexeme;
          if (match({TokenType::Comma})) {
            genericType += ", ";
          }
        } while (previous().type == TokenType::Comma);
      }

      consume(TokenType::RAngle, "Expect '>' after generic type arguments.");
      genericType += ">";

      Token genericToken =
          Token(TokenType::Identifier, genericType, baseType.line);

      // Check for optional type suffix (?)
      if (match({TokenType::Question})) {
        return Token(TokenType::Identifier, genericToken.lexeme + "?",
                     genericToken.line);
      }

      return genericToken;
    }

    // Check for optional type suffix (?)
    if (match({TokenType::Question})) {
      // Create a synthetic token for optional type
      return Token(TokenType::Identifier, baseType.lexeme + "?", baseType.line);
    }

    return baseType;
  }

  std::cout << "DEBUG: parseType failed at end, current token: "
            << peek().lexeme << " (type: " << static_cast<int>(peek().type)
            << ")" << std::endl;
  throw std::runtime_error("Expect type name.");
}

// Parse array literal: [1, 2, 3] or dictionary literal: ["key": value]
std::unique_ptr<Expr> Parser::arrayLiteral() {
  // We've already consumed the '['
  std::vector<std::unique_ptr<Expr>> elements;

  // Check for empty dictionary [:]
  if (check(TokenType::Colon)) {
    advance(); // consume ':'
    consume(TokenType::RSquare, "Expect ']' after empty dictionary literal.");
    std::vector<DictionaryLiteral::KeyValuePair> pairs;
    return std::make_unique<DictionaryLiteral>(std::move(pairs));
  }

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
          if (check(TokenType::RSquare))
            break; // Trailing comma
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
  return std::make_unique<SubscriptAccess>(std::move(object),
                                           std::move(arguments));
}

// Helper method to check current token type without consuming
bool Parser::check(TokenType type) {
  if (isAtEnd())
    return false;
  return peek().type == type;
}

// Parse block statement: { statements }
std::unique_ptr<Stmt> Parser::blockStatement() {
  std::vector<std::unique_ptr<Stmt>> statements;

  while (!check(TokenType::RBrace) && !check(TokenType::Catch) && !isAtEnd()) {
    auto decls = declaration();
    statements.insert(statements.end(), std::make_move_iterator(decls.begin()),
                      std::make_move_iterator(decls.end()));
  }

  // Only consume RBrace if we're not at a catch clause
  if (!check(TokenType::Catch)) {
    consume(TokenType::RBrace, "Expect '}' after block.");
  }
  return std::make_unique<BlockStmt>(std::move(statements));
}

// Parse if statement: if condition { thenBranch } else { elseBranch }
std::unique_ptr<Stmt> Parser::ifStatement() {
  // Check if this is an "if let" statement
  if (check(TokenType::Let)) {
    // Parse "if let variable = expression"
    consume(TokenType::Let, "Expect 'let' in if-let statement.");
    consume(TokenType::Identifier, "Expect variable name after 'let'.");
    Token variable = previous();

    consume(TokenType::Equal, "Expect '=' after variable in if-let statement.");
    auto expression = this->expression();

    consume(TokenType::LBrace, "Expect '{' after if-let condition.");
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

    return std::make_unique<IfLetStmt>(variable, std::move(expression),
                                       std::move(thenBranch),
                                       std::move(elseBranch));
  } else {
    // Regular if statement
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

    return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch),
                                    std::move(elseBranch));
  }
}

// Parse while statement: while condition { body }
std::unique_ptr<Stmt> Parser::whileStatement() {
  auto condition = expression();

  consume(TokenType::LBrace, "Expect '{' after while condition.");
  auto body = blockStatement();

  return std::make_unique<WhileStmt>(std::move(condition), std::move(body));
}

// Parse for statement: for variable in collection { body } or for (var1, var2)
// in collection { body }
std::unique_ptr<Stmt> Parser::forStatement() {
  // Check if this is a for-in loop or C-style for loop
  // Look ahead to determine if this is a C-style for loop (has semicolons) or
  // for-in loop
  bool isCStyleFor = false;
  if (check(TokenType::LParen)) {
    // Look ahead to see if we have semicolons (C-style) or 'in' keyword
    // (for-in)
    int savedCurrent = current;
    advance(); // consume '('

    // Skip tokens until we find either ';' (C-style) or 'in' (for-in) or ')'
    int parenDepth = 1;
    while (parenDepth > 0 && !isAtEnd()) {
      if (peek().type == TokenType::LParen)
        parenDepth++;
      else if (peek().type == TokenType::RParen)
        parenDepth--;
      else if (peek().type == TokenType::Semicolon && parenDepth == 1) {
        isCStyleFor = true;
        break;
      }
      advance();
    }

    // Restore position
    current = savedCurrent;
  }

  if (check(TokenType::LParen) && isCStyleFor) {
    // C-style for loop: for (initializer; condition; increment) { body }
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

      initializer =
          std::make_unique<VarStmt>(name, std::move(init), isConst, type);
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

    return std::make_unique<ForStmt>(std::move(initializer),
                                     std::move(condition), std::move(increment),
                                     std::move(body));
  } else {
    // For-in loop: for variable in collection { body } or for (var1, var2) in
    // collection { body }
    std::vector<Token> variables;

    // Check for tuple destructuring: (var1, var2)
    if (match({TokenType::LParen})) {
      do {
        consume(TokenType::Identifier, "Expect variable name in tuple.");
        variables.push_back(previous());
      } while (match({TokenType::Comma}));
      consume(TokenType::RParen, "Expect ')' after tuple variables.");
    } else {
      // Single variable
      consume(TokenType::Identifier, "Expect variable name.");
      variables.push_back(previous());
    }

    consume(TokenType::In, "Expect 'in' after for loop variable(s).");
    auto collection = expression();

    // Parse body - can be a block statement or a single statement
    std::unique_ptr<Stmt> body;
    if (match({TokenType::LBrace})) {
      body = blockStatement();
    } else {
      // Single statement
      body = statement();
    }

    return std::make_unique<ForInStmt>(std::move(variables),
                                       std::move(collection), std::move(body));
  }
}

// Parse function declaration: func name(parameters) -> ReturnType { body }
std::unique_ptr<Stmt> Parser::functionDeclaration() {
  // Function name can be an identifier or an operator
  Token functionName(TokenType::Identifier, "", peek().line);
  bool isOperatorFunction = false;

  if (match({TokenType::Identifier})) {
    functionName = previous();
  } else if (match({TokenType::Star, TokenType::Plus, TokenType::Minus,
                    TokenType::Slash, TokenType::Percent, TokenType::LAngle,
                    TokenType::RAngle, TokenType::Bang, TokenType::Equal,
                    TokenType::BitwiseAnd, TokenType::BitwiseOr,
                    TokenType::BitwiseXor, TokenType::BitwiseNot,
                    TokenType::StarStar, TokenType::LAngleRAngle,
                    TokenType::PlusMinus})) {
    functionName = previous();
    isOperatorFunction = true;
  } else {
    throw std::runtime_error("Expect function name.");
  }

  // Parse optional generic parameter clause (only for regular functions, not
  // operators)
  GenericParameterClause genericParams =
      isOperatorFunction ? GenericParameterClause(std::vector<TypeParameter>())
                         : parseGenericParameterClause();

  consume(TokenType::LParen, "Expect '(' after function name.");

  std::vector<Parameter> parameters;
  if (!check(TokenType::RParen)) {
    do {
      // Parse parameter name (can be identifier, keyword like 'in', 'self', or
      // underscore)
      Token paramName = Token(TokenType::Identifier, "", peek().line);
      Token externalName = Token(TokenType::Identifier, "", peek().line);

      // Check for underscore as external parameter name
      if (check(TokenType::Identifier) && peek().lexeme == "_") {
        advance(); // consume the underscore
        externalName = previous();

        // Must have internal parameter name after underscore
        consume(TokenType::Identifier,
                "Expect internal parameter name after '_'.");
        paramName = previous();
      } else if (check(TokenType::Identifier) || check(TokenType::In) ||
                 check(TokenType::Self) || check(TokenType::Left) ||
                 check(TokenType::Right)) {
        advance(); // consume the token
        paramName = previous();
        externalName =
            paramName; // Default: external name same as internal name

        // Check for external parameter name syntax: externalName internalName:
        // Type
        if (check(TokenType::Identifier)) {
          // This is external parameter name, consume the internal parameter
          // name
          externalName = paramName;
          consume(TokenType::Identifier, "Expect internal parameter name.");
          paramName =
              previous(); // Use internal name as the actual parameter name
        }
      } else {
        throw std::runtime_error("Expect parameter name.");
      }

      consume(TokenType::Colon, "Expect ':' after parameter name.");

      // Check for inout keyword before type
      bool isInout = false;
      if (match({TokenType::Inout})) {
        isInout = true;
      }

      Token paramType = parseType();

      // Check for variadic parameter (...)
      bool isVariadic = false;
      if (match({TokenType::Ellipsis})) {
        isVariadic = true;
      }

      parameters.emplace_back(paramName, externalName, paramType, isInout,
                              isVariadic);

      // If this is a variadic parameter, it must be the last one
      if (isVariadic && check(TokenType::Comma)) {
        throw std::runtime_error(
            "Variadic parameter must be the last parameter.");
      }
    } while (match({TokenType::Comma}));
  }

  consume(TokenType::RParen, "Expect ')' after parameters.");

  // Check for async keyword
  bool isAsync = false;
  if (match({TokenType::Async})) {
    isAsync = true;
  }

  // Check for throws keyword
  bool canThrow = false;
  if (match({TokenType::Throws})) {
    canThrow = true;
  }

  Token returnType = Token(TokenType::Identifier, "Void", functionName.line);
  if (match({TokenType::Arrow})) {
    returnType = parseType();
  }

  // Parse optional where clause
  WhereClause whereClause = parseWhereClause();

  consume(TokenType::LBrace, "Expect '{' before function body.");
  auto body = blockStatement();

  return std::make_unique<FunctionStmt>(
      functionName, std::move(parameters), returnType, std::move(body),
      AccessLevel::INTERNAL, std::move(genericParams), std::move(whereClause),
      false, canThrow, isAsync);
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

std::unique_ptr<Stmt> Parser::continueStatement() {
  Token keyword = previous();
  std::optional<Token> label;

  // Check for optional label: continue labelName
  if (check(TokenType::Identifier)) {
    label = advance();
  }

  match({TokenType::Semicolon}); // Optional semicolon
  return std::make_unique<ContinueStmt>(label.value_or(Token()));
}

std::unique_ptr<Stmt> Parser::breakStatement() {
  Token keyword = previous();
  std::optional<Token> label;

  // Check for optional label: break labelName
  if (check(TokenType::Identifier)) {
    label = advance();
  }

  match({TokenType::Semicolon}); // Optional semicolon
  return std::make_unique<BreakStmt>(label.value_or(Token()));
}

std::unique_ptr<Stmt> Parser::fallthroughStatement() {
  Token keyword = previous();
  match({TokenType::Semicolon}); // Optional semicolon
  return std::make_unique<FallthroughStmt>();
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
        if (peek().type == TokenType::LSquare)
          depth++;
        else if (peek().type == TokenType::RSquare)
          depth--;
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
    } else if (match({TokenType::QuestionDot})) {
      // Optional chaining: object?.property, object?.method(), object?[index]
      if (check(TokenType::Identifier)) {
        consume(TokenType::Identifier, "Expect property name after '?.'.");
        Token propertyName = previous();

        // Check if this is a method call: object?.method()
        if (match({TokenType::LParen})) {
          // Parse method call arguments
          std::vector<std::unique_ptr<Expr>> arguments;
          if (!check(TokenType::RParen)) {
            do {
              arguments.push_back(expression());
            } while (match({TokenType::Comma}));
          }
          consume(TokenType::RParen, "Expect ')' after method arguments.");

          // Create a Call expression as accessor
          auto callExpr = std::make_unique<Call>(
              std::make_unique<VarExpr>(propertyName), std::move(arguments));
          expr = std::make_unique<OptionalChaining>(
              std::move(expr), OptionalChaining::ChainType::Method,
              std::move(callExpr));
        } else {
          // Property access: object?.property
          auto propertyExpr = std::make_unique<VarExpr>(propertyName);
          expr = std::make_unique<OptionalChaining>(
              std::move(expr), OptionalChaining::ChainType::Property,
              std::move(propertyExpr));
        }
      } else if (match({TokenType::LSquare})) {
        // Optional subscript: object?[index]
        auto index = expression();
        consume(TokenType::RSquare,
                "Expect ']' after optional subscript index.");

        // Create an IndexAccess expression as accessor
        auto indexExpr = std::make_unique<IndexAccess>(
            std::make_unique<VarExpr>(
                Token(TokenType::Identifier, "__placeholder__", 0)),
            std::move(index));
        expr = std::make_unique<OptionalChaining>(
            std::move(expr), OptionalChaining::ChainType::Subscript,
            std::move(indexExpr));
      } else {
        throw std::runtime_error("Expect property name or '[' after '?.'.");
      }
    } else if (match({TokenType::QuestionLSquare})) {
      // Optional subscript: object?[index]
      auto index = expression();
      consume(TokenType::RSquare, "Expect ']' after optional subscript index.");

      // Create an IndexAccess expression as accessor
      auto indexExpr = std::make_unique<IndexAccess>(
          std::make_unique<VarExpr>(
              Token(TokenType::Identifier, "__placeholder__", 0)),
          std::move(index));
      expr = std::make_unique<OptionalChaining>(
          std::move(expr), OptionalChaining::ChainType::Subscript,
          std::move(indexExpr));
    } else if (match({TokenType::Dot})) {
      // Support both identifier member access (object.member) and tuple index
      // access (tuple.0)
      Token memberName(TokenType::Identifier, "", peek().line);
      if (check(TokenType::Identifier)) {
        consume(TokenType::Identifier, "Expect member name after '.'.");
        memberName = previous();
      } else if (check(TokenType::IntegerLiteral)) {
        // Tuple index access: tuple.0, tuple.1, etc.
        consume(TokenType::IntegerLiteral, "Expect tuple index after '.'.");
        memberName = previous();
      } else {
        throw std::runtime_error(
            "Expect member name or tuple index after '.'.");
      }

      // Create member access first
      expr = std::make_unique<MemberAccess>(std::move(expr), memberName);

      // Check if this is followed by a function call
      if (check(TokenType::LParen)) {
        // This is a method call: object.method(args)
        advance(); // consume '('
        expr = finishCall(std::move(expr));
      }
    } else if (match({TokenType::Bang})) {
      // Handle postfix bang operator for force unwrapping: expr!
      Token op = previous();
      expr = std::make_unique<Unary>(op, std::move(expr));
    } else {
      break;
    }
  }

  return expr;
}

// Parse function call arguments
std::unique_ptr<Expr> Parser::finishCall(std::unique_ptr<Expr> callee) {
  // Parse function call with support for labeled arguments
  std::vector<std::unique_ptr<Expr>> arguments;
  std::vector<Token> argumentLabels; // Store argument labels for validation
  bool hasAnyLabels = false;

  if (!check(TokenType::RParen)) {
    do {
      Token argumentLabel = Token(TokenType::Identifier, "", peek().line);
      bool hasLabel = false;

      // Check for parameter label: label: expression
      // Parameter labels can be identifiers or keywords like 'in'
      if (check(TokenType::Identifier) || check(TokenType::In)) {
        int savedCurrent = current;
        advance(); // consume identifier or keyword
        if (check(TokenType::Colon)) {
          // This is a parameter label
          argumentLabel = previous();
          hasLabel = true;
          hasAnyLabels = true;
          advance(); // consume ':'
          arguments.push_back(expression());
        } else {
          // Not a parameter label, restore position and parse as expression
          current = savedCurrent;
          arguments.push_back(expression());
        }
      } else {
        // Not an identifier or keyword, parse as expression
        arguments.push_back(expression());
      }

      argumentLabels.push_back(
          hasLabel ? argumentLabel
                   : Token(TokenType::Identifier, "", peek().line));
    } while (match({TokenType::Comma}));
  }

  consume(TokenType::RParen, "Expect ')' after arguments.");

  // Check if this is an operator function call (no labels expected)
  if (auto varExpr = dynamic_cast<const VarExpr *>(callee.get())) {
    if (isCustomOperator(varExpr->name.lexeme) ||
        isCustomPrefixOperator(varExpr->name.lexeme)) {
      // This is an operator function call, use regular Call instead of
      // LabeledCall
      return std::make_unique<Call>(std::move(callee), std::move(arguments));
    }
  }

  // For regular functions, create LabeledCall to handle mixed labeled/unlabeled
  // arguments
  return std::make_unique<LabeledCall>(std::move(callee), std::move(arguments),
                                       std::move(argumentLabels));
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
        // Check for inout keyword
        bool isInout = false;
        if (match({TokenType::Inout})) {
          isInout = true;
        }

        consume(TokenType::Identifier, "Expect parameter name.");
        Token paramName = previous();
        consume(TokenType::Colon, "Expect ':' after parameter name.");
        Token paramType = parseType();
        parameters.emplace_back(paramName, paramType, isInout);
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
  return std::make_unique<Closure>(std::move(parameters), returnType,
                                   std::move(body));
}

// Parse enum declaration: enum Name: RawType { case1, case2(AssociatedType),
// case3 = rawValue }
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
  std::vector<std::unique_ptr<Stmt>> nestedTypes;

  while (!check(TokenType::RBrace) && !isAtEnd()) {
    if (match({TokenType::Case})) {
      // Parse enum cases
      do {
        consume(TokenType::Identifier, "Expect case name.");
        Token caseName = previous();

        std::vector<Token> associatedTypes;
        std::unique_ptr<Expr> rawValue = nullptr;

        // Check for associated values: case name(Type1, Type2) or case
        // name(label: Type)
        if (match({TokenType::LParen})) {
          if (!check(TokenType::RParen)) {
            do {
              // Check if this is a labeled parameter: label: Type
              if (check(TokenType::Identifier) &&
                  tokens[current + 1].type == TokenType::Colon) {
                // Skip the label and colon
                advance(); // consume label
                advance(); // consume colon
              }
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

        cases.emplace_back(caseName, std::move(associatedTypes),
                           std::move(rawValue));
      } while (match({TokenType::Comma}));

      // Optional semicolon or newline
      match({TokenType::Semicolon});
    } else if (match({TokenType::Static})) {
      // Parse static members (like static subscript)
      if (match({TokenType::Subscript})) {
        // Parse static subscript declaration
        auto subscript = std::unique_ptr<SubscriptStmt>(
            static_cast<SubscriptStmt *>(subscriptDeclaration().release()));
        subscripts.push_back(std::move(subscript));
      } else {
        throw std::runtime_error(
            "Only static subscripts are currently supported in enums.");
      }
    } else if (match({TokenType::Struct})) {
      // Parse nested struct declaration
      auto nestedStruct = structDeclaration();
      nestedTypes.push_back(std::move(nestedStruct));
    } else if (match({TokenType::Class})) {
      // Parse nested class declaration
      auto nestedClass = classDeclaration();
      nestedTypes.push_back(std::move(nestedClass));
    } else if (match({TokenType::Enum})) {
      // Parse nested enum declaration
      auto nestedEnum = enumDeclaration();
      nestedTypes.push_back(std::move(nestedEnum));
    } else {
      throw std::runtime_error("Expect 'case', 'static', 'struct', 'class', or "
                               "'enum' in enum declaration.");
    }
  }

  consume(TokenType::RBrace, "Expect '}' after enum cases.");
  return std::make_unique<EnumStmt>(name, rawType, std::move(cases),
                                    std::move(subscripts),
                                    std::move(nestedTypes));
}

// Parse struct declaration: struct Name<T> { var member1: Type, let member2:
// Type = defaultValue }
std::unique_ptr<Stmt> Parser::structDeclaration() {
  consume(TokenType::Identifier, "Expect struct name.");
  Token name = previous();

  // std::cout << "Parsing struct: " << name.lexeme << std::endl;

  // Parse optional generic parameter clause
  GenericParameterClause genericParams = parseGenericParameterClause();

  // Parse protocol conformance
  std::vector<Token> conformedProtocols;
  if (match({TokenType::Colon})) {
    do {
      consume(TokenType::Identifier, "Expect protocol name.");
      conformedProtocols.push_back(previous());
    } while (match({TokenType::Comma}));
  }

  // Parse optional where clause
  WhereClause whereClause = parseWhereClause();

  consume(TokenType::LBrace, "Expect '{' after struct name.");

  std::vector<StructMember> members;
  std::vector<std::unique_ptr<FunctionStmt>> methods;
  std::vector<std::unique_ptr<InitStmt>> initializers;
  std::unique_ptr<DeinitStmt> deinitializer;
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts;
  std::vector<std::unique_ptr<Stmt>> nestedTypes;

  while (!check(TokenType::RBrace) && !isAtEnd()) {
    // Parse optional access level modifier for struct members
    AccessLevel memberAccessLevel = AccessLevel::INTERNAL;
    AccessLevel memberSetterAccessLevel = AccessLevel::INTERNAL;

    if (isAccessLevelToken(peek().type)) {
      auto accessPair = parseAccessLevelWithSetter();
      memberAccessLevel = accessPair.first;
      memberSetterAccessLevel = accessPair.second;
    }

    // Check for attributes like @resultBuilder
    if (match({TokenType::At})) {
      if (match({TokenType::ResultBuilder})) {
        // Parse @resultBuilder struct declaration
        auto resultBuilderStruct = resultBuilderDeclaration();
        nestedTypes.push_back(std::move(resultBuilderStruct));
        continue;
      } else {
        throw std::runtime_error("Unknown attribute in struct body.");
      }
    }

    // Check for mutating keyword before func
    bool isMutating = false;
    if (match({TokenType::Mutating})) {
      isMutating = true;
    }

    if (match({TokenType::Func})) {
      // Parse method declaration
      auto method = std::unique_ptr<FunctionStmt>(
          static_cast<FunctionStmt *>(functionDeclaration().release()));
      method->accessLevel = memberAccessLevel;
      method->isMutating = isMutating;
      methods.push_back(std::move(method));
    } else if (isMutating) {
      throw std::runtime_error("Expect 'func' after 'mutating'.");
    } else if (match({TokenType::Init})) {
      // Parse initializer declaration
      auto init = std::unique_ptr<InitStmt>(
          static_cast<InitStmt *>(initDeclaration().release()));
      init->accessLevel = memberAccessLevel;
      initializers.push_back(std::move(init));
    } else if (match({TokenType::Deinit})) {
      // Parse deinitializer declaration
      if (deinitializer) {
        throw std::runtime_error("Multiple deinitializers not allowed.");
      }
      deinitializer = std::unique_ptr<DeinitStmt>(
          static_cast<DeinitStmt *>(deinitDeclaration().release()));
    } else if (match({TokenType::Subscript})) {
      // Parse subscript declaration
      auto subscript = std::unique_ptr<SubscriptStmt>(
          static_cast<SubscriptStmt *>(subscriptDeclaration().release()));
      subscript->accessLevel = memberAccessLevel;
      subscript->setterAccessLevel = memberSetterAccessLevel;
      subscripts.push_back(std::move(subscript));
    } else if (match({TokenType::Struct})) {
      // Parse nested struct declaration
      auto nestedStruct = structDeclaration();
      nestedTypes.push_back(std::move(nestedStruct));
    } else if (match({TokenType::Class})) {
      // Parse nested class declaration
      auto nestedClass = classDeclaration();
      nestedTypes.push_back(std::move(nestedClass));
    } else if (match({TokenType::Enum})) {
      // Parse nested enum declaration
      auto nestedEnum = enumDeclaration();
      nestedTypes.push_back(std::move(nestedEnum));
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
            accessors.emplace_back(AccessorType::SET, std::move(body),
                                   paramName);
          } else if (match({TokenType::WillSet})) {
            std::string paramName = "newValue";
            if (match({TokenType::LParen})) {
              consume(TokenType::Identifier, "Expect parameter name.");
              paramName = previous().lexeme;
              consume(TokenType::RParen, "Expect ')' after parameter name.");
            }
            consume(TokenType::LBrace, "Expect '{' after 'willSet'.");
            auto body = blockStatement();
            accessors.emplace_back(AccessorType::WILL_SET, std::move(body),
                                   paramName);
          } else if (match({TokenType::DidSet})) {
            std::string paramName = "oldValue";
            if (match({TokenType::LParen})) {
              consume(TokenType::Identifier, "Expect parameter name.");
              paramName = previous().lexeme;
              consume(TokenType::RParen, "Expect ')' after parameter name.");
            }
            consume(TokenType::LBrace, "Expect '{' after 'didSet'.");
            auto body = blockStatement();
            accessors.emplace_back(AccessorType::DID_SET, std::move(body),
                                   paramName);
          } else {
            throw std::runtime_error("Expect 'get', 'set', 'willSet', or "
                                     "'didSet' in property accessor.");
          }
        }
        consume(TokenType::RBrace, "Expect '}' after property accessors.");
      }

      StructMember member(memberName, memberType, std::move(defaultValue),
                          isVar, memberAccessLevel, memberSetterAccessLevel);
      member.accessors = std::move(accessors);
      members.push_back(std::move(member));

      match({TokenType::Semicolon}); // Optional semicolon
    } else {
      throw std::runtime_error(
          "Expect 'var', 'let', 'func', 'init', 'deinit', 'subscript', "
          "'struct', 'class', or 'enum' in struct body.");
    }
  }

  consume(TokenType::RBrace, "Expect '}' after struct body.");
  return std::make_unique<StructStmt>(
      name, std::move(members), std::move(methods), std::move(initializers),
      std::move(deinitializer), std::move(subscripts),
      std::move(conformedProtocols), AccessLevel::INTERNAL,
      std::move(genericParams), std::move(whereClause), std::move(nestedTypes));
}

// Parse class declaration: class Name<T>: Superclass, Protocol1, Protocol2 {
// var member1: Type, func method() {} }
std::unique_ptr<Stmt> Parser::classDeclaration() {
  consume(TokenType::Identifier, "Expect class name.");
  Token name = previous();

  // Parse optional generic parameter clause
  GenericParameterClause genericParams = parseGenericParameterClause();

  std::vector<std::unique_ptr<Stmt>> nestedTypes;

  Token superclass = Token(TokenType::Nil, "", name.line);
  std::vector<Token> conformedProtocols;

  if (match({TokenType::Colon})) {
    // First identifier could be superclass or protocol
    consume(TokenType::Identifier, "Expect superclass or protocol name.");
    Token firstInheritance = previous();

    if (match({TokenType::Comma})) {
      // First was superclass, rest are protocols
      superclass = firstInheritance;
      do {
        consume(TokenType::Identifier, "Expect protocol name.");
        conformedProtocols.push_back(previous());
      } while (match({TokenType::Comma}));
    } else {
      // Only one inheritance - could be superclass or protocol
      // For now, assume it's a superclass (can be enhanced later with type
      // checking)
      superclass = firstInheritance;
    }
  }

  // Parse optional where clause
  WhereClause whereClause = parseWhereClause();

  consume(TokenType::LBrace, "Expect '{' after class name.");

  std::vector<StructMember> members;
  std::vector<std::unique_ptr<FunctionStmt>> methods;
  std::vector<std::unique_ptr<InitStmt>> initializers;
  std::unique_ptr<DeinitStmt> deinitializer;
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts;

  while (!check(TokenType::RBrace) && !isAtEnd()) {
    // Parse optional access level modifier for class members
    AccessLevel memberAccessLevel = AccessLevel::INTERNAL;
    AccessLevel memberSetterAccessLevel = AccessLevel::INTERNAL;

    if (isAccessLevelToken(peek().type)) {
      auto accessPair = parseAccessLevelWithSetter();
      memberAccessLevel = accessPair.first;
      memberSetterAccessLevel = accessPair.second;
    }

    // Check for override keyword before func
    bool isOverride = false;
    if (match({TokenType::Override})) {
      isOverride = true;
    }

    if (match({TokenType::Func})) {
      // Parse method declaration
      auto method = std::unique_ptr<FunctionStmt>(
          static_cast<FunctionStmt *>(functionDeclaration().release()));
      method->accessLevel = memberAccessLevel;
      method->isOverride = isOverride;
      methods.push_back(std::move(method));
    } else if (isOverride) {
      throw std::runtime_error("Expect 'func' after 'override'.");
    } else if (match({TokenType::Init})) {
      // Parse initializer declaration
      auto init = std::unique_ptr<InitStmt>(
          static_cast<InitStmt *>(initDeclaration().release()));
      init->accessLevel = memberAccessLevel;
      initializers.push_back(std::move(init));
    } else if (match({TokenType::Deinit})) {
      // Parse deinitializer declaration
      if (deinitializer) {
        throw std::runtime_error("Multiple deinitializers not allowed.");
      }
      deinitializer = std::unique_ptr<DeinitStmt>(
          static_cast<DeinitStmt *>(deinitDeclaration().release()));
    } else if (match({TokenType::Subscript})) {
      // Parse subscript declaration
      auto subscript = std::unique_ptr<SubscriptStmt>(
          static_cast<SubscriptStmt *>(subscriptDeclaration().release()));
      subscript->accessLevel = memberAccessLevel;
      subscript->setterAccessLevel = memberSetterAccessLevel;
      subscripts.push_back(std::move(subscript));
    } else if (match({TokenType::Struct})) {
      // Parse nested struct declaration
      auto nestedStruct = structDeclaration();
      nestedTypes.push_back(std::move(nestedStruct));
    } else if (match({TokenType::Class})) {
      // Parse nested class declaration
      auto nestedClass = classDeclaration();
      nestedTypes.push_back(std::move(nestedClass));
    } else if (match({TokenType::Enum})) {
      // Parse nested enum declaration
      auto nestedEnum = enumDeclaration();
      nestedTypes.push_back(std::move(nestedEnum));
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

      members.emplace_back(memberName, memberType, std::move(defaultValue),
                           isVar, memberAccessLevel, memberSetterAccessLevel);

      match({TokenType::Semicolon}); // Optional semicolon
    } else {
      throw std::runtime_error(
          "Expect 'var', 'let', 'func', 'init', 'deinit', 'subscript', "
          "'struct', 'class', or 'enum' in class body.");
    }
  }

  consume(TokenType::RBrace, "Expect '}' after class body.");
  return std::make_unique<ClassStmt>(
      name, superclass, std::move(members), std::move(methods),
      std::move(initializers), std::move(deinitializer), std::move(subscripts),
      std::move(conformedProtocols), AccessLevel::INTERNAL,
      std::move(genericParams), std::move(whereClause), std::move(nestedTypes));
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

// Parse init declaration: init(parameters) { body } or init?(parameters) { body
// }
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
      // Check for inout keyword
      bool isInout = false;
      if (match({TokenType::Inout})) {
        isInout = true;
      }

      consume(TokenType::Identifier, "Expect parameter name.");
      Token paramName = previous();
      consume(TokenType::Colon, "Expect ':' after parameter name.");
      Token paramType = parseType();
      parameters.emplace_back(paramName, paramType, isInout);
    } while (match({TokenType::Comma}));
  }

  consume(TokenType::RParen, "Expect ')' after parameters.");

  consume(TokenType::LBrace, "Expect '{' before initializer body.");
  auto body = blockStatement();

  InitType initType = isFailable ? InitType::FAILABLE : InitType::DESIGNATED;

  return std::make_unique<InitStmt>(initType, std::move(parameters),
                                    std::move(body), false);
}

// Parse deinit declaration: deinit { body }
std::unique_ptr<Stmt> Parser::deinitDeclaration() {
  consume(TokenType::LBrace, "Expect '{' before deinitializer body.");
  auto body = blockStatement();

  return std::make_unique<DeinitStmt>(std::move(body));
}

// Parse subscript declaration: subscript(parameters) -> ReturnType { get { }
// set { } }
std::unique_ptr<Stmt> Parser::subscriptDeclaration() {
  bool isStatic = false;
  // Check if this is a static subscript (would be handled by caller)

  consume(TokenType::LParen, "Expect '(' after 'subscript'.");

  std::vector<Parameter> parameters;
  if (!check(TokenType::RParen)) {
    do {
      // Check for inout keyword
      bool isInout = false;
      if (match({TokenType::Inout})) {
        isInout = true;
      }

      consume(TokenType::Identifier, "Expect parameter name.");
      Token paramName = previous();
      consume(TokenType::Colon, "Expect ':' after parameter name.");
      Token paramType = parseType();
      parameters.emplace_back(paramName, paramType, isInout);
    } while (match({TokenType::Comma}));
  }

  consume(TokenType::RParen, "Expect ')' after parameters.");
  consume(TokenType::Arrow, "Expect '->' after subscript parameters.");
  Token returnType = parseType();

  consume(TokenType::LBrace, "Expect '{' before subscript body.");

  std::vector<PropertyAccessor> accessors;

  // Check if this is a computed subscript (direct statements) or accessor-based
  // subscript (get/set)
  if (check(TokenType::Get) || check(TokenType::Set)) {
    // Accessor-based subscript
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
        throw std::runtime_error(
            "Expect 'get' or 'set' in subscript accessor.");
      }
    }
  } else {
    // Computed subscript - parse as a single block of statements
    auto body = blockStatement();
    // Create a getter accessor with the computed body
    accessors.emplace_back(AccessorType::GET, std::move(body));
    // The blockStatement already consumed the closing brace, so we need to
    // return early
    return std::make_unique<SubscriptStmt>(std::move(parameters), returnType,
                                           std::move(accessors), isStatic);
  }

  consume(TokenType::RBrace, "Expect '}' after subscript body.");

  return std::make_unique<SubscriptStmt>(std::move(parameters), returnType,
                                         std::move(accessors), isStatic);
}

// Parse access level modifier
AccessLevel Parser::parseAccessLevel() {
  if (isAccessLevelToken(peek().type)) {
    Token token = advance();
    return tokenToAccessLevel(token.type);
  }
  return AccessLevel::INTERNAL; // Default access level
}

// Parse access level with optional setter access level (e.g., private(set))
std::pair<AccessLevel, AccessLevel> Parser::parseAccessLevelWithSetter() {
  AccessLevel mainAccess = AccessLevel::INTERNAL;
  AccessLevel setterAccess = AccessLevel::INTERNAL;

  if (isAccessLevelToken(peek().type)) {
    Token token = advance();
    mainAccess = tokenToAccessLevel(token.type);
    setterAccess = mainAccess; // Default setter access is same as main access

    // Check for setter-specific access level: private(set), fileprivate(set),
    // etc.
    if (match({TokenType::LParen})) {
      if (match({TokenType::Set})) {
        consume(TokenType::RParen, "Expect ')' after 'set'.");
        // For private(set), fileprivate(set), etc., the setter has more
        // restrictive access
        setterAccess = mainAccess;
      } else {
        throw std::runtime_error(
            "Only 'set' is allowed in access level parentheses.");
      }
    }
  }

  return std::make_pair(mainAccess, setterAccess);
}

// Check if a token type represents an access level
bool Parser::isAccessLevelToken(TokenType type) {
  return type == TokenType::Open || type == TokenType::Public ||
         type == TokenType::Package || type == TokenType::Internal ||
         type == TokenType::Fileprivate || type == TokenType::Private;
}

// Parse protocol declaration: protocol Name: ParentProtocol { requirements }
std::unique_ptr<Stmt> Parser::protocolDeclaration() {
  consume(TokenType::Identifier, "Expect protocol name.");
  Token name = previous();

  std::cout << "Parsing protocol: " << name.lexeme << std::endl;

  // Parse protocol inheritance
  std::vector<Token> inheritedProtocols;
  if (match({TokenType::Colon})) {
    do {
      consume(TokenType::Identifier, "Expect protocol name.");
      inheritedProtocols.push_back(previous());
    } while (match({TokenType::Comma}));
  }

  consume(TokenType::LBrace, "Expect '{' after protocol name.");

  std::vector<ProtocolRequirement> requirements;

  while (!check(TokenType::RBrace) && !isAtEnd()) {
    // Parse optional access level modifier for protocol requirements
    AccessLevel requirementAccessLevel = AccessLevel::INTERNAL;
    AccessLevel requirementSetterAccessLevel = AccessLevel::INTERNAL;

    if (isAccessLevelToken(peek().type)) {
      auto accessPair = parseAccessLevelWithSetter();
      requirementAccessLevel = accessPair.first;
      requirementSetterAccessLevel = accessPair.second;
    }

    if (match({TokenType::Func})) {
      // Parse method requirement
      consume(TokenType::Identifier, "Expect method name.");
      Token methodName = previous();

      consume(TokenType::LParen, "Expect '(' after method name.");

      std::vector<Parameter> parameters;
      if (!check(TokenType::RParen)) {
        do {
          // Check for inout keyword
          bool isInout = false;
          if (match({TokenType::Inout})) {
            isInout = true;
          }

          consume(TokenType::Identifier, "Expect parameter name.");
          Token paramName = previous();
          consume(TokenType::Colon, "Expect ':' after parameter name.");
          Token paramType = parseType();
          parameters.emplace_back(paramName, paramType, isInout);
        } while (match({TokenType::Comma}));
      }

      consume(TokenType::RParen, "Expect ')' after parameters.");

      Token returnType = Token(TokenType::Unknown, "Void", name.line);
      if (match({TokenType::Arrow})) {
        returnType = parseType();
      }

      auto methodReq = std::make_unique<ProtocolMethodRequirement>(
          methodName, std::move(parameters), returnType, false, false);

      ProtocolRequirement requirement(std::move(methodReq));
      requirements.push_back(std::move(requirement));

    } else if (match({TokenType::Var, TokenType::Let})) {
      // Parse property requirement
      bool isVar = previous().type == TokenType::Var;

      consume(TokenType::Identifier, "Expect property name.");
      Token propertyName = previous();

      consume(TokenType::Colon, "Expect ':' after property name.");
      Token propertyType = parseType();

      consume(TokenType::LBrace, "Expect '{' after property type.");

      bool hasGetter = false;
      bool hasSetter = false;

      while (!check(TokenType::RBrace) && !isAtEnd()) {
        if (match({TokenType::Get})) {
          hasGetter = true;
        } else if (match({TokenType::Set})) {
          hasSetter = true;
        } else {
          throw std::runtime_error(
              "Expect 'get' or 'set' in property requirement.");
        }
      }

      consume(TokenType::RBrace, "Expect '}' after property accessors.");

      auto propReq = std::make_unique<ProtocolPropertyRequirement>(
          propertyName, propertyType, isVar, false, hasGetter, hasSetter);

      ProtocolRequirement requirement(std::move(propReq));
      requirements.push_back(std::move(requirement));

    } else if (match({TokenType::Init})) {
      // Parse initializer requirement
      bool isFailable = false;
      if (match({TokenType::Unknown}) && previous().lexeme == "?") {
        isFailable = true;
      }

      consume(TokenType::LParen, "Expect '(' after 'init'.");

      std::vector<Parameter> parameters;
      if (!check(TokenType::RParen)) {
        do {
          // Check for inout keyword
          bool isInout = false;
          if (match({TokenType::Inout})) {
            isInout = true;
          }

          consume(TokenType::Identifier, "Expect parameter name.");
          Token paramName = previous();
          consume(TokenType::Colon, "Expect ':' after parameter name.");
          Token paramType = parseType();
          parameters.emplace_back(paramName, paramType, isInout);
        } while (match({TokenType::Comma}));
      }

      consume(TokenType::RParen, "Expect ')' after parameters.");

      auto initReq = std::make_unique<ProtocolInitRequirement>(
          std::move(parameters), isFailable);

      ProtocolRequirement requirement(std::move(initReq));
      requirements.push_back(std::move(requirement));

    } else if (match({TokenType::Subscript})) {
      // Parse subscript requirement
      consume(TokenType::LParen, "Expect '(' after 'subscript'.");

      std::vector<Parameter> parameters;
      if (!check(TokenType::RParen)) {
        do {
          // Check for inout keyword
          bool isInout = false;
          if (match({TokenType::Inout})) {
            isInout = true;
          }

          consume(TokenType::Identifier, "Expect parameter name.");
          Token paramName = previous();
          consume(TokenType::Colon, "Expect ':' after parameter name.");
          Token paramType = parseType();
          parameters.emplace_back(paramName, paramType, isInout);
        } while (match({TokenType::Comma}));
      }

      consume(TokenType::RParen, "Expect ')' after parameters.");
      consume(TokenType::Arrow, "Expect '->' after subscript parameters.");
      Token returnType = parseType();

      consume(TokenType::LBrace, "Expect '{' after subscript return type.");

      bool hasGetter = false;
      bool hasSetter = false;

      while (!check(TokenType::RBrace) && !isAtEnd()) {
        if (match({TokenType::Get})) {
          hasGetter = true;
        } else if (match({TokenType::Set})) {
          hasSetter = true;
        } else {
          throw std::runtime_error(
              "Expect 'get' or 'set' in subscript requirement.");
        }
      }

      consume(TokenType::RBrace, "Expect '}' after subscript accessors.");

      auto subscriptReq = std::make_unique<ProtocolSubscriptRequirement>(
          std::move(parameters), returnType, false, hasGetter, hasSetter);

      ProtocolRequirement requirement(std::move(subscriptReq));
      requirements.push_back(std::move(requirement));

    } else {
      throw std::runtime_error("Expect 'var', 'let', 'func', 'init', or "
                               "'subscript' in protocol body.");
    }
  }

  consume(TokenType::RBrace, "Expect '}' after protocol body.");
  return std::make_unique<ProtocolStmt>(name, std::move(inheritedProtocols),
                                        std::move(requirements));
}

// Parse generic parameter clause: <T, U: Equatable, V: Collection>
GenericParameterClause Parser::parseGenericParameterClause() {
  std::vector<TypeParameter> parameters;

  if (!check(TokenType::LAngle)) {
    return GenericParameterClause(std::move(parameters));
  }

  consume(TokenType::LAngle, "Expect '<' to start generic parameter clause.");

  if (!check(TokenType::RAngle)) {
    do {
      auto typeParam = parseTypeParameter();
      parameters.push_back(std::move(typeParam));
    } while (match({TokenType::Comma}));
  }

  consume(TokenType::RAngle, "Expect '>' to end generic parameter clause.");
  return GenericParameterClause(std::move(parameters));
}

// Parse type parameter: T or T: Equatable
TypeParameter Parser::parseTypeParameter() {
  consume(TokenType::Identifier, "Expect type parameter name.");
  Token name = previous();

  std::vector<TypeConstraint> constraints;

  if (match({TokenType::Colon})) {
    do {
      // Parse constraint type directly since we already have the type name
      consume(TokenType::Identifier, "Expect constraint type name.");
      Token constraintType = previous();
      TypeConstraint constraint(name, ConstraintType::PROTOCOL_CONSTRAINT,
                                constraintType);
      constraints.push_back(std::move(constraint));
    } while (match({TokenType::Amp})); // Multiple constraints separated by &
  }

  return TypeParameter(name, std::move(constraints));
}

// Parse where clause: where T: Equatable, U.Element == String
WhereClause Parser::parseWhereClause() {
  std::vector<TypeConstraint> constraints;

  if (!match({TokenType::Where})) {
    return WhereClause(std::move(constraints));
  }

  do {
    auto constraint = parseTypeConstraint();
    constraints.push_back(std::move(constraint));
  } while (match({TokenType::Comma}));

  return WhereClause(std::move(constraints));
}

// Parse type constraint: T: Equatable or T.Element == String
// Note: This method is called in two contexts:
// 1. From parseTypeParameter after consuming T and colon: current token is
// constraint type (Equatable)
// 2. From parseWhereClause: current token is type name (T)
TypeConstraint Parser::parseTypeConstraint() {
  consume(TokenType::Identifier, "Expect type name in constraint.");
  Token typeName = previous();

  if (match({TokenType::Colon})) {
    // Class or protocol constraint: T: SomeClass or T: SomeProtocol
    consume(TokenType::Identifier, "Expect constraint type name.");
    Token constraintType = previous();
    return TypeConstraint(typeName, ConstraintType::PROTOCOL_CONSTRAINT,
                          constraintType);
  } else if (match({TokenType::Dot})) {
    // Associated type constraint: T.Element == String
    consume(TokenType::Identifier, "Expect associated type name.");
    Token associatedType = previous();
    consume(TokenType::EqualEqual, "Expect '==' in same-type constraint.");
    consume(TokenType::Identifier, "Expect type name in same-type constraint.");
    Token sameType = previous();

    // Create a compound type name for associated type
    Token compoundType =
        Token(TokenType::Identifier,
              typeName.lexeme + "." + associatedType.lexeme, typeName.line);
    return TypeConstraint(compoundType, ConstraintType::SAME_TYPE_CONSTRAINT,
                          sameType);
  } else {
    throw std::runtime_error("Expect ':' or '.' in type constraint.");
  }
}

// Parse generic type: Array<String> or Dictionary<String, Int>
GenericTypeReference Parser::parseGenericType() {
  consume(TokenType::Identifier, "Expect generic type name.");
  Token typeName = previous();

  std::vector<Token> typeArguments;

  if (match({TokenType::LAngle})) {
    if (!check(TokenType::RAngle)) {
      do {
        Token typeArg = parseType();
        typeArguments.push_back(typeArg);
      } while (match({TokenType::Comma}));
    }
    consume(TokenType::RAngle, "Expect '>' after generic type arguments.");
  }

  return GenericTypeReference(typeName, std::move(typeArguments));
}

// Parse generic type instantiation: MyStruct<Int, String>
GenericTypeInstantiation Parser::parseGenericTypeInstantiation() {
  consume(TokenType::Identifier, "Expect generic type name.");
  Token typeName = previous();

  std::vector<Token> typeArguments;

  consume(TokenType::LAngle, "Expect '<' after generic type name.");

  if (!check(TokenType::RAngle)) {
    do {
      Token typeArg = parseType();
      typeArguments.push_back(typeArg);
    } while (match({TokenType::Comma}));
  }

  consume(TokenType::RAngle, "Expect '>' after generic type arguments.");
  return GenericTypeInstantiation(typeName, std::move(typeArguments));
}

// Parse extension declaration: extension TypeName: Protocol1, Protocol2 where
// T: Equatable { members }
std::unique_ptr<Stmt> Parser::extensionDeclaration() {
  consume(TokenType::Identifier, "Expect type name after 'extension'.");
  Token typeName = previous();

  // std::cout << "Parsing extension for: " << typeName.lexeme << std::endl;

  // Parse optional generic parameter clause
  GenericParameterClause genericParams = parseGenericParameterClause();

  // Parse protocol conformance
  std::vector<Token> conformedProtocols;
  if (match({TokenType::Colon})) {
    do {
      consume(TokenType::Identifier, "Expect protocol name.");
      conformedProtocols.push_back(previous());
    } while (match({TokenType::Comma}));
  }

  // Parse optional where clause
  WhereClause whereClause = parseWhereClause();

  consume(TokenType::LBrace, "Expect '{' after extension declaration.");

  std::vector<StructMember> computedProperties;
  std::vector<std::unique_ptr<FunctionStmt>> methods;
  std::vector<std::unique_ptr<InitStmt>> initializers;
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts;

  while (!check(TokenType::RBrace) && !isAtEnd()) {
    // Parse optional access level modifier for extension members
    AccessLevel memberAccessLevel = AccessLevel::INTERNAL;
    AccessLevel memberSetterAccessLevel = AccessLevel::INTERNAL;

    if (isAccessLevelToken(peek().type)) {
      auto accessPair = parseAccessLevelWithSetter();
      memberAccessLevel = accessPair.first;
      memberSetterAccessLevel = accessPair.second;
    }

    // Check for mutating keyword before func
    bool isMutating = false;
    if (match({TokenType::Mutating})) {
      isMutating = true;
    }

    if (match({TokenType::Func})) {
      // Parse method declaration
      auto method = std::unique_ptr<FunctionStmt>(
          static_cast<FunctionStmt *>(functionDeclaration().release()));
      method->accessLevel = memberAccessLevel;
      method->isMutating = isMutating;
      methods.push_back(std::move(method));
    } else if (match({TokenType::Init})) {
      // Parse initializer declaration (convenience initializers only)
      auto init = std::unique_ptr<InitStmt>(
          static_cast<InitStmt *>(initDeclaration().release()));
      init->accessLevel = memberAccessLevel;
      init->initType = InitType::CONVENIENCE; // Extensions can only add
                                              // convenience initializers
      initializers.push_back(std::move(init));
    } else if (match({TokenType::Subscript})) {
      // Parse subscript declaration
      auto subscript = std::unique_ptr<SubscriptStmt>(
          static_cast<SubscriptStmt *>(subscriptDeclaration().release()));
      subscript->accessLevel = memberAccessLevel;
      subscript->setterAccessLevel = memberSetterAccessLevel;
      subscripts.push_back(std::move(subscript));
    } else if (match({TokenType::Var})) {
      // Parse computed property (extensions cannot add stored properties)
      consume(TokenType::Identifier, "Expect property name.");
      Token propertyName = previous();

      consume(TokenType::Colon, "Expect ':' after property name.");
      Token propertyType = parseType();

      consume(TokenType::LBrace, "Expect '{' after property type.");

      std::vector<PropertyAccessor> accessors;

      while (!check(TokenType::RBrace) && !isAtEnd()) {
        if (match({TokenType::Get})) {
          consume(TokenType::LBrace, "Expect '{' after 'get'.");
          auto body = blockStatement();
          accessors.emplace_back(AccessorType::GET, std::move(body));
        } else if (match({TokenType::Set})) {
          consume(TokenType::LBrace, "Expect '{' after 'set'.");
          auto body = blockStatement();
          accessors.emplace_back(AccessorType::SET, std::move(body));
        } else {
          throw std::runtime_error(
              "Expect 'get' or 'set' in computed property.");
        }
      }

      consume(TokenType::RBrace, "Expect '}' after property accessors.");

      // Create computed property member
      StructMember member(propertyName, propertyType, nullptr, true,
                          memberAccessLevel, memberSetterAccessLevel);
      member.accessors = std::move(accessors);
      computedProperties.push_back(std::move(member));
    } else {
      // If we found mutating but no valid declaration, that's an error
      if (isMutating) {
        throw std::runtime_error(
            "Expect 'func' after 'mutating' in extension.");
      }
      throw std::runtime_error(
          "Expect 'var', 'func', 'init', or 'subscript' in extension body.");
    }
  }

  consume(TokenType::RBrace, "Expect '}' after extension body.");

  return std::make_unique<ExtensionStmt>(
      typeName, std::move(conformedProtocols), std::move(genericParams),
      std::move(whereClause), AccessLevel::INTERNAL,
      std::move(computedProperties), std::move(methods),
      std::move(initializers), std::move(subscripts));
}

// Error handling statement parsing methods
std::unique_ptr<Stmt> Parser::throwStatement() {
  auto errorExpr = expression();
  match({TokenType::Semicolon}); // Optional semicolon
  return std::make_unique<ThrowStmt>(std::move(errorExpr));
}

std::unique_ptr<Stmt> Parser::doCatchStatement() {
  // Consume the opening brace for do block
  consume(TokenType::LBrace, "Expect '{' after 'do'.");

  // Parse do block
  auto doBlock = blockStatement();

  // Parse catch clauses
  std::vector<CatchClause> catchClauses;

  while (match({TokenType::Catch})) {
    std::optional<Token> errorType;
    std::optional<Token> errorVariable;

    // Check for error type pattern: catch ErrorType.case or catch
    // ErrorType.case(let variable)
    if (check(TokenType::Identifier)) {
      Token firstToken = advance();

      if (match({TokenType::Dot})) {
        // Pattern: catch ErrorType.case
        consume(TokenType::Identifier, "Expect error case after '.'.");
        Token caseToken = previous();
        errorType =
            Token(TokenType::Identifier,
                  firstToken.lexeme + "." + caseToken.lexeme, firstToken.line);

        // Check for associated value binding: (let variable)
        if (match({TokenType::LParen})) {
          if (match({TokenType::Let})) {
            consume(TokenType::Identifier, "Expect variable name after 'let'.");
            errorVariable = previous();
          }
          consume(TokenType::RParen,
                  "Expect ')' after associated value binding.");
        }
      } else {
        // Put the firstToken back since we didn't consume it with a dot
        current--;
        // Just error type: catch ErrorType
        errorType = firstToken;
      }
    }

    consume(TokenType::LBrace, "Expect '{' after catch clause.");
    auto catchBlock = blockStatement();

    // Create CatchClause with all required parameters
    Token pattern = errorType.value_or(Token(TokenType::Identifier, "", 0));
    std::string errorTypeStr = errorType ? errorType->lexeme : "";
    Token variable =
        errorVariable.value_or(Token(TokenType::Identifier, "", 0));
    std::unique_ptr<Expr> condition = nullptr; // No where condition for now

    catchClauses.emplace_back(pattern, errorTypeStr, variable,
                              std::move(condition), std::move(catchBlock));
  }

  return std::make_unique<DoCatchStmt>(std::move(doBlock),
                                       std::move(catchClauses));
}

std::unique_ptr<Stmt> Parser::deferStatement() {
  consume(TokenType::LBrace, "Expect '{' after 'defer'.");
  auto deferBlock = blockStatement();
  return std::make_unique<DeferStmt>(std::move(deferBlock));
}

std::unique_ptr<Stmt> Parser::guardStatement() {
  // Check if this is a "guard let" statement
  if (check(TokenType::Let)) {
    // Parse "guard let variable = expression"
    consume(TokenType::Let, "Expect 'let' in guard-let statement.");
    consume(TokenType::Identifier, "Expect variable name after 'let'.");
    Token variable = previous();

    consume(TokenType::Equal,
            "Expect '=' after variable in guard-let statement.");
    auto expression = this->expression();

    consume(TokenType::Else, "Expect 'else' after guard-let condition.");
    consume(TokenType::LBrace, "Expect '{' after 'else'.");
    auto elseBlock = blockStatement();

    return std::make_unique<GuardLetStmt>(variable, std::move(expression),
                                          std::move(elseBlock));
  } else {
    // Regular guard statement
    auto condition = expression();
    consume(TokenType::Else, "Expect 'else' after guard condition.");
    consume(TokenType::LBrace, "Expect '{' after 'else'.");
    auto elseBlock = blockStatement();
    return std::make_unique<GuardStmt>(std::move(condition),
                                       std::move(elseBlock));
  }
}

std::unique_ptr<Stmt> Parser::switchStatement() {
  auto expression = this->expression();
  consume(TokenType::LBrace, "Expect '{' after switch expression.");

  std::vector<SwitchCase> cases;

  while (!check(TokenType::RBrace) && !isAtEnd()) {
    if (match({TokenType::Case})) {
      // Parse case pattern
      auto pattern = this->expression();
      consume(TokenType::Colon, "Expect ':' after case pattern.");

      // Parse case statements
      std::vector<std::unique_ptr<Stmt>> statements;
      while (!check(TokenType::Case) && !check(TokenType::Default) &&
             !check(TokenType::RBrace) && !isAtEnd()) {
        statements.push_back(statement());
      }

      cases.emplace_back(std::move(pattern), std::move(statements), false);
    } else if (match({TokenType::Default})) {
      consume(TokenType::Colon, "Expect ':' after 'default'.");

      // Parse default statements
      std::vector<std::unique_ptr<Stmt>> statements;
      while (!check(TokenType::Case) && !check(TokenType::Default) &&
             !check(TokenType::RBrace) && !isAtEnd()) {
        statements.push_back(statement());
      }

      cases.emplace_back(nullptr, std::move(statements), true);
    } else {
      throw std::runtime_error(
          "Expect 'case' or 'default' in switch statement.");
    }
  }

  consume(TokenType::RBrace, "Expect '}' after switch body.");
  return std::make_unique<SwitchStmt>(std::move(expression), std::move(cases));
}

// Error handling expression parsing methods
std::unique_ptr<Expr> Parser::tryExpression() {
  bool isOptional = false;
  bool isForced = false;

  if (match({TokenType::Unknown})) {
    if (previous().lexeme == "?") {
      isOptional = true;
    } else if (previous().lexeme == "!") {
      isForced = true;
    } else {
      // Put the token back
      current--;
    }
  }

  auto expression = unary();
  return std::make_unique<TryExpr>(std::move(expression), isOptional, isForced);
}

std::unique_ptr<Expr> Parser::resultTypeExpression() {
  consume(TokenType::LAngle, "Expect '<' after 'Result'.");
  Token successType = parseType();
  consume(TokenType::Comma, "Expect ',' between Result type parameters.");
  Token errorType = parseType();
  consume(TokenType::RAngle, "Expect '>' after Result type parameters.");

  // Create literal expressions for the types
  auto successExpr = std::make_unique<Literal>(successType);
  auto errorExpr = std::make_unique<Literal>(errorType);

  return std::make_unique<ResultTypeExpr>(std::move(successExpr),
                                          std::move(errorExpr));
}

std::unique_ptr<Expr> Parser::errorLiteral() {
  consume(TokenType::Identifier, "Expect error type name.");
  Token errorType = previous();

  consume(TokenType::LParen, "Expect '(' after error type.");
  consume(TokenType::StringLiteral, "Expect error message string.");
  Token message = previous();

  std::vector<std::unique_ptr<Expr>> arguments;

  if (match({TokenType::Comma})) {
    do {
      auto argValue = expression();
      arguments.push_back(std::move(argValue));
    } while (match({TokenType::Comma}));
  }

  consume(TokenType::RParen, "Expect ')' after error literal.");

  return std::make_unique<ErrorLiteral>(errorType.lexeme, message.lexeme,
                                        std::move(arguments));
}

// Custom operator declaration parsing
std::unique_ptr<miniswift::Stmt>
miniswift::Parser::customOperatorDeclaration() {
  Token operatorType = previous(); // infix, prefix, or postfix
  consume(TokenType::Operator, "Expect 'operator' keyword.");

  // Accept various operator symbols including custom ones
  if (!match({TokenType::Identifier, TokenType::Star, TokenType::Plus,
              TokenType::Minus, TokenType::Slash, TokenType::Percent,
              TokenType::LAngle, TokenType::RAngle, TokenType::Bang,
              TokenType::Equal, TokenType::BitwiseAnd, TokenType::BitwiseOr,
              TokenType::BitwiseXor, TokenType::BitwiseNot, TokenType::StarStar,
              TokenType::LAngleRAngle, TokenType::PlusMinus,
              TokenType::CustomOperator})) {
    throw std::runtime_error("Expect operator symbol.");
  }

  Token operatorSymbol = previous();

  std::optional<Token> precedenceGroup;
  if (match({TokenType::Colon})) {
    consume(TokenType::Identifier, "Expect precedence group name.");
    precedenceGroup = previous();
  }

  match({TokenType::Semicolon}); // Optional semicolon
  return std::make_unique<CustomOperatorStmt>(operatorSymbol, operatorType);
}

// Operator precedence declaration parsing
std::unique_ptr<miniswift::Stmt>
miniswift::Parser::operatorPrecedenceDeclaration() {
  consume(TokenType::Identifier, "Expect precedence group name.");
  Token groupName = previous();

  consume(TokenType::LBrace, "Expect '{' after precedence group name.");

  std::string associativity = "none";
  int precedence = 100;
  std::vector<Token> higherThan;
  std::vector<Token> lowerThan;

  while (!check(TokenType::RBrace) && !isAtEnd()) {
    if (match({TokenType::Identifier})) {
      Token property = previous();
      if (property.lexeme == "associativity") {
        consume(TokenType::Colon, "Expect ':' after 'associativity'.");
        consume(TokenType::Identifier, "Expect associativity value.");
        associativity = previous().lexeme;
      } else if (property.lexeme == "precedence") {
        consume(TokenType::Colon, "Expect ':' after 'precedence'.");
        consume(TokenType::IntegerLiteral, "Expect precedence value.");
        precedence = std::stoi(previous().lexeme);
      } else if (property.lexeme == "higherThan") {
        consume(TokenType::Colon, "Expect ':' after 'higherThan'.");
        consume(TokenType::Identifier, "Expect precedence group name.");
        higherThan.push_back(previous());
      } else if (property.lexeme == "lowerThan") {
        consume(TokenType::Colon, "Expect ':' after 'lowerThan'.");
        consume(TokenType::Identifier, "Expect precedence group name.");
        lowerThan.push_back(previous());
      } else {
        // Skip unknown property
        advance();
      }
    } else {
      // Skip non-identifier tokens
      advance();
    }
    match({TokenType::Semicolon}); // Optional semicolon
  }

  consume(TokenType::RBrace, "Expect '}' after precedence group body.");

  // Create Token objects for the constructor
  Token associativityToken(TokenType::Identifier, associativity, 0);

  return std::make_unique<OperatorPrecedenceStmt>(
      groupName, associativityToken, precedence, higherThan, lowerThan);
}

// Result Builder declaration parsing
std::unique_ptr<miniswift::Stmt> miniswift::Parser::resultBuilderDeclaration() {
  consume(TokenType::Struct, "Expect 'struct' after '@resultBuilder'.");
  consume(TokenType::Identifier, "Expect Result Builder name.");
  Token builderName = previous();

  consume(TokenType::LBrace, "Expect '{' after Result Builder name.");

  std::vector<std::unique_ptr<FunctionStmt>> methods;

  while (!check(TokenType::RBrace) && !isAtEnd()) {
    if (match({TokenType::Static})) {
      if (match({TokenType::Func})) {
        auto method = functionDeclaration();
        // Cast Stmt to FunctionStmt
        auto funcStmt = std::unique_ptr<FunctionStmt>(
            static_cast<FunctionStmt *>(method.release()));
        methods.push_back(std::move(funcStmt));
      } else {
        throw std::runtime_error(
            "Expect 'func' after 'static' in Result Builder.");
      }
    } else {
      throw std::runtime_error("Expect 'static func' in Result Builder body.");
    }
  }

  consume(TokenType::RBrace, "Expect '}' after Result Builder body.");
  return std::make_unique<ResultBuilderStmt>(builderName, std::move(methods));
}

// Helper method to check if a token is a custom operator
bool miniswift::Parser::isCustomOperator(const std::string &lexeme) {
  // Custom operators typically contain special characters
  // This is a simplified check - in a real implementation, you'd maintain
  // a registry of declared custom operators
  return lexeme.find_first_of("+-*/%<>=!&|^~") != std::string::npos &&
         lexeme != "+" && lexeme != "-" && lexeme != "*" && lexeme != "/" &&
         lexeme != "%" && lexeme != "<" && lexeme != ">" && lexeme != "=" &&
         lexeme != "!" && lexeme != "&" && lexeme != "|" && lexeme != "^" &&
         lexeme != "~";
}

// Helper method to check if a token is a custom prefix operator
bool miniswift::Parser::isCustomPrefixOperator(const std::string &lexeme) {
  // For now, we'll use a simple registry of known prefix operators
  // In a real implementation, this would be populated by operator declarations
  static std::set<std::string> prefixOperators = {"±"};
  return prefixOperators.find(lexeme) != prefixOperators.end();
}

// Parse actor declaration: actor Name { properties and methods }
std::unique_ptr<Stmt> Parser::actorDeclaration() {
  consume(TokenType::Identifier, "Expect actor name.");
  Token name = previous();

  // Parse optional generic parameter clause
  GenericParameterClause genericParams = parseGenericParameterClause();

  // Parse protocol conformance
  std::vector<Token> conformedProtocols;
  if (match({TokenType::Colon})) {
    do {
      consume(TokenType::Identifier, "Expect protocol name.");
      conformedProtocols.push_back(previous());
    } while (match({TokenType::Comma}));
  }

  // Parse optional where clause
  WhereClause whereClause = parseWhereClause();

  consume(TokenType::LBrace, "Expect '{' after actor declaration.");

  std::vector<StructMember> properties;
  std::vector<std::unique_ptr<FunctionStmt>> methods;
  std::vector<std::unique_ptr<InitStmt>> initializers;
  std::vector<std::unique_ptr<SubscriptStmt>> subscripts;
  bool isGlobalActor = false;

  while (!check(TokenType::RBrace) && !isAtEnd()) {
    // Parse optional access level modifier for actor members
    AccessLevel memberAccessLevel = AccessLevel::INTERNAL;
    AccessLevel memberSetterAccessLevel = AccessLevel::INTERNAL;

    if (isAccessLevelToken(peek().type)) {
      auto accessPair = parseAccessLevelWithSetter();
      memberAccessLevel = accessPair.first;
      memberSetterAccessLevel = accessPair.second;
    }

    // Check for isolated/nonisolated keywords
    bool isIsolated = true; // Default for actor members
    if (match({TokenType::Nonisolated})) {
      isIsolated = false;
    } else if (match({TokenType::Isolated})) {
      isIsolated = true;
    }

    // Check for mutating keyword before func
    bool isMutating = false;
    if (match({TokenType::Mutating})) {
      isMutating = true;
    }

    if (match({TokenType::Func})) {
      // Parse method declaration
      auto method = std::unique_ptr<FunctionStmt>(
          static_cast<FunctionStmt *>(functionDeclaration().release()));
      method->accessLevel = memberAccessLevel;
      method->isMutating = isMutating;
      methods.push_back(std::move(method));
    } else if (isMutating) {
      throw std::runtime_error("Expect 'func' after 'mutating'.");
    } else if (match({TokenType::Init})) {
      // Parse initializer declaration
      auto init = std::unique_ptr<InitStmt>(
          static_cast<InitStmt *>(initDeclaration().release()));
      init->accessLevel = memberAccessLevel;
      initializers.push_back(std::move(init));
    } else if (match({TokenType::Subscript})) {
      // Parse subscript declaration
      auto subscript = std::unique_ptr<SubscriptStmt>(
          static_cast<SubscriptStmt *>(subscriptDeclaration().release()));
      subscript->accessLevel = memberAccessLevel;
      subscript->setterAccessLevel = memberSetterAccessLevel;
      subscripts.push_back(std::move(subscript));
    } else if (match({TokenType::Var, TokenType::Let})) {
      // Parse property declaration
      bool isVar = previous().type == TokenType::Var;

      consume(TokenType::Identifier, "Expect property name.");
      Token propertyName = previous();

      consume(TokenType::Colon, "Expect ':' after property name.");
      Token propertyType = parseType();

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
            accessors.emplace_back(AccessorType::SET, std::move(body),
                                   paramName);
          } else {
            throw std::runtime_error(
                "Expect 'get' or 'set' in property accessor.");
          }
        }
        consume(TokenType::RBrace, "Expect '}' after property accessors.");
      }

      StructMember member(propertyName, propertyType, std::move(defaultValue),
                          isVar, memberAccessLevel, memberSetterAccessLevel);
      member.accessors = std::move(accessors);
      properties.push_back(std::move(member));
    } else {
      throw std::runtime_error("Expect property, method, initializer, or "
                               "subscript in actor declaration.");
    }
  }

  consume(TokenType::RBrace, "Expect '}' after actor body.");

  return std::make_unique<ActorStmt>(
      name, std::move(properties), std::move(methods), std::move(initializers),
      AccessLevel::INTERNAL, isGlobalActor);
}

// Parse await expression: await expression
std::unique_ptr<Expr> Parser::awaitExpression() {
  Token awaitKeyword = previous(); // Get the 'await' token
  auto expression = unary();
  return std::make_unique<AwaitExpr>(awaitKeyword, std::move(expression));
}

// Parse task expression: Task { closure }
std::unique_ptr<Expr> Parser::taskExpression() {
  Token taskKeyword = previous(); // Get the 'Task' token
  consume(TokenType::LBrace, "Expect '{' after 'Task'.");

  // Parse the task body as a closure
  std::vector<std::unique_ptr<Stmt>> statements;
  while (!check(TokenType::RBrace) && !isAtEnd()) {
    statements.push_back(statement());
  }

  consume(TokenType::RBrace, "Expect '}' after task body.");

  // Create a closure expression with the task body
  auto taskBody = std::make_unique<Closure>(
      std::vector<Parameter>(), // No parameters for task closure
      Token(TokenType::Void, "Void", peek().line), // Return type is Void
      std::move(statements)                        // Pass statements directly
  );

  return std::make_unique<TaskExpr>(taskKeyword, TaskExpr::TaskType::Regular,
                                    std::move(taskBody));
}

// Parse macro declaration: macro Name(parameters) -> ReturnType =
// #externalMacro(module: "Module", type: "Type")
std::unique_ptr<Stmt> Parser::macroDeclaration() {
  consume(TokenType::Identifier, "Expect macro name.");
  Token name = previous();

  // Parse parameter list
  consume(TokenType::LParen, "Expect '(' after macro name.");
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

  // Parse return type
  Token returnType(TokenType::Void, "Void", peek().line);
  if (match({TokenType::Arrow})) {
    returnType = parseType();
  }

  // Parse macro body (either = #externalMacro or { body })
  std::unique_ptr<Expr> body = nullptr;
  if (match({TokenType::Equal})) {
    body = expression();
  } else if (match({TokenType::LBrace})) {
    // Parse macro body as block
    auto blockBody = blockStatement();
    // Convert block to expression (simplified)
    body = std::make_unique<Literal>(
        Token(TokenType::String, "block", peek().line));
  }

  return std::make_unique<MacroStmt>(name, std::move(parameters), returnType,
                                     std::move(body));
}

// Parse external macro declaration: externalMacro Name(parameters) ->
// ReturnType
std::unique_ptr<Stmt> Parser::externalMacroDeclaration() {
  consume(TokenType::Identifier, "Expect external macro name.");
  Token name = previous();

  // Parse parameter list
  consume(TokenType::LParen, "Expect '(' after external macro name.");
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

  // Parse return type
  Token returnType(TokenType::Void, "Void", peek().line);
  if (match({TokenType::Arrow})) {
    returnType = parseType();
  }

  // Parse module and type information
  std::string moduleName;
  std::string typeName;
  if (match({TokenType::LParen})) {
    if (match({TokenType::Identifier}) && previous().lexeme == "module") {
      consume(TokenType::Colon, "Expect ':' after 'module'.");
      consume(TokenType::String, "Expect module name string.");
      moduleName = previous().lexeme;
      if (match({TokenType::Comma})) {
        if (match({TokenType::Identifier}) && previous().lexeme == "type") {
          consume(TokenType::Colon, "Expect ':' after 'type'.");
          consume(TokenType::String, "Expect type name string.");
          typeName = previous().lexeme;
        }
      }
    }
    consume(TokenType::RParen, "Expect ')' after external macro info.");
  }

  return std::make_unique<ExternalMacroStmt>(name, std::move(parameters),
                                             returnType, moduleName, typeName);
}

// Parse freestanding macro declaration: @freestanding(expression) macro Name
std::unique_ptr<Stmt> Parser::freestandingMacroDeclaration() {
  consume(TokenType::LParen, "Expect '(' after '@freestanding'.");
  consume(TokenType::Identifier, "Expect macro role.");
  Token role = previous();
  consume(TokenType::RParen, "Expect ')' after macro role.");

  consume(TokenType::Macro, "Expect 'macro' after '@freestanding(...)'.");
  consume(TokenType::Identifier, "Expect freestanding macro name.");
  Token name = previous();

  // Parse parameter list
  consume(TokenType::LParen, "Expect '(' after freestanding macro name.");
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

  // Parse return type
  Token returnType(TokenType::Void, "Void", peek().line);
  if (match({TokenType::Arrow})) {
    returnType = parseType();
  }

  // Parse macro body
  std::unique_ptr<Expr> body = nullptr;
  if (match({TokenType::Equal})) {
    body = expression();
  }

  Token roleToken(TokenType::Identifier, role.lexeme, peek().line);
  return std::make_unique<FreestandingMacroStmt>(
      name, std::move(parameters), returnType, std::move(body),
      AccessLevel::INTERNAL, roleToken);
}

// Parse attached macro declaration: @attached(member) macro Name
std::unique_ptr<Stmt> Parser::attachedMacroDeclaration() {
  consume(TokenType::LParen, "Expect '(' after '@attached'.");
  consume(TokenType::Identifier, "Expect macro role.");
  Token role = previous();

  // Parse optional names list for member/memberAttribute roles
  std::vector<std::string> names;
  if (match({TokenType::Comma})) {
    if (match({TokenType::Identifier}) && previous().lexeme == "names") {
      consume(TokenType::Colon, "Expect ':' after 'names'.");
      consume(TokenType::LBrace, "Expect '{' after 'names:'.");
      if (!check(TokenType::RBrace)) {
        do {
          consume(TokenType::Identifier, "Expect name identifier.");
          names.push_back(previous().lexeme);
        } while (match({TokenType::Comma}));
      }
      consume(TokenType::RBrace, "Expect '}' after names list.");
    }
  }

  consume(TokenType::RParen, "Expect ')' after macro role.");

  consume(TokenType::Macro, "Expect 'macro' after '@attached(...)'.");
  consume(TokenType::Identifier, "Expect attached macro name.");
  Token name = previous();

  // Parse parameter list
  consume(TokenType::LParen, "Expect '(' after attached macro name.");
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

  // Parse return type
  Token returnType(TokenType::Void, "Void", peek().line);
  if (match({TokenType::Arrow})) {
    returnType = parseType();
  }

  // Parse macro body
  std::unique_ptr<Expr> body = nullptr;
  if (match({TokenType::Equal})) {
    body = expression();
  }

  // Convert string vector to Token vector
  std::vector<Token> nameTokens;
  for (const auto &nameStr : names) {
    nameTokens.emplace_back(TokenType::Identifier, nameStr, peek().line);
  }

  Token attachmentKind(TokenType::Identifier, role.lexeme, peek().line);
  return std::make_unique<AttachedMacroStmt>(
      name, std::move(parameters), std::move(body), AccessLevel::INTERNAL,
      attachmentKind, std::move(nameTokens));
}

} // namespace miniswift