#include "SemanticAnalyzer.h"
#include "../parser/AST.h"
#include <iostream>
#include <algorithm>

namespace miniswift {

SemanticAnalyzer::SemanticAnalyzer()
    : symbolTable(std::make_unique<SymbolTable>())
    , typeSystem(std::make_unique<TypeSystem>())
    , currentTypedProgram(nullptr)
    , currentExpressionType(nullptr)
    , currentFunctionReturnType(nullptr)
    , inFunctionContext(false)
    , inLoopContext(false)
    , inClassContext(false) {
    
    // 初始化内置类型
    // initializeBuiltinTypes();
    // initializeBuiltinFunctions();
}

// 移除默认析构函数定义

SemanticAnalysisResult SemanticAnalyzer::analyze(const std::vector<std::unique_ptr<Stmt>>& statements) {
    // 清空之前的结果
    errors.clear();
    warnings.clear();
    currentTypedProgram = std::make_unique<TypedProgram>();
    
    try {
        // 第一遍：收集所有类型和函数声明
        for (const auto& stmt : statements) {
            // collectDeclarations(*stmt);
        }
        
        // 第二遍：进行类型检查和语义分析
        for (const auto& stmt : statements) {
            stmt->accept(*this);
            // if (typedStmt) {
            //     currentTypedProgram->statements.push_back(typedStmt);
            // }
        }
        
        // 第三遍：进行全局检查
        // performGlobalChecks();
        
    } catch (const std::exception& e) {
        reportError("Semantic analysis failed: " + std::string(e.what()));
    }
    
    SemanticAnalysisResult result;
    result.typedAST = std::move(currentTypedProgram);
    result.errors = errors;
    result.warnings = warnings;
    
    return result;
}

void SemanticAnalyzer::visit(const Binary& expr) {
    // 分析左右操作数
    expr.left->accept(*this);
    auto leftType = currentExpressionType;
    
    expr.right->accept(*this);
    auto rightType = currentExpressionType;
    
    // 检查操作符兼容性
    // auto resultType = checkBinaryOperation(expr.op.lexeme, leftType, rightType);
    // if (!resultType) {
    //     reportError("Invalid binary operation: " + expr.op.lexeme + " between " + 
    //                leftType->toString() + " and " + rightType->toString(), 
    //                expr.op.line, expr.op.column);
    //     currentExpressionType = typeSystem->getErrorType();
    // } else {
    //     currentExpressionType = resultType;
    // }
}

void SemanticAnalyzer::visit(const Ternary& expr) {
    // 分析条件表达式
    expr.condition->accept(*this);
    auto conditionType = currentExpressionType;
    
    // if (!typeSystem->isConvertibleTo(conditionType, typeSystem->getBoolType())) {
    //     reportError("Ternary condition must be boolean, got " + conditionType->toString(),
    //                expr.question.line, expr.question.column);
    // }
    
    // 分析then和else表达式
    expr.thenBranch->accept(*this);
    auto thenType = currentExpressionType;
    
    expr.elseBranch->accept(*this);
    auto elseType = currentExpressionType;
    
    // 找到公共类型
    // auto commonType = typeSystem->findCommonType(thenType, elseType);
    // if (!commonType) {
    //     reportError("Incompatible types in ternary expression: " + 
    //                thenType->toString() + " and " + elseType->toString(),
    //                expr.colon.line, expr.colon.column);
    //     currentExpressionType = typeSystem->getErrorType();
    // } else {
    //     currentExpressionType = commonType;
    // }
}

void SemanticAnalyzer::visit(const Grouping& expr) {
    expr.expression->accept(*this);
    // 分组表达式的类型就是内部表达式的类型
}

void SemanticAnalyzer::visit(const Literal& expr) {
    // 根据字面量值确定类型
    // if (std::holds_alternative<int64_t>(expr.value)) {
    //     currentExpressionType = typeSystem->getIntType();
    // } else if (std::holds_alternative<double>(expr.value)) {
    //     currentExpressionType = typeSystem->getFloatType();
    // } else if (std::holds_alternative<bool>(expr.value)) {
    //     currentExpressionType = typeSystem->getBoolType();
    // } else if (std::holds_alternative<std::string>(expr.value)) {
    //     currentExpressionType = typeSystem->getStringType();
    // } else {
    //     reportError("Unknown literal type");
    //     currentExpressionType = typeSystem->getErrorType();
    // }
}

void SemanticAnalyzer::visit(const Unary& expr) {
    expr.right->accept(*this);
    auto operandType = currentExpressionType;
    
    // auto resultType = checkUnaryOperation(expr.op.lexeme, operandType);
    // if (!resultType) {
    //     reportError("Invalid unary operation: " + expr.op.lexeme + " on " + operandType->toString(),
    //                expr.op.line, expr.op.column);
    //     currentExpressionType = typeSystem->getErrorType();
    // } else {
    //     currentExpressionType = resultType;
    // }
}

void SemanticAnalyzer::visit(const VarExpr& expr) {
    // auto symbol = symbolTable->lookup(expr.name.lexeme);
    // if (!symbol) {
    //     reportError("Undefined variable: " + expr.name.lexeme, expr.name.line, expr.name.column);
    //     currentExpressionType = typeSystem->getErrorType();
    //     return;
    // }
    
    // if (symbol->kind != SymbolKind::Variable) {
    //     reportError("Expected variable, got " + symbolKindToString(symbol->kind),
    //                expr.name.line, expr.name.column);
    //     currentExpressionType = typeSystem->getErrorType();
    //     return;
    // }
    
    // auto varSymbol = static_cast<VariableSymbol*>(symbol.get());
    // currentExpressionType = varSymbol->type;
}

void SemanticAnalyzer::visit(const Assign& expr) {
    // 检查左值
    // auto symbol = symbolTable->lookup(expr.name.lexeme);
    // if (!symbol) {
    //     reportError("Undefined variable: " + expr.name.lexeme, expr.name.line, expr.name.column);
    //     currentExpressionType = typeSystem->getErrorType();
    //     return;
    // }
    
    // if (symbol->kind != SymbolKind::Variable) {
    //     reportError("Cannot assign to non-variable", expr.name.line, expr.name.column);
    //     currentExpressionType = typeSystem->getErrorType();
    //     return;
    // }
    
    // auto varSymbol = static_cast<VariableSymbol*>(symbol.get());
    // if (varSymbol->isConst) {
    //     reportError("Cannot assign to constant variable: " + expr.name.lexeme,
    //                expr.name.line, expr.name.column);
    //     currentExpressionType = typeSystem->getErrorType();
    //     return;
    // }
    
    // 分析右值
    expr.value->accept(*this);
    auto valueType = currentExpressionType;
    
    // 检查类型兼容性
    // if (!typeSystem->isConvertibleTo(valueType, varSymbol->type)) {
    //     reportError("Cannot assign " + valueType->toString() + " to " + varSymbol->type->toString(),
    //                expr.equals.line, expr.equals.column);
    //     currentExpressionType = typeSystem->getErrorType();
    //     return;
    // }
    
    // currentExpressionType = varSymbol->type;
}

void SemanticAnalyzer::visit(const Call& expr) {
    // 分析被调用的表达式
    expr.callee->accept(*this);
    
    // 检查是否为函数类型
    auto calleeType = currentExpressionType;
    // if (calleeType->getKind() != TypeKind::Function) {
    //     reportError("Cannot call non-function type: " + calleeType->toString(),
    //                expr.paren.line, expr.paren.column);
    //     currentExpressionType = typeSystem->getErrorType();
    //     return;
    // }
    
    // auto funcType = std::static_pointer_cast<FunctionType>(calleeType);
    
    // 检查参数数量
    // if (expr.arguments.size() != funcType->parameterTypes.size()) {
    //     reportError("Expected " + std::to_string(funcType->parameterTypes.size()) + 
    //                " arguments, got " + std::to_string(expr.arguments.size()),
    //                expr.paren.line, expr.paren.column);
    //     currentExpressionType = typeSystem->getErrorType();
    //     return;
    // }
    
    // 检查参数类型
    for (size_t i = 0; i < expr.arguments.size(); ++i) {
        expr.arguments[i]->accept(*this);
        auto argType = currentExpressionType;
        // auto expectedType = funcType->parameterTypes[i];
        
        // if (!typeSystem->isConvertibleTo(argType, expectedType)) {
        //     reportError("Argument " + std::to_string(i + 1) + ": expected " + 
        //                expectedType->toString() + ", got " + argType->toString());
        // }
    }
    
    // currentExpressionType = funcType->returnType;
}

void SemanticAnalyzer::visit(const ArrayLiteral& expr) {
    // if (expr.elements.empty()) {
    //     // 空数组需要类型推断或显式类型注解
    //     reportWarning("Empty array literal requires type annotation");
    //     currentExpressionType = typeSystem->getArrayType(typeSystem->getAnyType());
    //     return;
    // }
    
    // 分析第一个元素确定数组类型
    // expr.elements[0]->accept(*this);
    // auto elementType = currentExpressionType;
    
    // 检查所有元素类型一致
    for (size_t i = 0; i < expr.elements.size(); ++i) {
        expr.elements[i]->accept(*this);
        auto currentElementType = currentExpressionType;
        
        // if (!typeSystem->areTypesEqual(elementType, currentElementType)) {
        //     // 尝试找到公共类型
        //     auto commonType = typeSystem->findCommonType(elementType, currentElementType);
        //     if (commonType) {
        //         elementType = commonType;
        //     } else {
        //         reportError("Inconsistent array element types: " + 
        //                    elementType->toString() + " and " + currentElementType->toString());
        //         currentExpressionType = typeSystem->getErrorType();
        //         return;
        //     }
        // }
    }
    
    // currentExpressionType = typeSystem->getArrayType(elementType);
}

void SemanticAnalyzer::visit(const DictionaryLiteral& expr) {
    if (expr.pairs.empty()) {
        reportWarning("Empty dictionary literal requires type annotation");
        currentExpressionType = typeSystem->createDictionaryType(
            typeSystem->getAnyType(), typeSystem->getAnyType());
        return;
    }
    
    // 分析第一对键值确定字典类型
    expr.pairs[0].key->accept(*this);
    auto keyType = currentExpressionType;
    
    expr.pairs[0].value->accept(*this);
    auto valueType = currentExpressionType;
    
    // 检查所有键值对类型一致
    // 简化实现，注释掉复杂的类型检查
    /*
    for (size_t i = 1; i < expr.pairs.size(); ++i) {
        expr.pairs[i].first->accept(*this);
        auto currentKeyType = currentExpressionType;
        
        expr.pairs[i].value->accept(*this);
        auto currentValueType = currentExpressionType;
        
        if (!typeSystem->areTypesEqual(keyType, currentKeyType)) {
            auto commonKeyType = typeSystem->findCommonType(keyType, currentKeyType);
            if (commonKeyType) {
                keyType = commonKeyType;
            } else {
                reportError("Inconsistent dictionary key types");
                currentExpressionType = typeSystem->getErrorType();
                return;
            }
        }
        
        if (!typeSystem->areTypesEqual(valueType, currentValueType)) {
            auto commonValueType = typeSystem->findCommonType(valueType, currentValueType);
            if (commonValueType) {
                valueType = commonValueType;
            } else {
                reportError("Inconsistent dictionary value types");
                currentExpressionType = typeSystem->getErrorType();
                return;
            }
        }
    }
    
    currentExpressionType = typeSystem->getDictionaryType(keyType, valueType);
    */
}

void SemanticAnalyzer::visit(const TupleLiteral& expr) {
    // 简化实现
    /*
    std::vector<std::shared_ptr<Type>> elementTypes;
    
    for (const auto& element : expr.elements) {
        element->accept(*this);
        elementTypes.push_back(currentExpressionType);
    }
    
    currentExpressionType = typeSystem->getTupleType(elementTypes);
    */
}

void SemanticAnalyzer::visit(const IndexAccess& expr) {
    // 简化实现
    /*
    // 分析被索引的表达式
    expr.object->accept(*this);
    auto objectType = currentExpressionType;
    
    // 分析索引表达式
    expr.index->accept(*this);
    auto indexType = currentExpressionType;
    
    // 检查对象类型是否支持索引
    if (objectType->getKind() == TypeKind::Array) {
        auto arrayType = std::static_pointer_cast<ArrayType>(objectType);
        if (!typeSystem->isConvertible(indexType, typeSystem->getIntType())) {
            reportError("Array index must be integer, got " + indexType->toString());
            currentExpressionType = typeSystem->getErrorType();
        } else {
            currentExpressionType = arrayType->elementType;
        }
    } else if (objectType->getKind() == TypeKind::Dictionary) {
        auto dictType = std::static_pointer_cast<DictionaryType>(objectType);
        if (!typeSystem->isConvertible(indexType, dictType->keyType)) {
            reportError("Dictionary key type mismatch: expected " + 
                       dictType->keyType->toString() + ", got " + indexType->toString());
            currentExpressionType = typeSystem->getErrorType();
        } else {
            // 字典访问返回可选类型
            currentExpressionType = typeSystem->getOptionalType(dictType->valueType);
        }
    } else {
        reportError("Cannot index into type: " + objectType->toString());
        currentExpressionType = typeSystem->getErrorType();
    }
    */
}

// 语句访问者实现
void SemanticAnalyzer::visit(const ExprStmt& stmt) {
    stmt.expression->accept(*this);
    
    // 创建TypedExpr（简化实现）
    std::unique_ptr<TypedExpr> typedExpr = nullptr;
    if (auto call = dynamic_cast<const Call*>(stmt.expression.get())) {
        // 为Call表达式创建TypedCall
        auto originalCall = std::unique_ptr<Call>(static_cast<Call*>(call->clone().release()));
        auto voidType = std::make_shared<PrimitiveType>(TypeKind::Void, "Void");
        
        // 创建类型化的callee
        std::unique_ptr<TypedExpr> typedCallee = nullptr;
        if (auto varExpr = dynamic_cast<const VarExpr*>(call->callee.get())) {
            auto originalVarExpr = std::unique_ptr<VarExpr>(static_cast<VarExpr*>(varExpr->clone().release()));
            typedCallee = std::make_unique<TypedVarExpr>(std::move(originalVarExpr), voidType);
        }
        
        // 创建类型化的参数
        std::vector<std::unique_ptr<TypedExpr>> typedArgs;
        for (const auto& arg : call->arguments) {
            if (auto literal = dynamic_cast<const Literal*>(arg.get())) {
                auto originalLiteral = std::unique_ptr<Literal>(static_cast<Literal*>(literal->clone().release()));
                // 根据字面量的值确定类型
                std::shared_ptr<Type> argType;
                if (literal->value.type == TokenType::IntegerLiteral || literal->value.type == TokenType::FloatingLiteral) {
                    argType = std::make_shared<PrimitiveType>(TypeKind::Int, "Int");
                } else if (literal->value.type == TokenType::StringLiteral) {
                    argType = std::make_shared<PrimitiveType>(TypeKind::String, "String");
                } else {
                    argType = std::make_shared<PrimitiveType>(TypeKind::Int, "Int"); // 默认
                }
                typedArgs.push_back(std::make_unique<TypedLiteral>(std::move(originalLiteral), argType));
            } else {
                // 对于非字面量参数，先分析表达式类型
                arg->accept(*this);
                auto argType = currentExpressionType ? currentExpressionType : std::make_shared<PrimitiveType>(TypeKind::Int, "Int");
                
                // 创建适当的TypedExpr
                if (auto varExpr = dynamic_cast<const VarExpr*>(arg.get())) {
                    auto originalVarExpr = std::unique_ptr<VarExpr>(static_cast<VarExpr*>(varExpr->clone().release()));
                    typedArgs.push_back(std::make_unique<TypedVarExpr>(std::move(originalVarExpr), argType));
                }
            }
        }
        
        if (typedCallee) {
            typedExpr = std::make_unique<TypedCall>(std::move(originalCall), voidType, std::move(typedCallee), std::move(typedArgs));
        }
    }
    
    // 创建TypedExprStmt并添加到currentTypedProgram
    if (typedExpr && currentTypedProgram) {
        auto originalExprStmt = std::unique_ptr<ExprStmt>(static_cast<ExprStmt*>(stmt.clone().release()));
        auto typedExprStmt = std::make_unique<TypedExprStmt>(std::move(originalExprStmt), std::move(typedExpr));
        currentTypedProgram->addStatement(std::move(typedExprStmt));
    }
}

void SemanticAnalyzer::visit(const VarStmt& stmt) {
    // 创建基本类型（简化实现）
    auto voidType = std::make_shared<PrimitiveType>(TypeKind::Void, "Void");
    auto intType = std::make_shared<PrimitiveType>(TypeKind::Int, "Int");
    auto stringType = std::make_shared<PrimitiveType>(TypeKind::String, "String");
    auto doubleType = std::make_shared<PrimitiveType>(TypeKind::Double, "Double");
    
    std::shared_ptr<Type> varType;
    std::unique_ptr<TypedExpr> typedInitializer = nullptr;
    
    // 分析初始化表达式并推断类型（简化实现）
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
        
        // 简单的类型推断
        if (auto literal = dynamic_cast<const Literal*>(stmt.initializer.get())) {
            if (literal->value.type == TokenType::IntegerLiteral) {
                varType = intType;
            } else if (literal->value.type == TokenType::FloatingLiteral) {
                varType = doubleType;
            } else if (literal->value.type == TokenType::StringLiteral) {
                varType = stringType;
            } else {
                varType = voidType;
            }
        } else {
            varType = intType; // 默认类型
        }
    } else {
        varType = voidType; // 默认类型
    }
    
