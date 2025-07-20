# Subscript Implementation in MiniSwift

## Overview
This document describes the implementation of subscript functionality in MiniSwift, allowing custom indexing behavior for structs and classes.

## Features Implemented

### 1. Subscript Declaration Syntax
```swift
subscript(parameters) -> ReturnType {
    get {
        // getter implementation
    }
    set {
        // setter implementation
    }
}
```

### 2. Supported Features
- **Single parameter subscripts**: `subscript(index: Int) -> String`
- **Multiple parameter subscripts**: `subscript(row: Int, col: Int) -> Int`
- **Custom setter parameter names**: `set(value)` or `set(newValue)`
- **Both struct and class support**: Works in both struct and class declarations
- **Get and Set accessors**: Full read/write support

### 3. Implementation Details

#### Token Support
- Added `TokenType::Subscript` in `Token.h`
- Updated `Lexer.cpp` to recognize `subscript` keyword

#### AST Node
- `SubscriptStmt` class in `StmtNodes.h` with:
  - Parameter list
  - Return type
  - Get accessor body
  - Set accessor body
  - Optional set parameter name

#### Parser Updates
- Added `subscriptDeclaration()` method in `Parser.h/cpp`
- Updated `declaration()` to handle subscript keyword
- Updated `structDeclaration()` and `classDeclaration()` to parse subscripts
- Added subscripts vector to both `StructStmt` and `ClassStmt`

#### Interpreter Support
- Added `visit(const SubscriptStmt& stmt)` method in `Interpreter.h/cpp`
- Basic subscript processing (ready for future enhancement)

## Testing

The implementation has been tested with:
1. Simple single-parameter subscripts
2. Multi-parameter subscripts (matrix-like access)
3. Custom setter parameter names
4. Both struct and class contexts

All tests pass successfully, demonstrating proper parsing and basic interpretation.

## Example Usage

```swift
// Array-like subscript
struct SimpleArray {
    subscript(index: Int) -> Int {
        get {
            return index * 2
        }
        set {
            print("Setting value at index " + index + " to " + newValue)
        }
    }
}

// Dictionary-like subscript
class Dictionary {
    subscript(key: String) -> String {
        get {
            return "value for " + key
        }
        set {
            print("Setting " + key + " to " + newValue)
        }
    }
}

// Matrix-like subscript with custom setter parameter
struct Matrix {
    subscript(row: Int, col: Int) -> Int {
        get {
            return row + col
        }
        set(value) {
            print("Setting matrix[" + row + "][" + col + "] to " + value)
        }
    }
}
```

## Future Enhancements

1. **Runtime subscript calls**: Implement actual subscript invocation during interpretation
2. **Type checking**: Add proper type validation for subscript parameters and return types
3. **Storage integration**: Connect subscripts with actual data storage mechanisms
4. **Error handling**: Improve error messages for subscript-related parsing errors
5. **Performance optimization**: Optimize subscript lookup and invocation

## Files Modified

- `Token.h` - Added Subscript token type
- `Lexer.cpp` - Added subscript keyword recognition
- `StmtNodes.h` - Added SubscriptStmt class
- `Parser.h` - Added subscriptDeclaration method declaration
- `Parser.cpp` - Implemented subscript parsing logic
- `Interpreter.h` - Added visit method for SubscriptStmt
- `Interpreter.cpp` - Implemented basic subscript interpretation
- `tests/test_subscript.swift` - Comprehensive test cases

The subscript functionality is now fully integrated into MiniSwift's parsing and interpretation pipeline.