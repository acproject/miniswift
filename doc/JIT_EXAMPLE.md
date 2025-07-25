
### 实际构建执行引擎的代码如下
```cpp

#include <memory>
#include "exec_engine.h"
#include "exec_node.h"
#include "function_registry.h"
#include "type.h"

struct TestStruct {
  std::vector<uint32_t> a;
  std::vector<uint32_t> b;
};

jitfusion::LLVMComplexStruct load_u32s(int64_t entry_args) {
  auto *text_struct = reinterpret_cast<TestStruct *>(entry_args);
  jitfusion::LLVMComplexStruct ret{reinterpret_cast<int64_t>(text_struct->a.data()),
                                   static_cast<uint32_t>(text_struct->a.size())};
  return ret;
}

jitfusion::LLVMComplexStruct add(jitfusion::LLVMComplexStruct a, int64_t exec_context) {
  auto *data = reinterpret_cast<uint32_t *>(a.data);
  jitfusion::LLVMComplexStruct b;
  auto *new_data = reinterpret_cast<uint32_t *>(
      reinterpret_cast<jitfusion::ExecContext *>(exec_context)->arena.Allocate(a.len * sizeof(uint32_t)));
  b.len = a.len;
  for (uint32_t i = 0; i < a.len; i++) {
    new_data[i] = data[i] + 1;
  }
  b.data = reinterpret_cast<int64_t>(new_data);
  return b;
}

uint8_t store_u32s(int64_t entry_args, jitfusion::LLVMComplexStruct a) {
  auto *text_struct = reinterpret_cast<TestStruct *>(entry_args);
  auto *write_data = reinterpret_cast<uint32_t *>(a.data);
  text_struct->b.insert(text_struct->b.end(), write_data, write_data + a.len);
  return 0;
}

int main() {
  std::unique_ptr<jitfusion::FunctionRegistry> func_registry;
  jitfusion::FunctionRegistryFactory::CreateFunctionRegistry(&func_registry);
  jitfusion::FunctionSignature load_sign("load_u32s", {jitfusion::ValueType::kI64}, jitfusion::ValueType::kU32List);
  jitfusion::FunctionStructure load_func_struct = {jitfusion::FunctionType::kCFunc, reinterpret_cast<void *>(load_u32s),
                                                   nullptr};
  func_registry->RegisterFunc(load_sign, load_func_struct);
  jitfusion::FunctionSignature store_sign("store_u32s", {jitfusion::ValueType::kI64, jitfusion::ValueType::kU32List},
                                          jitfusion::ValueType::kU8);
  jitfusion::FunctionStructure store_func_struct = {jitfusion::FunctionType::kCFunc,
                                                    reinterpret_cast<void *>(store_u32s), nullptr};
  func_registry->RegisterFunc(store_sign, store_func_struct);

  jitfusion::FunctionSignature add_sign("add", {jitfusion::ValueType::kU32List, jitfusion::ValueType::kI64},
                                        jitfusion::ValueType::kU32List);
  jitfusion::FunctionStructure add_func_struct = {jitfusion::FunctionType::kCFunc, reinterpret_cast<void *>(add),
                                                  nullptr};
  func_registry->RegisterFunc(add_sign, add_func_struct);

  auto entry_node1 = std::unique_ptr<jitfusion::ExecNode>(new jitfusion::EntryArgumentNode);
  std::vector<std::unique_ptr<jitfusion::ExecNode>> load_func_args;
  load_func_args.emplace_back(std::move(entry_node1));
  auto load_node =
      std::unique_ptr<jitfusion::ExecNode>(new jitfusion::FunctionNode("load_u32s", std::move(load_func_args)));

  auto exec_ctx_node = std::unique_ptr<jitfusion::ExecNode>(new jitfusion::ExecContextNode);
  std::vector<std::unique_ptr<jitfusion::ExecNode>> add_func_args;
  add_func_args.emplace_back(std::move(load_node));
  add_func_args.emplace_back(std::move(exec_ctx_node));
  auto add_node = std::unique_ptr<jitfusion::ExecNode>(new jitfusion::FunctionNode("add", std::move(add_func_args)));

  auto entry_node2 = std::unique_ptr<jitfusion::ExecNode>(new jitfusion::EntryArgumentNode);
  std::vector<std::unique_ptr<jitfusion::ExecNode>> store_func_args;
  store_func_args.emplace_back(std::move(entry_node2));
  store_func_args.emplace_back(std::move(add_node));
  auto store_node =
      std::unique_ptr<jitfusion::ExecNode>(new jitfusion::FunctionNode("store_u32s", std::move(store_func_args)));

  jitfusion::ExecEngine exec_engine;
  exec_engine.Compile(store_node, func_registry);

  TestStruct text_st;
  text_st.a = {1, 2, 3, 4};
  jitfusion::RetType result;
  exec_engine.Execute(&text_st, &result);
  for (const auto &data : text_st.b) {
    std::cout << data << std::endl;
  }
}

struct FunctionStructure {
  FunctionType func_type;
  void *c_func_ptr;
  std::function<llvm::Value *(const FunctionSignature &, const std::vector<llvm::Type *> &,
                              const std::vector<llvm::Value *> &, IRCodeGenContext &)>
      codegen_func;
};


struct FunctionSignature {
  std::string func_name_;
  std::vector<ValueType> param_types_;
  ValueType ret_type_;
};


```