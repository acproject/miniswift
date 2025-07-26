#ifndef MINISWIFT_TYPED_AST_H
#define MINISWIFT_TYPED_AST_H

#include "../parser/AST.h"
#include "TypeSystem.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>

namespace miniswift {

// 前向声明
class TypedExpr;
class TypedStmt;
class TypedProgram;

// 类型化表达式基类
class TypedExpr {
public:
    TypedExpr(std::unique_ptr<Expr> originalExpr, std::shared_ptr<Type> type)
        : originalExpr_(std::move(originalExpr)), type_(type) {}
    
    virtual ~TypedExpr() = default;
    
    const Expr& getOriginalExpr() const { return *originalExpr_; }
    std::shared_ptr<Type> getType() const { return type_; }
    void setType(std::shared_ptr<Type> type) { type_ = type; }
    
    // 访问者模式支持
    virtual void accept(class TypedExprVisitor& visitor) const = 0;
    virtual std::unique_ptr<TypedExpr> clone() const = 0;
    
protected:
    std::unique_ptr<Expr> originalExpr_;
    std::shared_ptr<Type> type_;
};

// 类型化语句基类
class TypedStmt {
public:
    TypedStmt(std::unique_ptr<Stmt> originalStmt)
        : originalStmt_(std::move(originalStmt)) {}
    
    virtual ~TypedStmt() = default;
    
    const Stmt& getOriginalStmt() const { return *originalStmt_; }
    
    // 访问者模式支持
    virtual void accept(class TypedStmtVisitor& visitor) const = 0;
    virtual std::unique_ptr<TypedStmt> clone() const = 0;
    
protected:
    std::unique_ptr<Stmt> originalStmt_;
};

// 类型化表达式访问者接口
class TypedExprVisitor {
public:
    virtual ~TypedExprVisitor() = default;
    
    // 基础表达式
    virtual void visit(const class TypedBinary& expr) = 0;
    virtual void visit(const class TypedTernary& expr) = 0;
    virtual void visit(const class TypedGrouping& expr) = 0;
    virtual void visit(const class TypedLiteral& expr) = 0;
    virtual void visit(const class TypedUnary& expr) = 0;
    virtual void visit(const class TypedVarExpr& expr) = 0;
    virtual void visit(const class TypedAssign& expr) = 0;
    
    // 集合表达式
    virtual void visit(const class TypedArrayLiteral& expr) = 0;
    virtual void visit(const class TypedDictionaryLiteral& expr) = 0;
    virtual void visit(const class TypedTupleLiteral& expr) = 0;
    virtual void visit(const class TypedIndexAccess& expr) = 0;
    
    // 函数调用
    virtual void visit(const class TypedCall& expr) = 0;
    virtual void visit(const class TypedLabeledCall& expr) = 0;
    virtual void visit(const class TypedClosure& expr) = 0;
    
    // 成员访问
    virtual void visit(const class TypedMemberAccess& expr) = 0;
    virtual void visit(const class TypedStructInit& expr) = 0;
    
    // 类型操作
    virtual void visit(const class TypedTypeCheck& expr) = 0;
    virtual void visit(const class TypedTypeCast& expr) = 0;
    
    // 其他表达式...
};

// 类型化语句访问者接口
class TypedStmtVisitor {
public:
    virtual ~TypedStmtVisitor() = default;
    
    // 基础语句
    virtual void visit(const class TypedExprStmt& stmt) = 0;
    virtual void visit(const class TypedVarStmt& stmt) = 0;
    virtual void visit(const class TypedBlockStmt& stmt) = 0;
    
    // 控制流语句
    virtual void visit(const class TypedIfStmt& stmt) = 0;
    virtual void visit(const class TypedWhileStmt& stmt) = 0;
    virtual void visit(const class TypedForStmt& stmt) = 0;
    virtual void visit(const class TypedReturnStmt& stmt) = 0;
    virtual void visit(const class TypedContinueStmt& stmt) = 0;
    virtual void visit(const class TypedBreakStmt& stmt) = 0;
    virtual void visit(const class TypedFallthroughStmt& stmt) = 0;
    