    // 创建TypedVarStmt
    auto originalStmt = std::unique_ptr<VarStmt>(static_cast<VarStmt*>(stmt.clone().release()));
    auto typedVarStmt = std::make_unique<TypedVarStmt>(
        std::move(originalStmt), varType, std::move(typedInitializer));
    
    // 添加到当前TypedProgram
    if (currentTypedProgram) {
        currentTypedProgram->addStatement(std::move(typedVarStmt));
        currentTypedProgram->addSymbolMapping(stmt.name.lexeme, varType);
    }
}

void SemanticAnalyzer::visit(const BlockStmt& stmt) {
    // 简化实现
    /*
    symbolTable->enterScope();
    
    for (const auto& s : stmt.statements) {
        s->accept(*this);
    }
    
    symbolTable->exitScope();
    */
    for (const auto& s : stmt.statements) {
        s->accept(*this);
    }
}

void SemanticAnalyzer::visit(const IfStmt& stmt) {
    // 简化实现
    stmt.condition->accept(*this);
    /*
    auto conditionType = currentExpressionType;
    
    if (!typeSystem->isConvertible(conditionType, typeSystem->getBoolType())) {
            reportError("If condition must be boolean, got " + conditionType->toString());
        }
    */
    
    // 分析then分支
    stmt.thenBranch->accept(*this);
    
    // 分析else分支（如果存在）
    if (stmt.elseBranch) {
        stmt.elseBranch->accept(*this);
    }
}

