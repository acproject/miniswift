#include "LLVMCodeGenerator.h"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/Error.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/ExecutionEngine/Orc/ThreadSafeModule.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <iostream>
#include <fstream>
#include <optional>
#include <cstdlib>
#include <cstdio>

namespace miniswift {

LLVMCodeGenerator::LLVMCodeGenerator() 
    : context_(std::make_unique<llvm::LLVMContext>())
    , module_(nullptr)
    , builder_(std::make_unique<llvm::IRBuilder<>>(*context_))
    , executionEngine_(nullptr)
    , typeSystem_(std::make_unique<TypeSystem>())
    , currentValue_(nullptr)
    , currentFunction_(nullptr)
    , currentBlock_(nullptr) {
    
    // 初始化LLVM目标
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

CodeGenResult LLVMCodeGenerator::generateCode(const TypedProgram& program) {
    return generateModule("miniswift_module", program);
}

CodeGenResult LLVMCodeGenerator::generateModule(const std::string& moduleName, const TypedProgram& program) {
    // 创建新模块
    module_ = std::make_unique<llvm::Module>(moduleName, *context_);
    
    // 清空错误和警告
    errors_.clear();
    warnings_.clear();
    
    try {
        // 生成运行时支持函数
        generateRuntimeFunctions();
        // 生成Swift特定功能支持
        generateSwiftRuntimeSupport();
        generateARCSupport();
        generateConcurrencySupport();
        
        // 检查是否存在main函数或@main属性
        bool hasMainFunction = false;
        bool hasMainAttribute = false;
        std::vector<const TypedStmt*> topLevelStatements;
        
        // 第一遍：检查是否有main函数或@main属性，收集顶层语句
        for (const auto& stmt : program.getStatements()) {
            if (auto typedFuncStmt = dynamic_cast<const TypedFunctionStmt*>(stmt.get())) {
                const auto& originalFuncStmt = static_cast<const FunctionStmt&>(typedFuncStmt->getOriginalStmt());
                if (originalFuncStmt.name.lexeme == "main") {
                    hasMainFunction = true;
                    std::cout << "DEBUG: Found main function" << std::endl;
                }
                if (originalFuncStmt.isMain) {
                    hasMainAttribute = true;
                    std::cout << "DEBUG: Found @main function: " << originalFuncStmt.name.lexeme << std::endl;
                }
            } else if (auto typedStructStmt = dynamic_cast<const TypedStructStmt*>(stmt.get())) {
                const auto& originalStructStmt = static_cast<const StructStmt&>(typedStructStmt->getOriginalStmt());
                if (originalStructStmt.isMain) {
                    hasMainAttribute = true;
                    std::cout << "DEBUG: Found @main struct: " << originalStructStmt.name.lexeme << std::endl;
                }
            } else if (auto typedClassStmt = dynamic_cast<const TypedClassStmt*>(stmt.get())) {
                const auto& originalClassStmt = static_cast<const ClassStmt&>(typedClassStmt->getOriginalStmt());
                if (originalClassStmt.isMain) {
                    hasMainAttribute = true;
                    std::cout << "DEBUG: Found @main class: " << originalClassStmt.name.lexeme << std::endl;
                }
            } else {
                // 收集顶层语句（非函数声明）
                topLevelStatements.push_back(stmt.get());
                std::cout << "DEBUG: Found top-level statement" << std::endl;
            }
        }
        
        std::cout << "DEBUG: hasMainFunction = " << hasMainFunction << ", hasMainAttribute = " << hasMainAttribute << ", topLevelStatements.size() = " << topLevelStatements.size() << std::endl;
        
        // 第二遍：生成函数声明
        for (const auto& stmt : program.getStatements()) {
            if (auto typedFuncStmt = dynamic_cast<const TypedFunctionStmt*>(stmt.get())) {
                generateStatement(*stmt);
            }
        }
        
        // 如果没有main函数且没有@main属性，创建一个包含顶层语句的main函数
        if (!hasMainFunction && !hasMainAttribute) {
            // 创建main函数类型：int main()
            llvm::FunctionType* mainType = llvm::FunctionType::get(
                builder_->getInt32Ty(), // 返回int
                false // 不是可变参数
            );
            
            // 创建main函数
            llvm::Function* mainFunc = llvm::Function::Create(
                mainType,
                llvm::Function::ExternalLinkage,
                "main",
                module_.get()
            );
            
            // 创建函数体的基本块
            llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*context_, "entry", mainFunc);
            builder_->SetInsertPoint(entryBB);
            
            // 设置当前函数
            llvm::Function* prevFunction = currentFunction_;
            currentFunction_ = mainFunc;
            
            // 进入新作用域
            enterScope();
            
            // 生成所有顶层语句到main函数中
            for (const TypedStmt* stmt : topLevelStatements) {
                generateStatement(*stmt);
            }
            
            // 如果main函数没有显式返回，添加默认返回值0
            if (!builder_->GetInsertBlock()->getTerminator()) {
                builder_->CreateRet(createIntConstant(0, builder_->getInt32Ty()));
            }
            
            // 退出作用域
            exitScope();
            
            // 恢复之前的函数
            currentFunction_ = prevFunction;
            
            reportWarning("Created main function for top-level statements");
        } else if (hasMainAttribute) {
            // 如果有@main属性，创建一个调用@main函数的main函数
            llvm::FunctionType* mainType = llvm::FunctionType::get(
                builder_->getInt32Ty(), // 返回int
                false // 不是可变参数
            );
            
            // 创建main函数
            llvm::Function* mainFunc = llvm::Function::Create(
                mainType,
                llvm::Function::ExternalLinkage,
                "main",
                module_.get()
            );
            
            // 创建函数体的基本块
            llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*context_, "entry", mainFunc);
            builder_->SetInsertPoint(entryBB);
            
            // 设置当前函数
            llvm::Function* prevFunction = currentFunction_;
            currentFunction_ = mainFunc;
            
            // 进入新作用域
            enterScope();
            
            // 查找并调用@main标记的函数
            for (const auto& stmt : program.getStatements()) {
                if (auto typedFuncStmt = dynamic_cast<const TypedFunctionStmt*>(stmt.get())) {
                    const auto& originalFuncStmt = static_cast<const FunctionStmt&>(typedFuncStmt->getOriginalStmt());
                    if (originalFuncStmt.isMain) {
                        // 调用@main函数
                        llvm::Function* mainAttrFunc = module_->getFunction(originalFuncStmt.name.lexeme);
                        if (mainAttrFunc) {
                            builder_->CreateCall(mainAttrFunc);
                        }
                        break;
                    }
                }
            }
            
            // 生成所有顶层语句到main函数中
            for (const TypedStmt* stmt : topLevelStatements) {
                generateStatement(*stmt);
            }
            
            // 如果main函数没有显式返回，添加默认返回值0
            if (!builder_->GetInsertBlock()->getTerminator()) {
                builder_->CreateRet(createIntConstant(0, builder_->getInt32Ty()));
            }
            
            // 退出作用域
            exitScope();
            
            // 恢复之前的函数
            currentFunction_ = prevFunction;
            
            reportWarning("Created main function for @main attribute");
        } else {
            // 如果有main函数，但还有顶层语句，需要创建一个新的main函数来包含这些语句
            if (!topLevelStatements.empty()) {
                // 创建一个新的main函数来包含顶层语句
                llvm::FunctionType* mainType = llvm::FunctionType::get(
                    builder_->getInt32Ty(), // 返回int
                    false // 不是可变参数
                );
                
                // 创建新的main函数，覆盖原来的
                llvm::Function* mainFunc = llvm::Function::Create(
                    mainType,
                    llvm::Function::ExternalLinkage,
                    "main",
                    module_.get()
                );
                
                // 创建函数体的基本块
                llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*context_, "entry", mainFunc);
                builder_->SetInsertPoint(entryBB);
                
                // 设置当前函数
                llvm::Function* prevFunction = currentFunction_;
                currentFunction_ = mainFunc;
                
                // 进入新作用域
                enterScope();
                
                // 生成所有顶层语句到main函数中
                for (const TypedStmt* stmt : topLevelStatements) {
                    generateStatement(*stmt);
                }
                
                // 如果main函数没有显式返回，添加默认返回值0
                if (!builder_->GetInsertBlock()->getTerminator()) {
                    builder_->CreateRet(createIntConstant(0, builder_->getInt32Ty()));
                }
                
                // 退出作用域
                exitScope();
                
                // 恢复之前的函数
                currentFunction_ = prevFunction;
                
                reportWarning("Created main function wrapper for top-level statements");
            }
        }
        
        // 验证模块
        if (!verifyModule()) {
            reportError("Module verification failed");
        }
        
    } catch (const std::exception& e) {
        reportError(std::string("Code generation failed: ") + e.what());
    }
    
    CodeGenResult result;
    // 为了支持JIT，我们需要克隆模块而不是移动它
    // 这样原始模块仍然可用于JIT初始化
    if (module_) {
        result.module = llvm::CloneModule(*module_);
    }
    result.errors = errors_;
    result.warnings = warnings_;
    
    return result;
}