    // 函数和类型定义
    virtual void visit(const class TypedFunctionStmt& stmt) = 0;
    virtual void visit(const class TypedStructStmt& stmt) = 0;
    virtual void visit(const class TypedClassStmt& stmt) = 0;
    
    // 其他语句...
};

// 具体的类型化表达式类
class TypedBinary : public TypedExpr {
public:
    TypedBinary(std::unique_ptr<Binary> originalExpr, std::shared_ptr<Type> type,
                std::unique_ptr<TypedExpr> left, std::unique_ptr<TypedExpr> right)
        : TypedExpr(std::move(originalExpr), type),
          left_(std::move(left)), right_(std::move(right)) {}
    
    const TypedExpr& getLeft() const { return *left_; }
    const TypedExpr& getRight() const { return *right_; }
    
    void accept(TypedExprVisitor& visitor) const override {
        visitor.visit(*this);
    }
    
    std::unique_ptr<TypedExpr> clone() const override {
        return std::make_unique<TypedBinary>(
            std::unique_ptr<Binary>(static_cast<Binary*>(originalExpr_->clone().release())),
            type_, left_->clone(), right_->clone());
    }
    
private:
    std::unique_ptr<TypedExpr> left_;
    std::unique_ptr<TypedExpr> right_;
};

class TypedLiteral : public TypedExpr {
public:
    TypedLiteral(std::unique_ptr<Literal> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    
    void accept(TypedExprVisitor& visitor) const override {
        visitor.visit(*this);
    }
    
    std::unique_ptr<TypedExpr> clone() const override {
        return std::make_unique<TypedLiteral>(
            std::unique_ptr<Literal>(static_cast<Literal*>(originalExpr_->clone().release())),
            type_);
    }
};

class TypedVarExpr : public TypedExpr {
public:
    TypedVarExpr(std::unique_ptr<VarExpr> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    
    void accept(TypedExprVisitor& visitor) const override {
        visitor.visit(*this);
    }
    
    std::unique_ptr<TypedExpr> clone() const override {
        return std::make_unique<TypedVarExpr>(
            std::unique_ptr<VarExpr>(static_cast<VarExpr*>(originalExpr_->clone().release())),
            type_);
    }
};

class TypedCall : public TypedExpr {
public:
    TypedCall(std::unique_ptr<Call> originalExpr, std::shared_ptr<Type> type,
              std::unique_ptr<TypedExpr> callee, std::vector<std::unique_ptr<TypedExpr>> arguments)
        : TypedExpr(std::move(originalExpr), type),
          callee_(std::move(callee)), arguments_(std::move(arguments)) {}
    
    const TypedExpr& getCallee() const { return *callee_; }
    const std::vector<std::unique_ptr<TypedExpr>>& getArguments() const { return arguments_; }
    
    void accept(TypedExprVisitor& visitor) const override {
        visitor.visit(*this);
    }
    
    std::unique_ptr<TypedExpr> clone() const override {
        std::vector<std::unique_ptr<TypedExpr>> clonedArgs;
        for (const auto& arg : arguments_) {
            clonedArgs.push_back(arg->clone());
        }
        return std::make_unique<TypedCall>(
            std::unique_ptr<Call>(static_cast<Call*>(originalExpr_->clone().release())),
            type_, callee_->clone(), std::move(clonedArgs));
    }
    
private:
    std::unique_ptr<TypedExpr> callee_;
    std::vector<std::unique_ptr<TypedExpr>> arguments_;
};

// 具体的类型化语句类
class TypedVarStmt : public TypedStmt {
public:
    TypedVarStmt(std::unique_ptr<VarStmt> originalStmt, std::shared_ptr<Type> declaredType,
                 std::unique_ptr<TypedExpr> initializer = nullptr)
        : TypedStmt(std::move(originalStmt)), declaredType_(declaredType),
          initializer_(std::move(initializer)) {}
    
    std::shared_ptr<Type> getDeclaredType() const { return declaredType_; }
    const TypedExpr* getInitializer() const { return initializer_.get(); }
    
    void accept(TypedStmtVisitor& visitor) const override {
        visitor.visit(*this);
    }
    