void SemanticAnalyzer::visit(const WhileStmt& stmt) {
    // 简化实现
    stmt.condition->accept(*this);
    /*
    auto conditionType = currentExpressionType;
    
    if (!typeSystem->isConvertible(conditionType, typeSystem->getBoolType())) {
            reportError("While condition must be boolean, got " + conditionType->toString());
        }
    
    // 进入循环上下文
    bool prevInLoop = inLoopContext;
    inLoopContext = true;
    */
    
    // 分析循环体
    stmt.body->accept(*this);
    
    /*
    // 恢复循环上下文
    inLoopContext = prevInLoop;
    */
}

void SemanticAnalyzer::visit(const ForStmt& stmt) {
    // 简化实现
    /*
    symbolTable->enterScope();
    */
    
    // 分析初始化语句
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
    }
    
    // 分析条件表达式
    if (stmt.condition) {
        stmt.condition->accept(*this);
        /*
        auto conditionType = currentExpressionType;
        
        if (!typeSystem->isConvertible(conditionType, typeSystem->getBoolType())) {
            reportError("For condition must be boolean, got " + conditionType->toString());
        }
        */
    }
    
    // 分析增量表达式
    if (stmt.increment) {
        stmt.increment->accept(*this);
    }
    
    // 简化实现
    /*
    // 进入循环上下文
    bool prevInLoop = inLoopContext;
    inLoopContext = true;
    */
    
    // 分析循环体
    stmt.body->accept(*this);
    
    /*
    // 恢复循环上下文
    inLoopContext = prevInLoop;
    
    symbolTable->exitScope();
    */
}

