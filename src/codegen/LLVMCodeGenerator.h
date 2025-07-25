#ifndef MINISWIFT_LLVM_CODE_GENERATOR_H
#define MINISWIFT_LLVM_CODE_GENERATOR_H

#include "../semantic/TypedAST.h"
#include "../semantic/TypeSystem.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

namespace miniswift {

// 代码生成错误类型
struct CodeGenError {
    std::string message;
    int line;
    int column;
    
    CodeGenError(const std::string& msg, int l = -1, int c = -1)
        : message(msg), line(l), column(c) {}
};

// 代码生成结果
struct CodeGenResult {
    std::unique_ptr<llvm::Module> module;
    std::vector<CodeGenError> errors;
    std::vector<std::string> warnings;
    bool hasErrors() const { return !errors.empty(); }
};

// 变量信息结构
struct VariableInfo {
    llvm::Value* value;
    llvm::Type* type;
    bool isConst;
    bool isGlobal;
    
    VariableInfo() : value(nullptr), type(nullptr), isConst(false), isGlobal(false) {}
    VariableInfo(llvm::Value* v, llvm::Type* t, bool c = false, bool g = false)
        : value(v), type(t), isConst(c), isGlobal(g) {}
};

// 函数信息结构
struct FunctionInfo {
    llvm::Function* function;
    std::shared_ptr<FunctionType> swiftType;
    std::vector<std::string> parameterNames;
    
    FunctionInfo() : function(nullptr), swiftType(nullptr) {}
    FunctionInfo(llvm::Function* f, std::shared_ptr<FunctionType> t, const std::vector<std::string>& names)
        : function(f), swiftType(t), parameterNames(names) {}
};

// 类型信息结构
struct TypeInfo {
    llvm::Type* llvmType;
    std::shared_ptr<Type> swiftType;
    bool isValueType;
    size_t size;
    
    TypeInfo(llvm::Type* lt, std::shared_ptr<Type> st, bool vt = true, size_t s = 0)
        : llvmType(lt), swiftType(st), isValueType(vt), size(s) {}
};

// LLVM代码生成器主类
class LLVMCodeGenerator : public TypedExprVisitor, public TypedStmtVisitor {
public:
    LLVMCodeGenerator();
    virtual ~LLVMCodeGenerator() = default;
    
    // 主要生成接口
    CodeGenResult generateCode(const TypedProgram& program);
    CodeGenResult generateModule(const std::string& moduleName, const TypedProgram& program);
    
    // JIT编译和执行
    bool initializeJIT();
    llvm::GenericValue executeFunction(const std::string& functionName, const std::vector<llvm::GenericValue>& args);
    void* getCompiledFunction(const std::string& functionName);
    
    // AOT编译
    bool compileToObjectFile(const std::string& filename);
    bool compileToExecutable(const std::string& filename);
    bool compileToAssembly(const std::string& filename);
    
    // 优化
    void optimizeModule(llvm::Module* module);
    void addOptimizationPasses(llvm::legacy::PassManager& passManager);
    
    // 类型化表达式访问者实现
    void visit(const TypedBinary& expr) override;
    void visit(const TypedTernary& expr) override;
    void visit(const TypedGrouping& expr) override;
    void visit(const TypedLiteral& expr) override;
    void visit(const TypedUnary& expr) override;
    void visit(const TypedVarExpr& expr) override;
    void visit(const TypedAssign& expr) override;
    void visit(const TypedArrayLiteral& expr) override;
    void visit(const TypedDictionaryLiteral& expr) override;
    void visit(const TypedTupleLiteral& expr) override;
    void visit(const TypedIndexAccess& expr) override;
    void visit(const TypedCall& expr) override;
    void visit(const TypedLabeledCall& expr) override;
    void visit(const TypedClosure& expr) override;
    void visit(const TypedMemberAccess& expr) override;
    void visit(const TypedStructInit& expr) override;
    void visit(const TypedTypeCheck& expr) override;
    void visit(const TypedTypeCast& expr) override;
    
