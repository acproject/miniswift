LLVM代码生成和编译阶段 ：存在问题

- 编译器返回非零退出码（错误状态）
- 没有生成目标文件（.o文件）
- 没有生成可执行文件
- 缺少LLVM IR生成和编译过程的输出信息
增加多线程等
增加io库
增加network
暴露出Network和IO库的api ，并用swift文件进行调用 -> cpp-httplib 

并行库的加入
UI的完善

计算库的加入

如何让swift可以, AI化(主要结合Llama)

* https://docs.astral.sh/uv/ 

https://github.com/openai/gpt-oss

* yhirose/cpp-httplib - Single-header HTTP server, used by llama-server - MIT license
* stb-image - Single-header image format decoder, used by multimodal subsystem - Public domain
* nlohmann/json - Single-header JSON library, used by various tools/examples - MIT License
* minja - Minimal Jinja parser in C++, used by various tools/examples - MIT License
* linenoise.cpp - C++ library that provides readline-like line editing capabilities, used by llama-run - BSD 2-Clause License
* curl - Client-side URL transfer library, used by various tools/examples - CURL License
* miniaudio.h - Single-header audio format decoder, used by multimodal subsystem - Public domain