    std::unique_ptr<TypedStmt> clone() const override {
        return std::make_unique<TypedVarStmt>(
            std::unique_ptr<VarStmt>(static_cast<VarStmt*>(originalStmt_->clone().release())),
            declaredType_, initializer_ ? initializer_->clone() : nullptr);
    }
    
private:
    std::shared_ptr<Type> declaredType_;
    std::unique_ptr<TypedExpr> initializer_;
};

class TypedFunctionStmt : public TypedStmt {
public:
    TypedFunctionStmt(std::unique_ptr<FunctionStmt> originalStmt, std::shared_ptr<FunctionType> functionType,
                      std::vector<std::shared_ptr<Type>> parameterTypes, std::unique_ptr<TypedStmt> body)
        : TypedStmt(std::move(originalStmt)), functionType_(functionType),
          parameterTypes_(std::move(parameterTypes)), body_(std::move(body)) {}
    
    std::shared_ptr<FunctionType> getFunctionType() const { return functionType_; }
    const std::vector<std::shared_ptr<Type>>& getParameterTypes() const { return parameterTypes_; }
    const TypedStmt& getBody() const { return *body_; }
    
    void accept(TypedStmtVisitor& visitor) const override {
        visitor.visit(*this);
    }
    
    std::unique_ptr<TypedStmt> clone() const override {
        return std::make_unique<TypedFunctionStmt>(
            std::unique_ptr<FunctionStmt>(static_cast<FunctionStmt*>(originalStmt_->clone().release())),
            functionType_, parameterTypes_, body_->clone());
    }
    
private:
    std::shared_ptr<FunctionType> functionType_;
    std::vector<std::shared_ptr<Type>> parameterTypes_;
    std::unique_ptr<TypedStmt> body_;
};

class TypedBlockStmt : public TypedStmt {
public:
    TypedBlockStmt(std::unique_ptr<BlockStmt> originalStmt, std::vector<std::unique_ptr<TypedStmt>> statements)
        : TypedStmt(std::move(originalStmt)), statements_(std::move(statements)) {}
    
    const std::vector<std::unique_ptr<TypedStmt>>& getStatements() const { return statements_; }
    
    void accept(TypedStmtVisitor& visitor) const override {
        visitor.visit(*this);
    }
    
    std::unique_ptr<TypedStmt> clone() const override {
        std::vector<std::unique_ptr<TypedStmt>> clonedStmts;
        for (const auto& stmt : statements_) {
            clonedStmts.push_back(stmt->clone());
        }
        return std::make_unique<TypedBlockStmt>(
            std::unique_ptr<BlockStmt>(static_cast<BlockStmt*>(originalStmt_->clone().release())),
            std::move(clonedStmts));
    }
    
private:
    std::vector<std::unique_ptr<TypedStmt>> statements_;
};

// 类型化程序（整个AST的根）
class TypedProgram {
public:
    TypedProgram() = default;
    
    void addStatement(std::unique_ptr<TypedStmt> statement) {
        statements_.push_back(std::move(statement));
    }
    
    const std::vector<std::unique_ptr<TypedStmt>>& getStatements() const {
        return statements_;
    }
    
    // 类型信息查询
    void addTypeMapping(const Expr* expr, std::shared_ptr<Type> type) {
        typeMap_[expr] = type;
    }
    
    std::shared_ptr<Type> getTypeForExpression(const Expr* expr) const {
        auto it = typeMap_.find(expr);
        return it != typeMap_.end() ? it->second : nullptr;
    }
    
    // 符号信息
    void addSymbolMapping(const std::string& name, std::shared_ptr<Type> type) {
        symbolTypes_[name] = type;
    }
    
    std::shared_ptr<Type> getSymbolType(const std::string& name) const {
        auto it = symbolTypes_.find(name);
        return it != symbolTypes_.end() ? it->second : nullptr;
    }
    
    // 函数签名信息
    void addFunctionSignature(const std::string& name, std::shared_ptr<FunctionType> type) {
        functionSignatures_[name] = type;
    }
    