void SemanticAnalyzer::visit(const ReturnStmt& stmt) {
    // 简化实现
    /*
    if (!currentFunctionReturnType) {
        reportError("Return statement outside function");
        return;
    }
    
    auto expectedReturnType = currentFunctionReturnType;
    */
    
    if (stmt.value) {
        stmt.value->accept(*this);
        /*
        auto returnType = currentExpressionType;
        
        if (!typeSystem->isConvertible(returnType, expectedReturnType)) {
            reportError("Return type mismatch: expected " + expectedReturnType->toString() + 
                       ", got " + returnType->toString());
        }
        */
    } else {
        /*
        if (expectedReturnType->getKind() != TypeKind::Void) {
            reportError("Missing return value for non-void function");
        }
        */
    }
}

void SemanticAnalyzer::visit(const FunctionStmt& stmt) {
    std::cerr << "DEBUG: Visiting FunctionStmt: " << stmt.name.lexeme << std::endl;
    
    // 创建基本类型（简化实现）
    auto voidType = std::make_shared<PrimitiveType>(TypeKind::Void, "Void");
    auto intType = std::make_shared<PrimitiveType>(TypeKind::Int, "Int");
    auto stringType = std::make_shared<PrimitiveType>(TypeKind::String, "String");
    
    // 解析参数类型（简化实现）
    std::vector<std::shared_ptr<Type>> paramTypes;
    for (const auto& param : stmt.parameters) {
        if (param.type.lexeme == "Int") {
            paramTypes.push_back(intType);
        } else if (param.type.lexeme == "String") {
            paramTypes.push_back(stringType);
        } else {
            paramTypes.push_back(voidType); // 默认类型
        }
    }
    
    // 解析返回类型（简化实现）
    std::shared_ptr<Type> returnType;
    if (stmt.returnType.lexeme == "Int") {
        returnType = intType;
    } else if (stmt.returnType.lexeme == "String") {
        returnType = stringType;
    } else {
        returnType = voidType; // 默认返回类型
    }
    
    // 创建函数类型
    auto funcType = std::make_shared<FunctionType>(paramTypes, returnType);
    
    // 分析函数体
    std::unique_ptr<TypedStmt> typedBody = nullptr;
    if (stmt.body) {
        // 创建一个简单的TypedBlockStmt来包含函数体
        auto blockStmt = dynamic_cast<const BlockStmt*>(stmt.body.get());
        if (blockStmt) {
            std::vector<std::unique_ptr<TypedStmt>> typedStatements;
            
            std::cerr << "DEBUG: BlockStmt has " << blockStmt->statements.size() << " statements" << std::endl;
            
            // 分析块中的每个语句
            for (const auto& s : blockStmt->statements) {
                std::cerr << "DEBUG: Processing statement in function body" << std::endl;
                std::cerr << "DEBUG: Statement type: " << typeid(*s).name() << std::endl;
                // 为每个语句创建相应的TypedStmt
                if (auto exprStmt = dynamic_cast<const ExprStmt*>(s.get())) {
                    std::cerr << "DEBUG: Found ExprStmt" << std::endl;
                    // 分析表达式
                    exprStmt->expression->accept(*this);
                    
                    std::cerr << "DEBUG: Expression type: " << typeid(*exprStmt->expression).name() << std::endl;
                    
                    // 创建TypedExpr（简化实现）
                    std::unique_ptr<TypedExpr> typedExpr = nullptr;
                    if (auto call = dynamic_cast<const Call*>(exprStmt->expression.get())) {
                        // 为Call表达式创建TypedCall
                        auto originalCall = std::unique_ptr<Call>(static_cast<Call*>(call->clone().release()));
                        auto voidType = std::make_shared<PrimitiveType>(TypeKind::Void, "Void");
                        
                        // 创建类型化的callee
                        std::unique_ptr<TypedExpr> typedCallee = nullptr;
                        if (auto varExpr = dynamic_cast<const VarExpr*>(call->callee.get())) {
                            auto originalVarExpr = std::unique_ptr<VarExpr>(static_cast<VarExpr*>(varExpr->clone().release()));
                            typedCallee = std::make_unique<TypedVarExpr>(std::move(originalVarExpr), voidType);
                        }
                        
                        // 创建类型化的参数
                        std::vector<std::unique_ptr<TypedExpr>> typedArgs;
                        for (const auto& arg : call->arguments) {
                            if (auto literal = dynamic_cast<const Literal*>(arg.get())) {
                                auto originalLiteral = std::unique_ptr<Literal>(static_cast<Literal*>(literal->clone().release()));
                                // 根据字面量的值确定类型
                                std::shared_ptr<Type> argType;
                                if (literal->value.type == TokenType::IntegerLiteral || literal->value.type == TokenType::FloatingLiteral) {
                                    argType = std::make_shared<PrimitiveType>(TypeKind::Int, "Int");
                                } else if (literal->value.type == TokenType::StringLiteral) {
                                    argType = std::make_shared<PrimitiveType>(TypeKind::String, "String");
                                } else {
                                    argType = std::make_shared<PrimitiveType>(TypeKind::Int, "Int"); // 默认
                                }
                                typedArgs.push_back(std::make_unique<TypedLiteral>(std::move(originalLiteral), argType));
                            } else {
                                // 对于非字面量参数，先分析表达式类型
                                arg->accept(*this);
                                auto argType = currentExpressionType ? currentExpressionType : std::make_shared<PrimitiveType>(TypeKind::Int, "Int");
                                
                                // 创建适当的TypedExpr
                                if (auto varExpr = dynamic_cast<const VarExpr*>(arg.get())) {
                                    auto originalVarExpr = std::unique_ptr<VarExpr>(static_cast<VarExpr*>(varExpr->clone().release()));
                                    typedArgs.push_back(std::make_unique<TypedVarExpr>(std::move(originalVarExpr), argType));
                                } else {
                                    // 对于其他类型的表达式，暂时跳过或创建通用的TypedExpr
                                    // 这里可以根据需要扩展
                                }
                            }
                        }
                        
                        if (typedCallee) {
                            typedExpr = std::make_unique<TypedCall>(std::move(originalCall), voidType, std::move(typedCallee), std::move(typedArgs));
                        }
                    }
                    
                    auto originalExprStmt = std::unique_ptr<ExprStmt>(static_cast<ExprStmt*>(exprStmt->clone().release()));
                    auto typedExprStmt = std::make_unique<TypedExprStmt>(std::move(originalExprStmt), std::move(typedExpr));
                    std::cerr << "DEBUG: Created TypedExprStmt with TypedCall for function" << std::endl;
                    typedStatements.push_back(std::move(typedExprStmt));
                    std::cerr << "DEBUG: Added TypedExprStmt to typedStatements, size now: " << typedStatements.size() << std::endl;
                } else if (auto printStmt = dynamic_cast<const PrintStmt*>(s.get())) {
                    std::cerr << "DEBUG: Found PrintStmt" << std::endl;
                    // 将PrintStmt转换为print函数调用
                    if (!printStmt->expressions.empty()) {
                        // 创建print函数调用
                         auto printVarExpr = std::make_unique<VarExpr>(Token{TokenType::Identifier, "print", 0});
                        
                        // 创建参数列表
                        std::vector<std::unique_ptr<Expr>> args;
                        for (const auto& expr : printStmt->expressions) {
                            args.push_back(expr->clone());
                        }
                        
                        // 创建Call表达式
                        auto callExpr = std::make_unique<Call>(std::move(printVarExpr), std::move(args));
                        
                        // 创建TypedCall
                        auto voidType = std::make_shared<PrimitiveType>(TypeKind::Void, "Void");
                        
                        // 创建类型化的callee
                         auto typedCallee = std::make_unique<TypedVarExpr>(
                             std::make_unique<VarExpr>(Token{TokenType::Identifier, "print", 0}), voidType);
                        
                        // 创建类型化的参数
                        std::vector<std::unique_ptr<TypedExpr>> typedArgs;
                        for (const auto& expr : printStmt->expressions) {
                            if (auto literal = dynamic_cast<const Literal*>(expr.get())) {
                                auto originalLiteral = std::unique_ptr<Literal>(static_cast<Literal*>(literal->clone().release()));
                                auto intType = std::make_shared<PrimitiveType>(TypeKind::Int, "Int");
                                typedArgs.push_back(std::make_unique<TypedLiteral>(std::move(originalLiteral), intType));
                            }
                        }
                        
                        // 创建TypedCall
                        auto typedCall = std::make_unique<TypedCall>(std::move(callExpr), voidType, std::move(typedCallee), std::move(typedArgs));
                        
                        // 创建ExprStmt包装Call
                         auto printCallExpr = std::make_unique<Call>(
                             std::make_unique<VarExpr>(Token{TokenType::Identifier, "print", 0}),
                            [&]() {
                                std::vector<std::unique_ptr<Expr>> callArgs;
                                for (const auto& expr : printStmt->expressions) {
                                    callArgs.push_back(expr->clone());
                                }
                                return callArgs;
                            }()
                        );
                        auto exprStmt = std::make_unique<ExprStmt>(std::move(printCallExpr));
                        auto typedExprStmt = std::make_unique<TypedExprStmt>(std::move(exprStmt), std::move(typedCall));
                        typedStatements.push_back(std::move(typedExprStmt));
                        std::cerr << "DEBUG: Added TypedExprStmt with TypedCall from PrintStmt, typedStatements size: " << typedStatements.size() << std::endl;
                    }
                } else {
                    // 对于其他类型的语句，先简单处理
                    s->accept(*this);
                }
            }
            
            // 创建TypedBlockStmt
            std::cerr << "DEBUG: Creating TypedBlockStmt with " << typedStatements.size() << " statements" << std::endl;
            auto originalBlockStmt = std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(blockStmt->clone().release()));
            typedBody = std::make_unique<TypedBlockStmt>(std::move(originalBlockStmt), std::move(typedStatements));
            std::cerr << "DEBUG: TypedBlockStmt created successfully" << std::endl;
        }
    }
    
    // 创建TypedFunctionStmt
    auto originalStmt = std::unique_ptr<FunctionStmt>(static_cast<FunctionStmt*>(stmt.clone().release()));
    auto typedFuncStmt = std::make_unique<TypedFunctionStmt>(
        std::move(originalStmt), funcType, paramTypes, std::move(typedBody));
    
    // 添加到当前TypedProgram
    if (currentTypedProgram) {
        currentTypedProgram->addStatement(std::move(typedFuncStmt));
        currentTypedProgram->addFunctionSignature(stmt.name.lexeme, funcType);
    }
}