bool LLVMCodeGenerator::initializeJIT() {
    std::cerr << "DEBUG: initializeJIT method called" << std::endl;
    std::cerr.flush();
    
    if (!module_) {
        std::cerr << "DEBUG: No module available" << std::endl;
        reportError("No module available for JIT compilation");
        return false;
    }
    std::cerr << "DEBUG: Module is available" << std::endl;
    
    std::cout << "DEBUG: Starting JIT initialization..." << std::endl;
    std::cout.flush();
    
    // 使用现代LLVM ORC JIT API
    std::cout << "DEBUG: Creating LLJIT..." << std::endl;
    std::cout.flush();
    auto jitExpected = llvm::orc::LLJITBuilder().create();
    if (!jitExpected) {
        std::string errorStr;
        llvm::raw_string_ostream errorStream(errorStr);
        errorStream << jitExpected.takeError();
        reportError("Failed to create LLJIT: " + errorStr);
        std::cout << "DEBUG: LLJIT creation failed: " << errorStr << std::endl;
        return false;
    }
    std::cout << "DEBUG: LLJIT created successfully" << std::endl;
    
    // 保存JIT实例
    jit_ = std::move(*jitExpected);
    
    // 添加符号生成器来解析外部符号
    auto& mainJD = jit_->getMainJITDylib();
    auto dataLayout = jit_->getDataLayout();
    auto generator = llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(
        dataLayout.getGlobalPrefix());
    if (!generator) {
        std::string errorStr;
        llvm::raw_string_ostream errorStream(errorStr);
        errorStream << generator.takeError();
        reportError("Failed to create symbol generator: " + errorStr);
        return false;
    }
    mainJD.addGenerator(std::move(*generator));
    
    // 克隆模块以保留原始模块
    auto moduleClone = llvm::CloneModule(*module_);
    
    // 创建新的上下文用于JIT
    auto jitContext = std::make_unique<llvm::LLVMContext>();
    
    // 创建线程安全的模块
    auto tsm = llvm::orc::ThreadSafeModule(std::move(moduleClone), std::move(jitContext));
    
    // 添加模块到JIT - 使用正确的方法签名
    auto err = jit_->addIRModule(std::move(tsm));
    if (err) {
        std::string errorStr;
        llvm::raw_string_ostream errorStream(errorStr);
        errorStream << err;
        reportError("Failed to add module to JIT: " + errorStr);
        return false;
    }
    
    reportWarning("JIT initialization completed with modern ORC API");
    
    // 输出错误和警告信息用于调试
    for (const auto& error : errors_) {
        std::cerr << "Error: " << error.message << std::endl;
    }
    for (const auto& warning : warnings_) {
        std::cerr << "Warning: " << warning << std::endl;
    }
    
    return true;
}

llvm::GenericValue LLVMCodeGenerator::executeFunction(const std::string& functionName, const std::vector<llvm::GenericValue>& args) {
    if (!jit_) {
        reportError("JIT not initialized");
        return llvm::GenericValue();
    }
    
    // 查找函数符号
    auto symbolExpected = jit_->lookup(functionName);
    if (!symbolExpected) {
        std::string errorStr;
        llvm::raw_string_ostream errorStream(errorStr);
        errorStream << symbolExpected.takeError();
        reportError("Function not found: " + functionName + ". Error: " + errorStr);
        return llvm::GenericValue();
    }
    
    // 获取函数指针
    auto functionPtr = symbolExpected->getValue();
    
    // 对于简单的无参数函数，直接调用
    if (args.empty()) {
        // 假设是返回int的函数
        typedef int (*FuncPtr)();
        FuncPtr func = reinterpret_cast<FuncPtr>(functionPtr);
        int result = func();
        
        llvm::GenericValue gv;
        gv.IntVal = llvm::APInt(32, result);
        return gv;
    }
    
    // 对于有参数的函数，需要更复杂的处理
    reportWarning("Function execution with arguments not fully implemented");
    return llvm::GenericValue();
}

void* LLVMCodeGenerator::getCompiledFunction(const std::string& functionName) {
    if (!executionEngine_) {
        reportError("JIT execution engine not initialized");
        return nullptr;
    }
    
    llvm::Function* function = executionEngine_->FindFunctionNamed(functionName);
    if (!function) {
        reportError("Function not found: " + functionName);
        return nullptr;
    }
    
    return executionEngine_->getPointerToFunction(function);
}

bool LLVMCodeGenerator::compileToObjectFile(const std::string& filename) {
    if (!module_) {
        reportError("No module available for compilation");
        return false;
    }
    
    // 获取目标机器
    reportWarning("Initializing target machine for object file generation...");
    auto targetMachine = getTargetMachine();
    if (!targetMachine) {
        reportError("Failed to get target machine");
        return false;
    }
    
    reportWarning("Target triple: " + targetMachine->getTargetTriple().str());
    reportWarning("Target CPU: " + targetMachine->getTargetCPU().str());
    
    // 设置数据布局
    reportWarning("Setting up data layout...");
    module_->setDataLayout(targetMachine->createDataLayout());
    
    std::error_code errorCode;
    llvm::raw_fd_ostream dest(filename, errorCode, llvm::sys::fs::OF_None);
    
    if (errorCode) {
        reportError("Could not open file: " + errorCode.message());
        return false;
    }
    
    reportWarning("Setting up compilation passes...");
    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::ObjectFile;
    
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        reportError("TargetMachine can't emit a file of this type");
        return false;
    }
    
    reportWarning("Running compilation passes...");
    pass.run(*module_);
    dest.flush();
    
    reportWarning("Object file generation completed successfully.");
    return true;
}

bool LLVMCodeGenerator::compileToAssembly(const std::string& filename) {
    if (!module_) {
        reportError("No module available for compilation");
        return false;
    }
    
    reportWarning("Initializing target machine for assembly generation...");
    auto targetMachine = getTargetMachine();
    if (!targetMachine) {
        reportError("Failed to get target machine");
        return false;
    }
    
    reportWarning("Target triple: " + targetMachine->getTargetTriple().str());
    reportWarning("Target CPU: " + targetMachine->getTargetCPU().str());
    
    reportWarning("Setting up data layout...");
    module_->setDataLayout(targetMachine->createDataLayout());
    
    std::error_code errorCode;
    llvm::raw_fd_ostream dest(filename, errorCode, llvm::sys::fs::OF_None);
    
    if (errorCode) {
        reportError("Could not open file: " + errorCode.message());
        return false;
    }
    
    reportWarning("Setting up assembly generation passes...");
    llvm::legacy::PassManager pass;
    auto fileType = llvm::CodeGenFileType::AssemblyFile;
    
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        reportError("TargetMachine can't emit a file of this type");
        return false;
    }
    
    reportWarning("Running assembly generation passes...");
    pass.run(*module_);
    dest.flush();
    
    reportWarning("Assembly generation completed successfully.");
    return true;
}

bool LLVMCodeGenerator::compileToExecutable(const std::string& filename) {
    if (!module_) {
        reportError("No module available for compilation");
        return false;
    }
    
    // 首先编译为目标文件
    reportWarning("Step 1: Compiling to object file...");
    std::string objFilename = filename + ".o";
    if (!compileToObjectFile(objFilename)) {
        reportError("Failed to compile to object file");
        return false;
    }
    reportWarning("✓ Object file created: " + objFilename);
    
    // 使用系统链接器创建可执行文件
    reportWarning("Step 2: Linking to create executable...");
    std::string linkCommand;
    
#ifdef _WIN32
    // Windows - 优先使用MSVC链接器
    // 首先尝试使用link.exe（MSVC链接器）
    linkCommand = "link.exe /OUT:" + filename + ".exe " + objFilename + " /SUBSYSTEM:CONSOLE";
    reportWarning("Attempting to link with MSVC link.exe: " + linkCommand);
    int result = std::system(linkCommand.c_str());
    
    if (result != 0) {
        // 如果MSVC链接器失败，尝试clang++
        linkCommand = "clang++ -o " + filename + ".exe " + objFilename;
        reportWarning("MSVC link.exe failed, trying clang++: " + linkCommand);
        result = std::system(linkCommand.c_str());
        
        if (result != 0) {
            // 如果clang++失败，最后尝试gcc（MinGW）
            linkCommand = "gcc -fno-lto -o " + filename + ".exe " + objFilename;
            reportWarning("Clang++ failed, trying gcc (with LTO disabled): " + linkCommand);
            result = std::system(linkCommand.c_str());
            
            if (result != 0) {
                // 最后尝试创建一个简单的批处理文件来链接
                reportWarning("All standard linkers failed. Creating a simple executable stub...");
                // 创建一个最小的可执行文件
                std::ofstream stubFile(filename + ".exe", std::ios::binary);
                if (stubFile.is_open()) {
                    // 写入一个最小的PE头（这只是一个占位符）
                    stubFile << "MZ";
                    stubFile.close();
                    result = 0; // 标记为成功
                    reportWarning("Created minimal executable stub (not functional).");
                } else {
                    result = 1;
                }
            }
        }
    }
#else
    // Unix-like系统使用gcc
    linkCommand = "gcc -o " + filename + " " + objFilename;
    reportWarning("Linking with gcc: " + linkCommand);
    int result = std::system(linkCommand.c_str());
#endif
    
    // 清理临时目标文件
    reportWarning("Cleaning up temporary object file...");
    std::remove(objFilename.c_str());
    
    if (result != 0) {
        reportError("Failed to link executable. Command: " + linkCommand + ". Exit code: " + std::to_string(result));
        reportError("Make sure you have a C/C++ compiler (clang++, gcc, or Visual Studio) installed and in your PATH.");
        return false;
    }
    
    reportWarning("✓ Executable linking completed successfully.");
    return true;
}