    std::shared_ptr<FunctionType> getFunctionSignature(const std::string& name) const {
        auto it = functionSignatures_.find(name);
        return it != functionSignatures_.end() ? it->second : nullptr;
    }
    
    // 类型定义信息
    void addTypeDefinition(const std::string& name, std::shared_ptr<Type> type) {
        typeDefinitions_[name] = type;
    }
    
    std::shared_ptr<Type> getTypeDefinition(const std::string& name) const {
        auto it = typeDefinitions_.find(name);
        return it != typeDefinitions_.end() ? it->second : nullptr;
    }
    
private:
    std::vector<std::unique_ptr<TypedStmt>> statements_;
    
    // 类型映射表
    std::unordered_map<const Expr*, std::shared_ptr<Type>> typeMap_;
    std::unordered_map<std::string, std::shared_ptr<Type>> symbolTypes_;
    std::unordered_map<std::string, std::shared_ptr<FunctionType>> functionSignatures_;
    std::unordered_map<std::string, std::shared_ptr<Type>> typeDefinitions_;
};

// 类型化AST构建器
class TypedASTBuilder {
public:
    TypedASTBuilder(std::shared_ptr<TypeSystem> typeSystem)
        : typeSystem_(typeSystem) {}
    
    // 表达式构建
    std::unique_ptr<TypedExpr> buildTypedExpression(std::unique_ptr<Expr> expr, std::shared_ptr<Type> type);
    std::unique_ptr<TypedBinary> buildTypedBinary(std::unique_ptr<Binary> expr, std::shared_ptr<Type> type,
                                                  std::unique_ptr<TypedExpr> left, std::unique_ptr<TypedExpr> right);
    std::unique_ptr<TypedCall> buildTypedCall(std::unique_ptr<Call> expr, std::shared_ptr<Type> type,
                                              std::unique_ptr<TypedExpr> callee, std::vector<std::unique_ptr<TypedExpr>> arguments);
    
    // 语句构建
    std::unique_ptr<TypedStmt> buildTypedStatement(std::unique_ptr<Stmt> stmt);
    std::unique_ptr<TypedVarStmt> buildTypedVarStmt(std::unique_ptr<VarStmt> stmt, std::shared_ptr<Type> declaredType,
                                                    std::unique_ptr<TypedExpr> initializer = nullptr);
    std::unique_ptr<TypedFunctionStmt> buildTypedFunctionStmt(std::unique_ptr<FunctionStmt> stmt, std::shared_ptr<FunctionType> functionType,
                                                              std::vector<std::shared_ptr<Type>> parameterTypes, std::unique_ptr<TypedStmt> body);
    