void SemanticAnalyzer::visit(const StructStmt& stmt) {
    // 简化实现
    /*
    // 检查结构体名是否已存在
    if (symbolTable->lookupInCurrentScope(stmt.name)) {
        reportError("Struct already declared: " + stmt.name);
        return;
    }
    
    // 创建结构体类型
    auto structType = std::make_shared<UserDefinedType>();
    structType->name = stmt.name;
    structType->kind = TypeKind::Struct;
    
    // 分析字段
    for (const auto& field : stmt.fields) {
        auto fieldType = typeSystem->getTypeByName(field.typeName);
        if (!fieldType) {
            reportError("Unknown field type: " + field.typeName);
            continue;
        }
        
        structType->fields[field.name] = fieldType;
    }
    
    // 注册类型
    typeSystem->registerUserDefinedType(structType);
    
    // 创建类型符号
    auto typeSymbol = std::make_shared<TypeSymbol>();
    typeSymbol->name = stmt.name;
    typeSymbol->type = structType;
    typeSymbol->accessLevel = AccessLevel::Internal;
    
    symbolTable->define(stmt.name, typeSymbol);
    */
}

void SemanticAnalyzer::visit(const ClassStmt& stmt) {
    // 简化实现
    /*
    // 检查类名是否已存在
    if (symbolTable->lookupInCurrentScope(stmt.name)) {
        reportError("Class already declared: " + stmt.name);
        return;
    }
    
    // 创建类类型
    auto classType = std::make_shared<UserDefinedType>();
    classType->name = stmt.name;
    classType->kind = TypeKind::Class;
    
    // 处理继承
    if (!stmt.superclass.empty()) {
        auto superType = typeSystem->getTypeByName(stmt.superclass);
        if (!superType) {
            reportError("Unknown superclass: " + stmt.superclass);
        } else if (superType->getKind() != TypeKind::Class) {
            reportError("Superclass must be a class type");
        } else {
            classType->superType = superType;
        }
    }
    
    // 注册类型（需要先注册以支持递归引用）
    typeSystem->registerUserDefinedType(classType);
    
    // 进入类作用域
    symbolTable->enterScope();
    
    // 分析类成员
    for (const auto& member : stmt.members) {
        member->accept(*this);
    }
    
    symbolTable->exitScope();
    
    // 创建类型符号
    auto typeSymbol = std::make_shared<TypeSymbol>();
    typeSymbol->name = stmt.name;
    typeSymbol->type = classType;
    typeSymbol->accessLevel = AccessLevel::Internal;
    
    symbolTable->define(stmt.name, typeSymbol);
    */
}