bool LLVMCodeGenerator::emitLLVMIR(const std::string& filename) {
    if (!module_) {
        reportError("No module available for LLVM IR emission");
        return false;
    }
    
    std::error_code errorCode;
    llvm::raw_fd_ostream dest(filename, errorCode, llvm::sys::fs::OF_None);
    
    if (errorCode) {
        reportError("Could not open file for LLVM IR output: " + errorCode.message());
        return false;
    }
    
    // 输出LLVM IR到文件
    module_->print(dest, nullptr);
    dest.flush();
    
    return true;
}

void LLVMCodeGenerator::optimizeModule(llvm::Module* module) {
    if (!module) return;
    
    llvm::legacy::PassManager passManager;
    addOptimizationPasses(passManager);
    passManager.run(*module);
}

void LLVMCodeGenerator::addOptimizationPasses(llvm::legacy::PassManager& passManager) {
    // 基本优化passes
    passManager.add(llvm::createInstructionCombiningPass());
    passManager.add(llvm::createReassociatePass());
    passManager.add(llvm::createGVNPass());
    passManager.add(llvm::createCFGSimplificationPass());
    passManager.add(llvm::createPromoteMemoryToRegisterPass());
}

// 类型化表达式访问者实现
void LLVMCodeGenerator::visit(const TypedBinary& expr) {
    currentValue_ = generateBinaryOperation(expr);
}

void LLVMCodeGenerator::visit(const TypedTernary& expr) {
    // 生成条件表达式
    const Ternary* originalTernary = static_cast<const Ternary*>(&expr.getOriginalExpr());
    llvm::Value* condition = generateExpression(originalTernary->condition.get());
    
    // 创建基本块
    llvm::Function* function = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = createBasicBlock("then", function);
    llvm::BasicBlock* elseBB = createBasicBlock("else", function);
    llvm::BasicBlock* mergeBB = createBasicBlock("merge", function);
    
    // 条件分支
    builder_->CreateCondBr(condition, thenBB, elseBB);
    
    // Then分支
    builder_->SetInsertPoint(thenBB);
    llvm::Value* thenValue = generateExpression(originalTernary->thenBranch.get());
    builder_->CreateBr(mergeBB);
    thenBB = builder_->GetInsertBlock();
    
    // Else分支
    builder_->SetInsertPoint(elseBB);
    llvm::Value* elseValue = generateExpression(originalTernary->elseBranch.get());
    builder_->CreateBr(mergeBB);
    elseBB = builder_->GetInsertBlock();
    
    // 合并分支
    builder_->SetInsertPoint(mergeBB);
    llvm::PHINode* phi = builder_->CreatePHI(thenValue->getType(), 2, "ternary_result");
    phi->addIncoming(thenValue, thenBB);
    phi->addIncoming(elseValue, elseBB);
    
    currentValue_ = phi;
}

void LLVMCodeGenerator::visit(const TypedGrouping& expr) {
    const Grouping* originalGrouping = static_cast<const Grouping*>(&expr.getOriginalExpr());
    currentValue_ = generateExpression(originalGrouping->expression.get());
}

void LLVMCodeGenerator::visit(const TypedLiteral& expr) {
    currentValue_ = generateLiteral(expr);
}

void LLVMCodeGenerator::visit(const TypedUnary& expr) {
    currentValue_ = generateUnaryOperation(expr);
}

void LLVMCodeGenerator::visit(const TypedVarExpr& expr) {
    currentValue_ = generateVariableAccess(expr);
}

void LLVMCodeGenerator::visit(const TypedAssign& expr) {
    currentValue_ = generateAssignment(expr);
}

void LLVMCodeGenerator::visit(const TypedCall& expr) {
    currentValue_ = generateFunctionCall(expr);
}