    // 类型化语句访问者实现
    void visit(const TypedExprStmt& stmt) override;
    void visit(const TypedVarStmt& stmt) override;
    void visit(const TypedBlockStmt& stmt) override;
    void visit(const TypedIfStmt& stmt) override;
    void visit(const TypedWhileStmt& stmt) override;
    void visit(const TypedForStmt& stmt) override;
    void visit(const TypedReturnStmt& stmt) override;
    void visit(const TypedFunctionStmt& stmt) override;
    void visit(const TypedStructStmt& stmt) override;
    void visit(const TypedClassStmt& stmt) override;
    
private:
    // LLVM核心组件
    std::unique_ptr<llvm::LLVMContext> context_;
    std::unique_ptr<llvm::Module> module_;
    std::unique_ptr<llvm::IRBuilder<>> builder_;
    std::unique_ptr<llvm::ExecutionEngine> executionEngine_;
    std::unique_ptr<llvm::orc::LLJIT> jit_;
    
    // 符号表和类型映射
    std::unordered_map<std::string, VariableInfo> variables_;
    std::unordered_map<std::string, FunctionInfo> functions_;
    std::unordered_map<std::string, TypeInfo> types_;
    
    // 类型系统
    std::unique_ptr<TypeSystem> typeSystem_;
    
    // 当前生成状态
    llvm::Value* currentValue_;
    llvm::Function* currentFunction_;
    llvm::BasicBlock* currentBlock_;
    std::vector<CodeGenError> errors_;
    std::vector<std::string> warnings_;
    
    // 作用域管理
    struct Scope {
        std::unordered_map<std::string, VariableInfo> localVariables;
        llvm::BasicBlock* entryBlock;
        llvm::BasicBlock* exitBlock;
    };
    std::vector<Scope> scopeStack_;
    
    // 类型转换
    llvm::Type* convertSwiftTypeToLLVM(const std::shared_ptr<Type>& swiftType);
    llvm::Type* getPrimitiveType(TypeKind kind);
    llvm::Type* getArrayType(const std::shared_ptr<ArrayType>& arrayType);
    llvm::Type* getDictionaryType(const std::shared_ptr<DictionaryType>& dictType);
    llvm::Type* getTupleType(const std::shared_ptr<TupleType>& tupleType);
    llvm::Type* getFunctionType(const std::shared_ptr<FunctionType>& funcType);
    llvm::Type* getStructType(const std::shared_ptr<Type>& structType);
    llvm::Type* getClassType(const std::shared_ptr<Type>& classType);
    
    // 值生成
    llvm::Value* generateExpression(const TypedExpr& expr);
    llvm::Value* generateExpression(const Expr* expr);  // 重载版本用于原始Expr
    void generateStatement(const TypedStmt& stmt);
    void generateStatement(const Stmt* stmt);  // 重载版本用于原始Stmt
    llvm::Value* generateLiteral(const TypedLiteral& literal);
    llvm::Value* generateBinaryOperation(const TypedBinary& binary);
    llvm::Value* generateUnaryOperation(const TypedUnary& unary);
    llvm::Value* generateFunctionCall(const TypedCall& call);
    llvm::Value* generatePrintCall(const TypedCall& call);
    llvm::Value* generateVariableAccess(const TypedVarExpr& varExpr);
    llvm::Value* generateAssignment(const TypedAssign& assign);
    
    // 控制流生成
    void generateIfStatement(const TypedIfStmt& ifStmt);
    void generateWhileLoop(const TypedWhileStmt& whileStmt);
    void generateForLoop(const TypedForStmt& forStmt);
    void generateReturnStatement(const TypedReturnStmt& returnStmt);
    
    // 函数生成
    llvm::Function* generateFunction(const TypedFunctionStmt& funcStmt);
    void generateFunctionBody(llvm::Function* function, const TypedStmt& body);
    llvm::Function* createFunctionDeclaration(const std::string& name, const std::shared_ptr<FunctionType>& funcType);
    
    // 类型定义生成
    llvm::StructType* generateStructType(const TypedStructStmt& structStmt);
    llvm::StructType* generateClassType(const TypedClassStmt& classStmt);
    
    // 内存管理
    llvm::Value* allocateVariable(const std::string& name, llvm::Type* type, bool isGlobal = false);
    llvm::Value* allocateArray(llvm::Type* elementType, llvm::Value* size);
    llvm::Value* allocateStruct(llvm::StructType* structType);
    void generateDeallocation(llvm::Value* ptr);
    