// 私有辅助方法实现（移除不存在的方法）

// 移除重复的方法定义

// 移除不存在的方法实现

void SemanticAnalyzer::reportError(const std::string& message, int line, int column) {
    errors.emplace_back(message, line, column);
}

void SemanticAnalyzer::reportWarning(const std::string& message) {
    warnings.emplace_back(message);
}

// 移除重复的方法实现

// 移除重复的visit方法定义

// 辅助方法实现
std::shared_ptr<Type> SemanticAnalyzer::analyzeExpression(const Expr& expr) {
    expr.accept(*this);
    return currentExpressionType;
}

void SemanticAnalyzer::analyzeStatement(const Stmt& stmt) {
    stmt.accept(*this);
}

bool SemanticAnalyzer::isTypeCompatible(const std::shared_ptr<Type>& from, const std::shared_ptr<Type>& to) {
    return true; // 简化实现
}

std::shared_ptr<Type> SemanticAnalyzer::getCommonType(const std::shared_ptr<Type>& type1, const std::shared_ptr<Type>& type2) {
    return type1; // 简化实现
}

void SemanticAnalyzer::enterScope() {}
void SemanticAnalyzer::exitScope() {}
void SemanticAnalyzer::declareVariable(const std::string& name, const std::shared_ptr<Type>& type, bool isConst) {}
void SemanticAnalyzer::declareFunction(const std::string& name, const std::shared_ptr<FunctionType>& type) {}
void SemanticAnalyzer::declareType(const std::string& name, const std::shared_ptr<Type>& type) {}