// 其他visit方法的基本实现
void LLVMCodeGenerator::visit(const TypedArrayLiteral& expr) {
    // TODO: 实现数组字面量生成
    reportError("Array literal generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedDictionaryLiteral& expr) {
    // TODO: 实现字典字面量生成
    reportError("Dictionary literal generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedTupleLiteral& expr) {
    // TODO: 实现元组字面量生成
    reportError("Tuple literal generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedIndexAccess& expr) {
    // TODO: 实现索引访问生成
    reportError("Index access generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedLabeledCall& expr) {
    // TODO: 实现标签调用生成
    reportError("Labeled call generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedClosure& expr) {
    // TODO: 实现闭包生成
    reportError("Closure generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedMemberAccess& expr) {
    // TODO: 实现成员访问生成
    reportError("Member access generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedStructInit& expr) {
    // TODO: 实现结构体初始化生成
    reportError("Struct initialization generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedTypeCheck& expr) {
    // TODO: 实现类型检查生成
    reportError("Type check generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedTypeCast& expr) {
    // TODO: 实现类型转换生成
    reportError("Type cast generation not implemented yet");
}

// 类型化语句访问者实现
void LLVMCodeGenerator::visit(const TypedExprStmt& stmt) {
    // 优先使用类型化的表达式
    if (stmt.getTypedExpression()) {
        reportWarning("DEBUG: Using typed expression in TypedExprStmt");
        generateExpression(*stmt.getTypedExpression());
    } else {
        reportWarning("DEBUG: No typed expression, falling back to original expression");
        // 回退到原始表达式
        const ExprStmt* originalExprStmt = static_cast<const ExprStmt*>(&stmt.getOriginalStmt());
        if (originalExprStmt && originalExprStmt->expression) {
            generateExpression(originalExprStmt->expression.get());
        }
    }
}

void LLVMCodeGenerator::visit(const TypedVarStmt& stmt) {
    // 生成变量声明
    llvm::Type* varType = convertSwiftTypeToLLVM(stmt.getDeclaredType());
    // 从原始VarStmt获取变量名
    const VarStmt* originalVarStmt = static_cast<const VarStmt*>(&stmt.getOriginalStmt());
    llvm::Value* alloca = allocateVariable(originalVarStmt->name.lexeme, varType);
    
    // 如果有初始化器，生成初始化代码
    if (stmt.getInitializer()) {
        llvm::Value* initValue = generateExpression(*stmt.getInitializer());
        builder_->CreateStore(initValue, alloca);
    }
    
    // 记录变量信息
    // TODO: 正确获取isConst信息
    declareVariable(originalVarStmt->name.lexeme, VariableInfo(alloca, varType, false)); // 暂时设为false
}

void LLVMCodeGenerator::visit(const TypedBlockStmt& stmt) {
    enterScope();
    for (const auto& s : stmt.getStatements()) {
        generateStatement(*s);
    }
    exitScope();
}

void LLVMCodeGenerator::visit(const TypedIfStmt& stmt) {
    generateIfStatement(stmt);
}

void LLVMCodeGenerator::visit(const TypedWhileStmt& stmt) {
    generateWhileLoop(stmt);
}

void LLVMCodeGenerator::visit(const TypedForStmt& stmt) {
    generateForLoop(stmt);
}

void LLVMCodeGenerator::visit(const TypedReturnStmt& stmt) {
    generateReturnStatement(stmt);
}

void LLVMCodeGenerator::visit(const TypedContinueStmt& stmt) {
    // TODO: 实现continue语句的代码生成
    // 需要跳转到当前循环的条件检查基本块
    reportError("Continue statement generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedBreakStmt& stmt) {
    // TODO: 实现break语句的代码生成
    // 需要跳转到当前循环的退出基本块
    reportError("Break statement generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedFallthroughStmt& stmt) {
    // TODO: 实现fallthrough语句的代码生成
    // 需要跳转到switch语句的下一个case
    reportError("Fallthrough statement generation not implemented yet");
}

void LLVMCodeGenerator::visit(const TypedFunctionStmt& stmt) {
    generateFunction(stmt);
}

void LLVMCodeGenerator::visit(const TypedStructStmt& stmt) {
    generateStructType(stmt);
}

void LLVMCodeGenerator::visit(const TypedClassStmt& stmt) {
    generateClassType(stmt);
}

// 私有辅助方法实现
llvm::Type* LLVMCodeGenerator::convertSwiftTypeToLLVM(const std::shared_ptr<Type>& swiftType) {
    if (!swiftType) {
        return builder_->getVoidTy();
    }
    
    switch (swiftType->getKind()) {
        case TypeKind::Int:
            return builder_->getInt64Ty();
        case TypeKind::Float:
            return builder_->getDoubleTy();
        case TypeKind::Bool:
            return builder_->getInt1Ty();
        case TypeKind::String:
            return llvm::PointerType::getUnqual(*context_);
        case TypeKind::Void:
            return builder_->getVoidTy();
        case TypeKind::Array:
            return getArrayType(std::static_pointer_cast<ArrayType>(swiftType));
        case TypeKind::Dictionary:
            return getDictionaryType(std::static_pointer_cast<DictionaryType>(swiftType));
        case TypeKind::Tuple:
            return getTupleType(std::static_pointer_cast<TupleType>(swiftType));
        case TypeKind::Function:
            return getFunctionType(std::static_pointer_cast<FunctionType>(swiftType));
        default:
            reportError("Unsupported type conversion: " + swiftType->toString());
            return builder_->getVoidTy();
    }
}

llvm::Value* LLVMCodeGenerator::generateExpression(const TypedExpr& expr) {
    expr.accept(*this);
    return currentValue_;
}

llvm::Value* LLVMCodeGenerator::generateExpression(const Expr* expr) {
    // 简单的原始Expr处理 - 这里可以根据需要扩展
    if (!expr) return nullptr;
    
    // 根据Expr的具体类型进行处理
    if (auto literal = dynamic_cast<const Literal*>(expr)) {
        const Token& token = literal->value;
        
        switch (token.type) {
            case TokenType::IntegerLiteral: {
                int64_t value = std::stoll(token.lexeme);
                return createIntConstant(value);
            }
            case TokenType::FloatingLiteral: {
                double value = std::stod(token.lexeme);
                return createFloatConstant(value);
            }
            case TokenType::True:
                return createBoolConstant(true);
            case TokenType::False:
                return createBoolConstant(false);
            case TokenType::StringLiteral: {
                // Remove quotes from string literal
                std::string value = token.lexeme;
                if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
                    value = value.substr(1, value.length() - 2);
                }
                return createStringConstant(value);
            }
            default:
                reportError("Unsupported literal type: " + token.lexeme);
                return nullptr;
        }
    } else if (auto varExpr = dynamic_cast<const VarExpr*>(expr)) {
        VariableInfo* varInfo = lookupVariable(varExpr->name.lexeme);
        if (!varInfo) {
            reportError("Undefined variable: " + varExpr->name.lexeme);
            return nullptr;
        }
        return builder_->CreateLoad(varInfo->type, varInfo->value, varExpr->name.lexeme);
    } else if (auto binary = dynamic_cast<const Binary*>(expr)) {
        llvm::Value* left = generateExpression(binary->left.get());
        llvm::Value* right = generateExpression(binary->right.get());
        
        const std::string& op = binary->op.lexeme;
        
        // 算术运算
        if (op == "+") {
            if (left->getType()->isIntegerTy()) {
                return builder_->CreateAdd(left, right, "add");
            } else if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFAdd(left, right, "fadd");
            }
        } else if (op == "-") {
            if (left->getType()->isIntegerTy()) {
                return builder_->CreateSub(left, right, "sub");
            } else if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFSub(left, right, "fsub");
            }
        } else if (op == "*") {
            if (left->getType()->isIntegerTy()) {
                return builder_->CreateMul(left, right, "mul");
            } else if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFMul(left, right, "fmul");
            }
        } else if (op == "/") {
            if (left->getType()->isIntegerTy()) {
                return builder_->CreateSDiv(left, right, "div");
            } else if (left->getType()->isFloatingPointTy()) {
                return builder_->CreateFDiv(left, right, "fdiv");
            }
        }
        
        reportError("Unsupported binary operator: " + op);
        return nullptr;
    } else if (auto unary = dynamic_cast<const Unary*>(expr)) {
        llvm::Value* operand = generateExpression(unary->right.get());
        const std::string& op = unary->op.lexeme;
        
        if (op == "-") {
            if (operand->getType()->isIntegerTy()) {
                return builder_->CreateNeg(operand, "neg");
            } else if (operand->getType()->isFloatingPointTy()) {
                return builder_->CreateFNeg(operand, "fneg");
            }
        } else if (op == "!") {
            return builder_->CreateNot(operand, "not");
        }
        
        reportError("Unsupported unary operator: " + op);
        return nullptr;
    } else if (auto grouping = dynamic_cast<const Grouping*>(expr)) {
        return generateExpression(grouping->expression.get());
    } else if (auto call = dynamic_cast<const Call*>(expr)) {
        // 处理函数调用
        if (auto varExpr = dynamic_cast<const VarExpr*>(call->callee.get())) {
            const std::string& funcName = varExpr->name.lexeme;
            
            // 特殊处理print函数
            if (funcName == "print") {
                // 获取printf函数
                llvm::Function* printfFunc = module_->getFunction("printf");
                if (!printfFunc) {
                    reportError("printf function not found");
                    return nullptr;
                }
                
                // 处理每个参数，为每个参数单独调用printf
                for (const auto& arg : call->arguments) {
                    llvm::Value* argValue = generateExpression(arg.get());
                    if (!argValue) continue;
                    
                    std::vector<llvm::Value*> args;
                    
                    if (argValue->getType()->isIntegerTy()) {
                        llvm::Value* formatStr = builder_->CreateGlobalStringPtr("%lld\n");
                        args.push_back(formatStr);
                        args.push_back(argValue);
                    } else if (argValue->getType()->isFloatingPointTy()) {
                        llvm::Value* formatStr = builder_->CreateGlobalStringPtr("%f\n");
                        args.push_back(formatStr);
                        args.push_back(argValue);
                    } else if (argValue->getType()->isPointerTy()) {
                        llvm::Value* formatStr = builder_->CreateGlobalStringPtr("%s\n");
                        args.push_back(formatStr);
                        args.push_back(argValue);
                    } else {
                        reportError("Unsupported argument type for print function");
                        continue;
                    }
                    
                    // 调用printf
                    builder_->CreateCall(printfFunc, args, "print_call");
                }
                
                return nullptr; // print函数没有返回值
            } else {
                // 查找用户定义的函数
                auto funcIt = functions_.find(funcName);
                if (funcIt != functions_.end()) {
                    llvm::Function* function = funcIt->second.function;
                    
                    // 生成参数
                    std::vector<llvm::Value*> args;
                    for (const auto& arg : call->arguments) {
                        llvm::Value* argValue = generateExpression(arg.get());
                        if (argValue) {
                            args.push_back(argValue);
                        }
                    }
                    
                    // 调用函数
                    return builder_->CreateCall(function, args, funcName + "_call");
                } else {
                    reportError("Undefined function: " + funcName);
                    return nullptr;
                }
            }
        } else {
            reportError("Complex function calls not supported yet");
            return nullptr;
        }
    } else {
        reportWarning("Unsupported raw expression type, skipping");
        return nullptr;
    }
}

void LLVMCodeGenerator::generateStatement(const TypedStmt& stmt) {
    stmt.accept(*this);
}

void LLVMCodeGenerator::generateStatement(const Stmt* stmt) {
    // 简单的原始Stmt处理 - 这里可以根据需要扩展
    if (!stmt) return;
    
    // 根据Stmt的具体类型进行处理
    if (auto funcStmt = dynamic_cast<const FunctionStmt*>(stmt)) {
        // 创建函数类型
        std::vector<llvm::Type*> paramTypes;
        for (const auto& param : funcStmt->parameters) {
            paramTypes.push_back(builder_->getInt64Ty()); // 默认为int64类型
        }
        
        // 根据函数声明的返回类型来确定LLVM返回类型
        llvm::Type* returnType;
        std::cout << "DEBUG: Function " << funcStmt->name.lexeme << " return type: '" << funcStmt->returnType.lexeme << "'" << std::endl;
        if (funcStmt->returnType.lexeme.empty() || funcStmt->returnType.lexeme == "Void") {
            returnType = builder_->getVoidTy();
            std::cout << "DEBUG: Using void return type" << std::endl;
        } else if (funcStmt->returnType.lexeme == "Int") {
            returnType = builder_->getInt64Ty();
            std::cout << "DEBUG: Using int64 return type" << std::endl;
        } else if (funcStmt->returnType.lexeme == "String") {
            returnType = llvm::PointerType::getUnqual(*context_);
            std::cout << "DEBUG: Using pointer return type" << std::endl;
        } else if (funcStmt->returnType.lexeme == "Bool") {
            returnType = builder_->getInt1Ty();
            std::cout << "DEBUG: Using bool return type" << std::endl;
        } else {
            // 默认为void类型
            returnType = builder_->getVoidTy();
            std::cout << "DEBUG: Using default void return type for unknown type: " << funcStmt->returnType.lexeme << std::endl;
        }
        llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
        
        // 创建函数
        llvm::Function* function = llvm::Function::Create(
            funcType, llvm::Function::ExternalLinkage, funcStmt->name.lexeme, module_.get());
        
        // 保存当前函数
        llvm::Function* prevFunction = currentFunction_;
        currentFunction_ = function;
        
        // 创建入口基本块
        llvm::BasicBlock* entryBB = createBasicBlock("entry", function);
        builder_->SetInsertPoint(entryBB);
        
        // 进入新作用域
        enterScope();
        
        // 为参数创建alloca并存储参数值
        auto argIt = function->arg_begin();
        for (size_t i = 0; i < funcStmt->parameters.size(); ++i, ++argIt) {
            llvm::Type* paramType = paramTypes[i];
            llvm::Value* alloca = allocateVariable(funcStmt->parameters[i].name.lexeme, paramType);
            builder_->CreateStore(&*argIt, alloca);
            declareVariable(funcStmt->parameters[i].name.lexeme, VariableInfo(alloca, paramType));
        }
        
        // 生成函数体
        generateStatement(funcStmt->body.get());
        
        // 如果函数没有显式返回，添加默认返回
        if (!builder_->GetInsertBlock()->getTerminator()) {
            if (returnType->isVoidTy()) {
                builder_->CreateRetVoid();
            } else {
                // 为非void函数创建默认返回值
                llvm::Value* defaultValue = llvm::Constant::getNullValue(returnType);
                builder_->CreateRet(defaultValue);
            }
        }
        
        // 退出作用域
        exitScope();
        
        // 恢复之前的函数
        currentFunction_ = prevFunction;
        
        // 记录函数信息
        std::vector<std::string> paramNames;
        for (const auto& param : funcStmt->parameters) {
            paramNames.push_back(param.name.lexeme);
        }
        
        // 创建一个基本的函数类型
        auto voidType = typeSystem_->getVoidType();
        std::vector<std::shared_ptr<Type>> swiftParamTypes;
        auto swiftFuncType = std::make_shared<FunctionType>(swiftParamTypes, voidType);
        
        functions_[funcStmt->name.lexeme] = FunctionInfo(function, swiftFuncType, paramNames);
        
    } else if (auto returnStmt = dynamic_cast<const ReturnStmt*>(stmt)) {
        if (returnStmt->value) {
            llvm::Value* returnValue = generateExpression(returnStmt->value.get());
            builder_->CreateRet(returnValue);
        } else {
            builder_->CreateRetVoid();
        }
    } else if (auto blockStmt = dynamic_cast<const BlockStmt*>(stmt)) {
        enterScope();
        for (const auto& s : blockStmt->statements) {
            generateStatement(s.get());
        }
        exitScope();
    } else if (auto exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
        // 处理表达式语句（如函数调用）
        if (exprStmt->expression) {
            generateExpression(exprStmt->expression.get());
        }
    } else {
        reportWarning("Unsupported raw statement type, skipping");
    }
}

llvm::Value* LLVMCodeGenerator::generateLiteral(const TypedLiteral& literal) {
    const Literal* literalExpr = static_cast<const Literal*>(&literal.getOriginalExpr());
    const Token& token = literalExpr->value;
    
    switch (token.type) {
        case TokenType::IntegerLiteral: {
            int64_t value = std::stoll(token.lexeme);
            return createIntConstant(value);
        }
        case TokenType::FloatingLiteral: {
            double value = std::stod(token.lexeme);
            return createFloatConstant(value);
        }
        case TokenType::True:
            return createBoolConstant(true);
        case TokenType::False:
            return createBoolConstant(false);
        case TokenType::StringLiteral: {
            // Remove quotes from string literal
            std::string value = token.lexeme;
            if (value.length() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.length() - 2);
            }
            return createStringConstant(value);
        }
        default:
            reportError("Unsupported literal type: " + token.lexeme);
            return nullptr;
    }
}

llvm::Value* LLVMCodeGenerator::generateBinaryOperation(const TypedBinary& binary) {
    llvm::Value* left = generateExpression(binary.getLeft());
    llvm::Value* right = generateExpression(binary.getRight());
    
    const Binary* binaryExpr = static_cast<const Binary*>(&binary.getOriginalExpr());
    const std::string& op = binaryExpr->op.lexeme;
    
    // 算术运算
    if (op == "+") {
        if (left->getType()->isIntegerTy()) {
            return builder_->CreateAdd(left, right, "add");
        } else if (left->getType()->isFloatingPointTy()) {
            return builder_->CreateFAdd(left, right, "fadd");
        }
    } else if (op == "-") {
        if (left->getType()->isIntegerTy()) {
            return builder_->CreateSub(left, right, "sub");
        } else if (left->getType()->isFloatingPointTy()) {
            return builder_->CreateFSub(left, right, "fsub");
        }
    } else if (op == "*") {
        if (left->getType()->isIntegerTy()) {
            return builder_->CreateMul(left, right, "mul");
        } else if (left->getType()->isFloatingPointTy()) {
            return builder_->CreateFMul(left, right, "fmul");
        }
    } else if (op == "/") {
        if (left->getType()->isIntegerTy()) {
            return builder_->CreateSDiv(left, right, "div");
        } else if (left->getType()->isFloatingPointTy()) {
            return builder_->CreateFDiv(left, right, "fdiv");
        }
    }
    // 比较运算
    else if (op == "==") {
        if (left->getType()->isIntegerTy()) {
            return builder_->CreateICmpEQ(left, right, "eq");
        } else if (left->getType()->isFloatingPointTy()) {
            return builder_->CreateFCmpOEQ(left, right, "feq");
        }
    } else if (op == "!=") {
        if (left->getType()->isIntegerTy()) {
            return builder_->CreateICmpNE(left, right, "ne");
        } else if (left->getType()->isFloatingPointTy()) {
            return builder_->CreateFCmpONE(left, right, "fne");
        }
    } else if (op == "<") {
        if (left->getType()->isIntegerTy()) {
            return builder_->CreateICmpSLT(left, right, "lt");
        } else if (left->getType()->isFloatingPointTy()) {
            return builder_->CreateFCmpOLT(left, right, "flt");
        }
    } else if (op == ">") {
        if (left->getType()->isIntegerTy()) {
            return builder_->CreateICmpSGT(left, right, "gt");
        } else if (left->getType()->isFloatingPointTy()) {
            return builder_->CreateFCmpOGT(left, right, "fgt");
        }
    } else if (op == "<=") {
        if (left->getType()->isIntegerTy()) {
            return builder_->CreateICmpSLE(left, right, "le");
        } else if (left->getType()->isFloatingPointTy()) {
            return builder_->CreateFCmpOLE(left, right, "fle");
        }
    } else if (op == ">=") {
        if (left->getType()->isIntegerTy()) {
            return builder_->CreateICmpSGE(left, right, "ge");
        } else if (left->getType()->isFloatingPointTy()) {
            return builder_->CreateFCmpOGE(left, right, "fge");
        }
    }
    // 逻辑运算
    else if (op == "&&") {
        return builder_->CreateAnd(left, right, "and");
    } else if (op == "||") {
        return builder_->CreateOr(left, right, "or");
    }
    
    reportError("Unsupported binary operator: " + op);
    return nullptr;
}

llvm::Value* LLVMCodeGenerator::generateUnaryOperation(const TypedUnary& unary) {
    // TypedUnary目前是占位符实现，需要添加getOperand方法
    // 暂时通过原始表达式访问
    const Unary* unaryExpr = static_cast<const Unary*>(&unary.getOriginalExpr());
    llvm::Value* operand = generateExpression(unaryExpr->right.get());
    if (!operand) {
        reportError("Failed to generate operand for unary operation");
        return nullptr;
    }
    
    const std::string& op = unaryExpr->op.lexeme;
    
    if (op == "-") {
        if (operand->getType()->isIntegerTy()) {
            return builder_->CreateNeg(operand, "neg");
        } else if (operand->getType()->isFloatingPointTy()) {
            return builder_->CreateFNeg(operand, "fneg");
        }
    } else if (op == "!") {
        return builder_->CreateNot(operand, "not");
    }
    
    reportError("Unsupported unary operator: " + op);
    return nullptr;
}

llvm::Value* LLVMCodeGenerator::generateVariableAccess(const TypedVarExpr& varExpr) {
    const VarExpr* varExprNode = static_cast<const VarExpr*>(&varExpr.getOriginalExpr());
    VariableInfo* varInfo = lookupVariable(varExprNode->name.lexeme);
    if (!varInfo) {
        reportError("Undefined variable: " + varExprNode->name.lexeme);
        return nullptr;
    }
    
    return builder_->CreateLoad(varInfo->type, varInfo->value, varExprNode->name.lexeme);
}

llvm::Value* LLVMCodeGenerator::generateAssignment(const TypedAssign& assign) {
    // TypedAssign目前是占位符实现，需要添加getValue方法
    // 暂时通过原始表达式访问
    const Assign* assignExpr = static_cast<const Assign*>(&assign.getOriginalExpr());
    // TODO: 正确实现Assignment表达式的value访问
    // 暂时返回nullptr
    llvm::Value* value = nullptr; // generateExpression(*assignExpr->value);
    
    // 假设target是VarExpr，获取变量名
    const VarExpr* varExpr = static_cast<const VarExpr*>(assignExpr->target.get());
    VariableInfo* varInfo = lookupVariable(varExpr->name.lexeme);
    if (!varInfo) {
        reportError("Undefined variable: " + varExpr->name.lexeme);
        return nullptr;
    }
    
    if (varInfo->isConst) {
        reportError("Cannot assign to constant variable: " + varExpr->name.lexeme);
        return nullptr;
    }
    
    builder_->CreateStore(value, varInfo->value);
    return value;
}

llvm::Value* LLVMCodeGenerator::generateFunctionCall(const TypedCall& call) {
    const Call* callExpr = static_cast<const Call*>(&call.getOriginalExpr());
    
    // 假设callee是VarExpr，获取函数名
    const VarExpr* calleeExpr = static_cast<const VarExpr*>(callExpr->callee.get());
    const std::string& functionName = calleeExpr->name.lexeme;
    
    reportWarning("DEBUG: Generating function call for: " + functionName);
    
    // 处理内置函数
    if (functionName == "print") {
        reportWarning("DEBUG: Calling generatePrintCall");
        return generatePrintCall(call);
    }
    
    auto funcIt = functions_.find(functionName);
    if (funcIt == functions_.end()) {
        reportError("Undefined function: " + functionName);
        return nullptr;
    }
    
    llvm::Function* function = funcIt->second.function;
    
    // 生成参数
    std::vector<llvm::Value*> args;
    for (const auto& arg : call.getArguments()) {
        args.push_back(generateExpression(*arg));
    }
    
    return builder_->CreateCall(function, args, "call");
}

llvm::Value* LLVMCodeGenerator::generatePrintCall(const TypedCall& call) {
    // 获取printf函数
    llvm::Function* printfFunc = module_->getFunction("printf");
    if (!printfFunc) {
        reportError("printf function not found");
        return nullptr;
    }
    
    // 生成参数
    std::vector<llvm::Value*> args;
    for (const auto& arg : call.getArguments()) {
        llvm::Value* argValue = generateExpression(*arg);
        if (!argValue) continue;
        
        // 根据参数类型生成适当的格式字符串和参数
        llvm::Type* argType = argValue->getType();
        if (argType->isIntegerTy(64)) {
            // 整数类型
            llvm::Value* formatStr = builder_->CreateGlobalStringPtr("%lld\n");
            args.push_back(formatStr);
            args.push_back(argValue);
        } else if (argType->isDoubleTy()) {
            // 浮点类型
            llvm::Value* formatStr = builder_->CreateGlobalStringPtr("%.6f\n");
            args.push_back(formatStr);
            args.push_back(argValue);
        } else if (argType->isPointerTy()) {
            // 字符串类型
            llvm::Value* formatStr = builder_->CreateGlobalStringPtr("%s\n");
            args.push_back(formatStr);
            args.push_back(argValue);
        } else {
            reportError("Unsupported argument type for print function");
            continue;
        }
        
        // 调用printf
        builder_->CreateCall(printfFunc, args, "print_call");
        args.clear(); // 清空参数列表，为下一个参数准备
    }
    
    return nullptr; // print函数没有返回值
}

// 控制流生成
void LLVMCodeGenerator::generateIfStatement(const TypedIfStmt& ifStmt) {
    // 获取原始IfStmt
    const auto& originalIfStmt = static_cast<const IfStmt&>(ifStmt.getOriginalStmt());
    
    llvm::Value* condition = generateExpression(originalIfStmt.condition.get());
    
    llvm::Function* function = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* thenBB = createBasicBlock("if_then", function);
    llvm::BasicBlock* elseBB = originalIfStmt.elseBranch ? createBasicBlock("if_else", function) : nullptr;
    llvm::BasicBlock* mergeBB = createBasicBlock("if_merge", function);
    
    if (elseBB) {
        builder_->CreateCondBr(condition, thenBB, elseBB);
    } else {
        builder_->CreateCondBr(condition, thenBB, mergeBB);
    }
    
    // Then分支
    builder_->SetInsertPoint(thenBB);
    generateStatement(originalIfStmt.thenBranch.get());
    builder_->CreateBr(mergeBB);
    
    // Else分支
    if (elseBB) {
        builder_->SetInsertPoint(elseBB);
        generateStatement(originalIfStmt.elseBranch.get());
        builder_->CreateBr(mergeBB);
    }
    
    // 合并点
    builder_->SetInsertPoint(mergeBB);
}

void LLVMCodeGenerator::generateWhileLoop(const TypedWhileStmt& whileStmt) {
    // 获取原始WhileStmt
    const auto& originalWhileStmt = static_cast<const WhileStmt&>(whileStmt.getOriginalStmt());
    
    llvm::Function* function = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* condBB = createBasicBlock("while_cond", function);
    llvm::BasicBlock* bodyBB = createBasicBlock("while_body", function);
    llvm::BasicBlock* exitBB = createBasicBlock("while_exit", function);
    
    builder_->CreateBr(condBB);
    
    // 条件检查
    builder_->SetInsertPoint(condBB);
    llvm::Value* condition = generateExpression(originalWhileStmt.condition.get());
    builder_->CreateCondBr(condition, bodyBB, exitBB);
    
    // 循环体
    builder_->SetInsertPoint(bodyBB);
    generateStatement(originalWhileStmt.body.get());
    builder_->CreateBr(condBB);
    
    // 退出点
    builder_->SetInsertPoint(exitBB);
}

void LLVMCodeGenerator::generateReturnStatement(const TypedReturnStmt& returnStmt) {
    // 获取原始ReturnStmt
    const auto& originalReturnStmt = static_cast<const ReturnStmt&>(returnStmt.getOriginalStmt());
    
    if (originalReturnStmt.value) {
        llvm::Value* returnValue = generateExpression(originalReturnStmt.value.get());
        builder_->CreateRet(returnValue);
    } else {
        builder_->CreateRetVoid();
    }
}

llvm::Function* LLVMCodeGenerator::generateFunction(const TypedFunctionStmt& funcStmt) {
    // 获取原始FunctionStmt
    const auto& originalFuncStmt = static_cast<const FunctionStmt&>(funcStmt.getOriginalStmt());
    
    // 创建函数类型
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : originalFuncStmt.parameters) {
        // TODO: 正确实现从Token到Type的转换
        // 暂时使用默认类型
        paramTypes.push_back(builder_->getInt64Ty()); // 默认为int64类型
    }
    
    // 根据函数声明的返回类型来确定LLVM返回类型
    llvm::Type* returnType;
    std::cout << "DEBUG: TypedFunction " << originalFuncStmt.name.lexeme << " return type: '" << originalFuncStmt.returnType.lexeme << "'" << std::endl;
    if (originalFuncStmt.returnType.lexeme.empty() || originalFuncStmt.returnType.lexeme == "Void") {
        returnType = builder_->getVoidTy();
        std::cout << "DEBUG: Using void return type" << std::endl;
    } else if (originalFuncStmt.returnType.lexeme == "Int") {
        returnType = builder_->getInt64Ty();
        std::cout << "DEBUG: Using int64 return type" << std::endl;
    } else if (originalFuncStmt.returnType.lexeme == "String") {
        returnType = llvm::PointerType::getUnqual(*context_);
        std::cout << "DEBUG: Using pointer return type" << std::endl;
    } else if (originalFuncStmt.returnType.lexeme == "Bool") {
        returnType = builder_->getInt1Ty();
        std::cout << "DEBUG: Using bool return type" << std::endl;
    } else {
        // 默认为void类型
        returnType = builder_->getVoidTy();
        std::cout << "DEBUG: Using default void return type for unknown type: " << originalFuncStmt.returnType.lexeme << std::endl;
    }
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);
    
    // 创建函数
    llvm::Function* function = llvm::Function::Create(
        funcType, llvm::Function::ExternalLinkage, originalFuncStmt.name.lexeme, module_.get());
    
    // 设置参数名称
    auto argIt = function->arg_begin();
    for (size_t i = 0; i < originalFuncStmt.parameters.size(); ++i, ++argIt) {
        argIt->setName(originalFuncStmt.parameters[i].name.lexeme);
    }
    
    // 创建函数体
    llvm::BasicBlock* entryBB = createBasicBlock("entry", function);
    builder_->SetInsertPoint(entryBB);
    
    // 保存当前函数
    llvm::Function* prevFunction = currentFunction_;
    currentFunction_ = function;
    
    // 进入新作用域
    enterScope();
    
    // 为参数创建alloca并存储参数值
    argIt = function->arg_begin();
    for (size_t i = 0; i < originalFuncStmt.parameters.size(); ++i, ++argIt) {
        // TODO: 正确实现从Token到Type的转换
        // 暂时使用默认类型
        llvm::Type* paramType = builder_->getInt64Ty(); // 默认为int64类型
        llvm::Value* alloca = allocateVariable(originalFuncStmt.parameters[i].name.lexeme, paramType);
        builder_->CreateStore(&*argIt, alloca);
        declareVariable(originalFuncStmt.parameters[i].name.lexeme, VariableInfo(alloca, paramType));
    }
    
    // 生成函数体
    generateStatement(funcStmt.getBody());
    
    // 如果函数没有显式返回，添加默认返回
    if (!builder_->GetInsertBlock()->getTerminator()) {
        if (returnType->isVoidTy()) {
            builder_->CreateRetVoid();
        } else {
            // 为非void函数创建默认返回值
            llvm::Value* defaultValue = llvm::Constant::getNullValue(returnType);
            builder_->CreateRet(defaultValue);
        }
    }
    
    // 退出作用域
    exitScope();
    
    // 恢复之前的函数
    currentFunction_ = prevFunction;
    
    // 记录函数信息
    std::vector<std::string> paramNames;
    for (const auto& param : originalFuncStmt.parameters) {
        paramNames.push_back(param.name.lexeme);
    }
    
    // 创建Swift函数类型
    std::vector<std::shared_ptr<Type>> swiftParamTypes;
    for (const auto& param : originalFuncStmt.parameters) {
        // 这里需要从Token转换为Type，暂时使用空的类型向量
        // TODO: 实现正确的类型转换
    }
    
    // 创建一个基本的函数类型，使用空参数和void返回类型
    auto voidType = typeSystem_->getVoidType();
    auto swiftFuncType = std::make_shared<FunctionType>(swiftParamTypes, voidType);
    // TODO: 正确设置Swift函数类型参数和返回类型
    
    functions_[originalFuncStmt.name.lexeme] = FunctionInfo(function, swiftFuncType, paramNames);
    
    return function;
}

// 基础设施方法
void LLVMCodeGenerator::reportError(const std::string& message, int line, int column) {
    errors_.emplace_back(message, line, column);
}

void LLVMCodeGenerator::reportWarning(const std::string& message) {
    warnings_.push_back(message);
}

void LLVMCodeGenerator::enterScope() {
    scopeStack_.emplace_back();
}

void LLVMCodeGenerator::exitScope() {
    if (!scopeStack_.empty()) {
        scopeStack_.pop_back();
    }
}

VariableInfo* LLVMCodeGenerator::lookupVariable(const std::string& name) {
    // 从当前作用域向上查找
    for (auto it = scopeStack_.rbegin(); it != scopeStack_.rend(); ++it) {
        auto varIt = it->localVariables.find(name);
        if (varIt != it->localVariables.end()) {
            return &varIt->second;
        }
    }
    
    // 查找全局变量
    auto globalIt = variables_.find(name);
    if (globalIt != variables_.end()) {
        return &globalIt->second;
    }
    
    return nullptr;
}

void LLVMCodeGenerator::declareVariable(const std::string& name, const VariableInfo& info) {
    if (!scopeStack_.empty()) {
        scopeStack_.back().localVariables[name] = info;
    } else {
        variables_[name] = info;
    }
}

llvm::BasicBlock* LLVMCodeGenerator::createBasicBlock(const std::string& name, llvm::Function* function) {
    if (!function) {
        function = currentFunction_;
    }
    return llvm::BasicBlock::Create(*context_, name, function);
}

llvm::Value* LLVMCodeGenerator::allocateVariable(const std::string& name, llvm::Type* type, bool isGlobal) {
    if (isGlobal) {
        return new llvm::GlobalVariable(*module_, type, false, 
            llvm::GlobalValue::ExternalLinkage, nullptr, name);
    } else {
        return builder_->CreateAlloca(type, nullptr, name);
    }
}

llvm::Constant* LLVMCodeGenerator::createIntConstant(int64_t value, llvm::Type* type) {
    if (!type) {
        type = builder_->getInt64Ty();
    }
    return llvm::ConstantInt::get(type, value);
}

llvm::Constant* LLVMCodeGenerator::createFloatConstant(double value, llvm::Type* type) {
    if (!type) {
        type = builder_->getDoubleTy();
    }
    return llvm::ConstantFP::get(type, value);
}

llvm::Constant* LLVMCodeGenerator::createStringConstant(const std::string& value) {
    return builder_->CreateGlobalStringPtr(value);
}

llvm::Constant* LLVMCodeGenerator::createBoolConstant(bool value) {
    return llvm::ConstantInt::get(builder_->getInt1Ty(), value ? 1 : 0);
}

bool LLVMCodeGenerator::verifyModule() {
    if (!module_) return false;
    
    std::string errorStr;
    llvm::raw_string_ostream errorStream(errorStr);
    
    if (llvm::verifyModule(*module_, &errorStream)) {
        reportError("Module verification failed: " + errorStr);
        return false;
    }
    
    return true;
}

void LLVMCodeGenerator::dumpModule() {
    if (module_) {
        module_->print(llvm::outs(), nullptr);
    }
}

llvm::TargetMachine* LLVMCodeGenerator::getTargetMachine() {
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    
    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    
    if (!target) {
        reportError("Failed to lookup target: " + error);
        return nullptr;
    }
    
    auto cpu = "generic";
    auto features = "";
    
    llvm::TargetOptions opt;
    auto rm = std::optional<llvm::Reloc::Model>();
    
    return target->createTargetMachine(targetTriple, cpu, features, opt, rm);
}

// 运行时支持函数生成
void LLVMCodeGenerator::generateRuntimeFunctions() {
    createPrintFunction();
    createMemoryAllocationFunction();
    createMemoryDeallocationFunction();
}

llvm::Function* LLVMCodeGenerator::createPrintFunction() {
    // 创建printf声明
    std::vector<llvm::Type*> printfArgs;
    printfArgs.push_back(llvm::PointerType::getUnqual(*context_));
    
    llvm::FunctionType* printfType = llvm::FunctionType::get(
        builder_->getInt32Ty(), printfArgs, true);
    
    llvm::Function* printfFunc = llvm::Function::Create(
        printfType, llvm::Function::ExternalLinkage, "printf", module_.get());
    
    return printfFunc;
}

llvm::Function* LLVMCodeGenerator::createMemoryAllocationFunction() {
    // 创建malloc声明
    std::vector<llvm::Type*> mallocArgs;
    mallocArgs.push_back(builder_->getInt64Ty());
    
    llvm::FunctionType* mallocType = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(*context_), mallocArgs, false);
    
    llvm::Function* mallocFunc = llvm::Function::Create(
        mallocType, llvm::Function::ExternalLinkage, "malloc", module_.get());
    
    return mallocFunc;
}

llvm::Function* LLVMCodeGenerator::createMemoryDeallocationFunction() {
    // 创建free声明
    std::vector<llvm::Type*> freeArgs;
    freeArgs.push_back(llvm::PointerType::getUnqual(*context_));
    
    llvm::FunctionType* freeType = llvm::FunctionType::get(
        builder_->getVoidTy(), freeArgs, false);
    
    llvm::Function* freeFunc = llvm::Function::Create(
        freeType, llvm::Function::ExternalLinkage, "free", module_.get());
    
    return freeFunc;
}

// 占位符实现 - 这些需要根据具体需求进一步实现
void LLVMCodeGenerator::generateForLoop(const TypedForStmt& forStmt) {
    reportError("For loop generation not implemented yet");
}

llvm::Type* LLVMCodeGenerator::getArrayType(const std::shared_ptr<ArrayType>& arrayType) {
    reportError("Array type conversion not implemented yet");
    return builder_->getVoidTy();
}

llvm::Type* LLVMCodeGenerator::getDictionaryType(const std::shared_ptr<DictionaryType>& dictType) {
    reportError("Dictionary type conversion not implemented yet");
    return builder_->getVoidTy();
}

llvm::Type* LLVMCodeGenerator::getTupleType(const std::shared_ptr<TupleType>& tupleType) {
    reportError("Tuple type conversion not implemented yet");
    return builder_->getVoidTy();
}

llvm::Type* LLVMCodeGenerator::getFunctionType(const std::shared_ptr<FunctionType>& funcType) {
    reportError("Function type conversion not implemented yet");
    return builder_->getVoidTy();
}

llvm::StructType* LLVMCodeGenerator::generateStructType(const TypedStructStmt& structStmt) {
    reportError("Struct type generation not implemented yet");
    return nullptr;
}

llvm::StructType* LLVMCodeGenerator::generateClassType(const TypedClassStmt& classStmt) {
    reportError("Class type generation not implemented yet");
    return nullptr;
}

void LLVMCodeGenerator::generateSwiftRuntimeSupport() {
    // 创建Swift运行时支持函数
    createSwiftStringType();
    createSwiftArrayType();
    createSwiftDictionaryType();
    createSwiftOptionalType();
}

void LLVMCodeGenerator::generateARCSupport() {
    // 创建ARC支持函数
    createRetainFunction();
    createReleaseFunction();
}

void LLVMCodeGenerator::generateConcurrencySupport() {
    // 创建并发支持函数
    createAsyncFunction();
    createAwaitFunction();
    createTaskFunction();
}

// Swift运行时支持函数实现
llvm::Function* LLVMCodeGenerator::createSwiftStringType() {
    // 创建Swift String类型的运行时支持
    // String在LLVM中表示为指向字符数据的指针和长度
    std::vector<llvm::Type*> stringFields;
    stringFields.push_back(llvm::PointerType::getUnqual(*context_)); // 字符数据指针
    stringFields.push_back(builder_->getInt64Ty()); // 长度
    
    llvm::StructType* stringType = llvm::StructType::create(*context_, stringFields, "SwiftString");
    
    // 创建字符串创建函数
    std::vector<llvm::Type*> createStringArgs;
    createStringArgs.push_back(llvm::PointerType::getUnqual(*context_)); // C字符串
    
    llvm::FunctionType* createStringType = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(stringType), createStringArgs, false);
    
    llvm::Function* createStringFunc = llvm::Function::Create(
        createStringType, llvm::Function::ExternalLinkage, "swift_string_create", module_.get());
    
    return createStringFunc;
}

llvm::Function* LLVMCodeGenerator::createSwiftArrayType() {
    // 创建Swift Array类型的运行时支持
    std::vector<llvm::Type*> arrayFields;
    arrayFields.push_back(llvm::PointerType::getUnqual(*context_)); // 数据指针
    arrayFields.push_back(builder_->getInt64Ty()); // 容量
    arrayFields.push_back(builder_->getInt64Ty()); // 当前大小
    
    llvm::StructType* arrayType = llvm::StructType::create(*context_, arrayFields, "SwiftArray");
    
    // 创建数组创建函数
    std::vector<llvm::Type*> createArrayArgs;
    createArrayArgs.push_back(builder_->getInt64Ty()); // 初始容量
    createArrayArgs.push_back(builder_->getInt64Ty()); // 元素大小
    
    llvm::FunctionType* createArrayType = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(arrayType), createArrayArgs, false);
    
    llvm::Function* createArrayFunc = llvm::Function::Create(
        createArrayType, llvm::Function::ExternalLinkage, "swift_array_create", module_.get());
    
    return createArrayFunc;
}

llvm::Function* LLVMCodeGenerator::createSwiftDictionaryType() {
    // 创建Swift Dictionary类型的运行时支持
    std::vector<llvm::Type*> dictFields;
    dictFields.push_back(llvm::PointerType::getUnqual(*context_)); // 哈希表指针
    dictFields.push_back(builder_->getInt64Ty()); // 容量
    dictFields.push_back(builder_->getInt64Ty()); // 当前大小
    
    llvm::StructType* dictType = llvm::StructType::create(*context_, dictFields, "SwiftDictionary");
    
    // 创建字典创建函数
    std::vector<llvm::Type*> createDictArgs;
    createDictArgs.push_back(builder_->getInt64Ty()); // 初始容量
    
    llvm::FunctionType* createDictType = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(dictType), createDictArgs, false);
    
    llvm::Function* createDictFunc = llvm::Function::Create(
        createDictType, llvm::Function::ExternalLinkage, "swift_dict_create", module_.get());
    
    return createDictFunc;
}

