#ifndef MINISWIFT_GENERIC_NODES_H
#define MINISWIFT_GENERIC_NODES_H

#include "../lexer/Token.h"
#include <vector>
#include <memory>
#include <string>

namespace miniswift {

// Forward declarations
class Expr;
class Stmt;

// Type parameter constraint types
enum class ConstraintType {
    CLASS_CONSTRAINT,    // T: SomeClass
    PROTOCOL_CONSTRAINT, // T: SomeProtocol
    SAME_TYPE_CONSTRAINT // T == SomeType
};

// Type constraint for generic parameters
struct TypeConstraint {
    Token typeParameter;     // The type parameter being constrained (e.g., T)
    ConstraintType constraintType;
    Token constraintTarget;  // The class/protocol/type being constrained to
    
    TypeConstraint(Token typeParam, ConstraintType type, Token target)
        : typeParameter(typeParam), constraintType(type), constraintTarget(target) {}
};

// Generic type parameter (e.g., T, Element, Key, Value)
struct TypeParameter {
    Token name;                              // Parameter name (T, Element, etc.)
    std::vector<TypeConstraint> constraints; // Constraints on this parameter
    
    explicit TypeParameter(Token name, std::vector<TypeConstraint> constraints = {})
        : name(name), constraints(std::move(constraints)) {}
};

// Generic parameter clause: <T, U: Equatable, V: SomeClass>
struct GenericParameterClause {
    std::vector<TypeParameter> parameters;
    
    explicit GenericParameterClause(std::vector<TypeParameter> params)
        : parameters(std::move(params)) {}
        
    bool isEmpty() const { return parameters.empty(); }
};

// Generic type representation (e.g., Array<Int>, Dictionary<String, Int>)
struct GenericType {
    Token baseName;                           // Base type name (Array, Dictionary, etc.)
    std::vector<Token> typeArguments;        // Type arguments (Int, String, etc.)
    
    GenericType(Token base, std::vector<Token> args)
        : baseName(base), typeArguments(std::move(args)) {}
};

// Where clause for additional generic constraints
// Example: where T: Equatable, U.Element == String
struct WhereClause {
    std::vector<TypeConstraint> constraints;
    
    explicit WhereClause(std::vector<TypeConstraint> constraints)
        : constraints(std::move(constraints)) {}
        
    bool isEmpty() const { return constraints.empty(); }
};

// Generic function signature enhancement
struct GenericFunctionSignature {
    Token name;
    GenericParameterClause genericParams;    // <T, U, V>
    std::vector<struct Parameter> parameters; // Function parameters
    Token returnType;                        // Return type (can be generic)
    WhereClause whereClause;                 // where clause constraints
    
    GenericFunctionSignature(Token name, GenericParameterClause genericParams,
                           std::vector<struct Parameter> params, Token returnType,
                           WhereClause whereClause = WhereClause({}))
        : name(name), genericParams(std::move(genericParams)),
          parameters(std::move(params)), returnType(returnType),
          whereClause(std::move(whereClause)) {}
};

// Generic type instantiation expression: Stack<String>()
struct GenericTypeInstantiation {
    Token typeName;                    // Base type name
    std::vector<Token> typeArguments; // Type arguments
    
    GenericTypeInstantiation(Token name, std::vector<Token> args)
        : typeName(name), typeArguments(std::move(args)) {}
};

} // namespace miniswift

#endif // MINISWIFT_GENERIC_NODES_H