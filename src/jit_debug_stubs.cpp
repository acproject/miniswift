// JIT debug stubs to resolve missing symbols
// These are required by LLVM's JIT debugging infrastructure

extern "C" {
    // GDB JIT interface symbols
    void* __jit_debug_descriptor = nullptr;
    void __jit_debug_register_code() {}
}