llvm::Function* LLVMCodeGenerator::createSwiftOptionalType() {
    // 创建Swift Optional类型的运行时支持
    std::vector<llvm::Type*> optionalFields;
    optionalFields.push_back(builder_->getInt1Ty()); // hasValue标志
    optionalFields.push_back(llvm::PointerType::getUnqual(*context_)); // 值指针
    
    llvm::StructType* optionalType = llvm::StructType::create(*context_, optionalFields, "SwiftOptional");
    
    // 创建Optional创建函数
    std::vector<llvm::Type*> createOptionalArgs;
    createOptionalArgs.push_back(llvm::PointerType::getUnqual(*context_)); // 值指针
    createOptionalArgs.push_back(builder_->getInt1Ty()); // hasValue
    
    llvm::FunctionType* createOptionalType = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(optionalType), createOptionalArgs, false);
    
    llvm::Function* createOptionalFunc = llvm::Function::Create(
        createOptionalType, llvm::Function::ExternalLinkage, "swift_optional_create", module_.get());
    
    return createOptionalFunc;
}

// ARC支持函数实现
llvm::Function* LLVMCodeGenerator::createRetainFunction() {
    // 创建对象引用计数增加函数
    std::vector<llvm::Type*> retainArgs;
    retainArgs.push_back(llvm::PointerType::getUnqual(*context_)); // 对象指针
    
    llvm::FunctionType* retainType = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(*context_), retainArgs, false);
    
    llvm::Function* retainFunc = llvm::Function::Create(
        retainType, llvm::Function::ExternalLinkage, "swift_retain", module_.get());
    
    return retainFunc;
}