std::shared_ptr<Type> SemanticAnalyzer::inferType(const Expr& expr) {
    return nullptr;
}

std::shared_ptr<Type> SemanticAnalyzer::inferLiteralType(const Literal& literal) {
    return nullptr;
}

std::shared_ptr<Type> SemanticAnalyzer::inferBinaryType(const Binary& binary) {
    return nullptr;
}

std::shared_ptr<Type> SemanticAnalyzer::inferCallType(const Call& call) {
    return nullptr;
}

void SemanticAnalyzer::analyzeControlFlow(const Stmt& stmt) {}
bool SemanticAnalyzer::isReachable(const Stmt& stmt) { return true; }
void SemanticAnalyzer::checkMemorySafety(const Expr& expr) {}
void SemanticAnalyzer::checkNullPointerAccess(const Expr& expr) {}
void SemanticAnalyzer::checkArrayBounds(const IndexAccess& access) {}
void SemanticAnalyzer::checkConcurrencySafety(const Stmt& stmt) {}
void SemanticAnalyzer::checkActorIsolation(const Expr& expr) {}

std::shared_ptr<Type> SemanticAnalyzer::instantiateGenericType(const std::shared_ptr<Type>& genericType, 
                                            const std::unordered_map<std::string, std::shared_ptr<Type>>& substitutions) {
    return genericType;
}