    // 运行时支持
    void generateRuntimeFunctions();
    llvm::Function* createPrintFunction();
    llvm::Function* createMemoryAllocationFunction();
    llvm::Function* createMemoryDeallocationFunction();
    llvm::Function* createArrayAccessFunction();
    llvm::Function* createStringConcatenationFunction();
    
    // 错误处理
    void reportError(const std::string& message, int line = -1, int column = -1);
    void reportWarning(const std::string& message);
    
    // 作用域管理
    void enterScope();
    void exitScope();
    VariableInfo* lookupVariable(const std::string& name);
    void declareVariable(const std::string& name, const VariableInfo& info);
    
    // 基本块管理
    llvm::BasicBlock* createBasicBlock(const std::string& name, llvm::Function* function = nullptr);
    void setCurrentBlock(llvm::BasicBlock* block);
    
    // 类型检查和转换
    bool isCompatibleType(llvm::Type* from, llvm::Type* to);
    llvm::Value* convertValue(llvm::Value* value, llvm::Type* targetType);
    llvm::Value* createImplicitCast(llvm::Value* value, llvm::Type* targetType);
    
    // 常量生成
    llvm::Constant* createIntConstant(int64_t value, llvm::Type* type = nullptr);
    llvm::Constant* createFloatConstant(double value, llvm::Type* type = nullptr);
    llvm::Constant* createStringConstant(const std::string& value);
    llvm::Constant* createBoolConstant(bool value);
    llvm::Constant* createNullConstant(llvm::Type* type);
    
    // 调试信息生成
    void generateDebugInfo(const TypedStmt& stmt);
    void generateDebugInfo(const TypedExpr& expr);
    
    // 优化辅助
    void runBasicOptimizations();
    void runAdvancedOptimizations();
    
    // 目标平台支持
    void initializeTargetMachine();
    llvm::TargetMachine* getTargetMachine();
    
    // 模块验证
    bool verifyModule();
    void dumpModule();
    
    // Swift特定功能
    void generateSwiftRuntimeSupport();
    llvm::Function* createSwiftStringType();
    llvm::Function* createSwiftArrayType();
    llvm::Function* createSwiftDictionaryType();
    llvm::Function* createSwiftOptionalType();
    
    // ARC支持
    void generateARCSupport();
    llvm::Function* createRetainFunction();
    llvm::Function* createReleaseFunction();
    void insertRetain(llvm::Value* object);
    void insertRelease(llvm::Value* object);
    
    // 并发支持
    void generateConcurrencySupport();
    llvm::Function* createAsyncFunction();
    llvm::Function* createAwaitFunction();
    llvm::Function* createTaskFunction();
};

// LLVM代码生成器工厂
class LLVMCodeGeneratorFactory {
public:
    static std::unique_ptr<LLVMCodeGenerator> create();
    static std::unique_ptr<LLVMCodeGenerator> createWithOptimizations(bool enableOptimizations = true);
    static std::unique_ptr<LLVMCodeGenerator> createForTarget(const std::string& targetTriple);
};

// 编译管道
class CompilationPipeline {
public:
    CompilationPipeline();
    
    // 设置编译选项
    void setOptimizationLevel(int level); // 0-3
    void setTargetTriple(const std::string& triple);
    void setOutputFormat(const std::string& format); // "object", "assembly", "executable"
    void enableDebugInfo(bool enable);
    void enableJIT(bool enable);
    
    // 编译流程
    bool compile(const TypedProgram& program, const std::string& outputFile);
    bool compileAndRun(const TypedProgram& program, const std::string& mainFunction = "main");
    
    // 获取编译结果
    const std::vector<CodeGenError>& getErrors() const { return errors_; }
    const std::vector<std::string>& getWarnings() const { return warnings_; }
    
private:
    std::unique_ptr<LLVMCodeGenerator> codeGenerator_;
    int optimizationLevel_;
    std::string targetTriple_;
    std::string outputFormat_;
    bool debugInfo_;
    bool jitEnabled_;
    std::vector<CodeGenError> errors_;
    std::vector<std::string> warnings_;
};

} // namespace miniswift

#endif // MINISWIFT_LLVM_CODE_GENERATOR_H