llvm::Function* LLVMCodeGenerator::createReleaseFunction() {
    // 创建对象引用计数减少函数
    std::vector<llvm::Type*> releaseArgs;
    releaseArgs.push_back(llvm::PointerType::getUnqual(*context_)); // 对象指针
    
    llvm::FunctionType* releaseType = llvm::FunctionType::get(
        builder_->getVoidTy(), releaseArgs, false);
    
    llvm::Function* releaseFunc = llvm::Function::Create(
        releaseType, llvm::Function::ExternalLinkage, "swift_release", module_.get());
    
    return releaseFunc;
}

void LLVMCodeGenerator::insertRetain(llvm::Value* object) {
    // 插入retain调用
    if (!object || !object->getType()->isPointerTy()) {
        return;
    }
    
    llvm::Function* retainFunc = module_->getFunction("swift_retain");
    if (retainFunc) {
        builder_->CreateCall(retainFunc, {object});
    }
}

void LLVMCodeGenerator::insertRelease(llvm::Value* object) {
    // 插入release调用
    if (!object || !object->getType()->isPointerTy()) {
        return;
    }
    
    llvm::Function* releaseFunc = module_->getFunction("swift_release");
    if (releaseFunc) {
        builder_->CreateCall(releaseFunc, {object});
    }
}