    // 程序构建
    std::unique_ptr<TypedProgram> buildTypedProgram(const std::vector<std::unique_ptr<Stmt>>& statements);
    
private:
    std::shared_ptr<TypeSystem> typeSystem_;
};

// 占位符实现（需要根据具体需求补充）
class TypedTernary : public TypedExpr {
public:
    TypedTernary(std::unique_ptr<Ternary> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedGrouping : public TypedExpr {
public:
    TypedGrouping(std::unique_ptr<Grouping> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedUnary : public TypedExpr {
public:
    TypedUnary(std::unique_ptr<Unary> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedAssign : public TypedExpr {
public:
    TypedAssign(std::unique_ptr<Assign> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedArrayLiteral : public TypedExpr {
public:
    TypedArrayLiteral(std::unique_ptr<ArrayLiteral> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedDictionaryLiteral : public TypedExpr {
public:
    TypedDictionaryLiteral(std::unique_ptr<DictionaryLiteral> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedTupleLiteral : public TypedExpr {
public:
    TypedTupleLiteral(std::unique_ptr<TupleLiteral> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedIndexAccess : public TypedExpr {
public:
    TypedIndexAccess(std::unique_ptr<IndexAccess> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedLabeledCall : public TypedExpr {
public:
    TypedLabeledCall(std::unique_ptr<LabeledCall> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedClosure : public TypedExpr {
public:
    TypedClosure(std::unique_ptr<Closure> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedMemberAccess : public TypedExpr {
public:
    TypedMemberAccess(std::unique_ptr<MemberAccess> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedStructInit : public TypedExpr {
public:
    TypedStructInit(std::unique_ptr<StructInit> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedTypeCheck : public TypedExpr {
public:
    TypedTypeCheck(std::unique_ptr<TypeCheck> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

class TypedTypeCast : public TypedExpr {
public:
    TypedTypeCast(std::unique_ptr<TypeCast> originalExpr, std::shared_ptr<Type> type)
        : TypedExpr(std::move(originalExpr), type) {}
    void accept(TypedExprVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedExpr> clone() const override { return nullptr; } // TODO: 实现
};

// 语句占位符
class TypedExprStmt : public TypedStmt {
public:
    TypedExprStmt(std::unique_ptr<ExprStmt> originalStmt, std::unique_ptr<TypedExpr> typedExpression = nullptr)
        : TypedStmt(std::move(originalStmt)), typedExpression_(std::move(typedExpression)) {}
    
    const TypedExpr* getTypedExpression() const { return typedExpression_.get(); }
    
    void accept(TypedStmtVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedStmt> clone() const override { return nullptr; } // TODO: 实现
    
private:
    std::unique_ptr<TypedExpr> typedExpression_;
};

class TypedIfStmt : public TypedStmt {
public:
    TypedIfStmt(std::unique_ptr<IfStmt> originalStmt)
        : TypedStmt(std::move(originalStmt)) {}
    void accept(TypedStmtVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedStmt> clone() const override { return nullptr; } // TODO: 实现
};

class TypedWhileStmt : public TypedStmt {
public:
    TypedWhileStmt(std::unique_ptr<WhileStmt> originalStmt)
        : TypedStmt(std::move(originalStmt)) {}
    void accept(TypedStmtVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedStmt> clone() const override { return nullptr; } // TODO: 实现
};

class TypedForStmt : public TypedStmt {
public:
    TypedForStmt(std::unique_ptr<ForStmt> originalStmt)
        : TypedStmt(std::move(originalStmt)) {}
    void accept(TypedStmtVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedStmt> clone() const override { return nullptr; } // TODO: 实现
};

class TypedReturnStmt : public TypedStmt {
public:
    TypedReturnStmt(std::unique_ptr<ReturnStmt> originalStmt)
        : TypedStmt(std::move(originalStmt)) {}
    void accept(TypedStmtVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedStmt> clone() const override { return nullptr; } // TODO: 实现
};

class TypedStructStmt : public TypedStmt {
public:
    TypedStructStmt(std::unique_ptr<StructStmt> originalStmt)
        : TypedStmt(std::move(originalStmt)) {}
    void accept(TypedStmtVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedStmt> clone() const override { return nullptr; } // TODO: 实现
};

class TypedContinueStmt : public TypedStmt {
public:
    TypedContinueStmt(std::unique_ptr<ContinueStmt> originalStmt)
        : TypedStmt(std::move(originalStmt)) {}
    void accept(TypedStmtVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedStmt> clone() const override { return nullptr; } // TODO: 实现
};

class TypedBreakStmt : public TypedStmt {
public:
    TypedBreakStmt(std::unique_ptr<BreakStmt> originalStmt)
        : TypedStmt(std::move(originalStmt)) {}
    void accept(TypedStmtVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedStmt> clone() const override { return nullptr; } // TODO: 实现
};

class TypedFallthroughStmt : public TypedStmt {
public:
    TypedFallthroughStmt(std::unique_ptr<FallthroughStmt> originalStmt)
        : TypedStmt(std::move(originalStmt)) {}
    void accept(TypedStmtVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedStmt> clone() const override { return nullptr; } // TODO: 实现
};

class TypedClassStmt : public TypedStmt {
public:
    TypedClassStmt(std::unique_ptr<ClassStmt> originalStmt)
        : TypedStmt(std::move(originalStmt)) {}
    void accept(TypedStmtVisitor& visitor) const override { visitor.visit(*this); }
    std::unique_ptr<TypedStmt> clone() const override { return nullptr; } // TODO: 实现
};

} // namespace miniswift

#endif // MINISWIFT_TYPED_AST_H