// 添加缺失的visit方法实现
void SemanticAnalyzer::visit(const PrintStmt& stmt) {
    for (const auto& expr : stmt.expressions) {
        expr->accept(*this);
    }
}

void SemanticAnalyzer::visit(const IfLetStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const ForInStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const EnumStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const InitStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const DeinitStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const SubscriptStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const ProtocolStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const ExtensionStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const ThrowStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const DoCatchStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const DeferStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const GuardStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const GuardLetStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const SwitchStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const CustomOperatorStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const OperatorPrecedenceStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const ResultBuilderStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const ActorStmt& stmt) {
    // 简化实现
}

void SemanticAnalyzer::visit(const MacroStmt& stmt) {
    (void)stmt; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const ExternalMacroStmt& stmt) {
    (void)stmt; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const FreestandingMacroStmt& stmt) {
    (void)stmt; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const AttachedMacroStmt& stmt) {
    (void)stmt; // 标记参数为有意未使用
    // 简化实现
}

// 添加缺失的表达式visit方法（只添加真正缺失的）
void SemanticAnalyzer::visit(const SubscriptAccess& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const LabeledCall& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const Closure& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const EnumAccess& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const MemberAccess& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const StructInit& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const Super& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const StringInterpolation& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const OptionalChaining& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const Range& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const GenericTypeInstantiationExpr& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const TypeCheck& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const TypeCast& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const TryExpr& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const ResultTypeExpr& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const ErrorLiteral& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const CustomOperatorExpr& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const BitwiseExpr& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const OverflowExpr& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const ResultBuilderExpr& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const AwaitExpr& expr) {
    (void)expr; // 标记参数为有意未使用
    // 简化实现
}

void SemanticAnalyzer::visit(const TaskExpr& expr) {
    // 简化实现
}

void SemanticAnalyzer::visit(const OpaqueTypeExpr& expr) {
    // 简化实现
}

void SemanticAnalyzer::visit(const BoxedProtocolTypeExpr& expr) {
    // 简化实现
}

void SemanticAnalyzer::visit(const MacroExpansionExpr& expr) {
    // 简化实现
}

void SemanticAnalyzer::visit(const FreestandingMacroExpr& expr) {
    // 简化实现
}

void SemanticAnalyzer::visit(const AttachedMacroExpr& expr) {
    // 简化实现
}

} // namespace miniswift