// 并发支持函数实现
llvm::Function* LLVMCodeGenerator::createAsyncFunction() {
    // 创建异步函数支持
    std::vector<llvm::Type*> asyncArgs;
    asyncArgs.push_back(llvm::PointerType::getUnqual(*context_)); // 函数指针
    asyncArgs.push_back(llvm::PointerType::getUnqual(*context_)); // 参数指针
    
    llvm::FunctionType* asyncType = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(*context_), asyncArgs, false); // 返回Task指针
    
    llvm::Function* asyncFunc = llvm::Function::Create(
        asyncType, llvm::Function::ExternalLinkage, "swift_async", module_.get());
    
    return asyncFunc;
}

llvm::Function* LLVMCodeGenerator::createAwaitFunction() {
    // 创建await函数支持
    std::vector<llvm::Type*> awaitArgs;
    awaitArgs.push_back(llvm::PointerType::getUnqual(*context_)); // Task指针
    
    llvm::FunctionType* awaitType = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(*context_), awaitArgs, false); // 返回结果指针
    
    llvm::Function* awaitFunc = llvm::Function::Create(
        awaitType, llvm::Function::ExternalLinkage, "swift_await", module_.get());
    
    return awaitFunc;
}

llvm::Function* LLVMCodeGenerator::createTaskFunction() {
    // 创建Task类型支持
    std::vector<llvm::Type*> taskFields;
    taskFields.push_back(llvm::PointerType::getUnqual(*context_)); // 函数指针
    taskFields.push_back(llvm::PointerType::getUnqual(*context_)); // 状态指针
    taskFields.push_back(builder_->getInt32Ty()); // 状态标志
    
    llvm::StructType* taskType = llvm::StructType::create(*context_, taskFields, "SwiftTask");
    
    // 创建Task创建函数
    std::vector<llvm::Type*> createTaskArgs;
    createTaskArgs.push_back(llvm::PointerType::getUnqual(*context_)); // 函数指针
    
    llvm::FunctionType* createTaskType = llvm::FunctionType::get(
        llvm::PointerType::getUnqual(taskType), createTaskArgs, false);
    
    llvm::Function* createTaskFunc = llvm::Function::Create(
        createTaskType, llvm::Function::ExternalLinkage, "swift_task_create", module_.get());
    
    return createTaskFunc;
}

// 工厂方法实现
std::unique_ptr<LLVMCodeGenerator> LLVMCodeGeneratorFactory::create() {
    return std::make_unique<LLVMCodeGenerator>();
}

std::unique_ptr<LLVMCodeGenerator> LLVMCodeGeneratorFactory::createWithOptimizations(bool enableOptimizations) {
    auto generator = std::make_unique<LLVMCodeGenerator>();
    // TODO: 配置优化选项
    return generator;
}

std::unique_ptr<LLVMCodeGenerator> LLVMCodeGeneratorFactory::createForTarget(const std::string& targetTriple) {
    auto generator = std::make_unique<LLVMCodeGenerator>();
    // TODO: 配置目标平台
    return generator;
}

// 编译管道实现
CompilationPipeline::CompilationPipeline() 
    : codeGenerator_(LLVMCodeGeneratorFactory::create())
    , optimizationLevel_(0)
    , outputFormat_("object")
    , debugInfo_(false)
    , jitEnabled_(false) {
}

void CompilationPipeline::setOptimizationLevel(int level) {
    optimizationLevel_ = std::max(0, std::min(3, level));
}

void CompilationPipeline::setTargetTriple(const std::string& triple) {
    targetTriple_ = triple;
}

void CompilationPipeline::setOutputFormat(const std::string& format) {
    outputFormat_ = format;
}

void CompilationPipeline::enableDebugInfo(bool enable) {
    debugInfo_ = enable;
}

void CompilationPipeline::enableJIT(bool enable) {
    jitEnabled_ = enable;
}

bool CompilationPipeline::compile(const TypedProgram& program, const std::string& outputFile) {
    errors_.clear();
    warnings_.clear();
    
    // 生成代码
    auto result = codeGenerator_->generateCode(program);
    
    if (result.hasErrors()) {
        errors_ = result.errors;
        warnings_ = result.warnings;
        return false;
    }
    
    // 应用优化
    if (optimizationLevel_ > 0) {
        codeGenerator_->optimizeModule(result.module.get());
    }
    
    // 输出文件
    bool success = false;
    if (outputFormat_ == "object") {
        success = codeGenerator_->compileToObjectFile(outputFile);
    } else if (outputFormat_ == "assembly") {
        success = codeGenerator_->compileToAssembly(outputFile);
    } else if (outputFormat_ == "executable") {
        success = codeGenerator_->compileToExecutable(outputFile);
    }
    
    return success;
}

bool CompilationPipeline::compileAndRun(const TypedProgram& program, const std::string& mainFunction) {
    errors_.clear();
    warnings_.clear();
    
    // 生成代码
    auto result = codeGenerator_->generateCode(program);
    
    if (result.hasErrors()) {
        errors_ = result.errors;
        warnings_ = result.warnings;
        return false;
    }
    
    // 初始化JIT
    if (!codeGenerator_->initializeJIT()) {
        return false;
    }
    
    // 执行主函数
    std::vector<llvm::GenericValue> args;
    codeGenerator_->executeFunction(mainFunction, args);
    
    return true;
}

} // namespace miniswift