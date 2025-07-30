# MiniSwift IO/Network Framework

## 概述

本框架为MiniSwift解释器提供了完整的IO和网络功能支持，基于C++20标准实现，充分利用了现代C++的特性如协程、智能指针、并发编程等。

## 架构设计

### 核心模块

1. **IO模块** (`IO/IORuntime.h`, `IO/IORuntime.cpp`)
   - 文件和目录操作
   - 异步IO支持
   - IO调度器
   - 缓冲区管理

2. **Network模块** (`Network/NetworkRuntime.h`, `Network/NetworkRuntime.cpp`)
   - TCP/UDP Socket支持
   - HTTP客户端和服务器
   - DNS解析
   - 网络调度器

3. **集成模块** (`IONetworkIntegration.h`, `IONetworkIntegration.cpp`)
   - Swift风格的API接口
   - Value类型转换
   - 错误处理
   - 异步操作包装

### 设计特点

- **单例模式**: IORuntime和NetworkRuntime采用单例模式，确保全局唯一性
- **异步支持**: 所有IO和网络操作都支持异步执行
- **线程安全**: 使用互斥锁和原子操作确保线程安全
- **资源管理**: 使用RAII和智能指针进行自动资源管理
- **错误处理**: 统一的错误处理机制，支持Swift风格的Result类型

## 主要功能

### IO功能

#### 文件操作
```cpp
// 同步文件操作
IOResult readFile(const std::string& path);
IOResult writeFile(const std::string& path, const std::vector<uint8_t>& data);
IOResult writeFile(const std::string& path, const std::string& text);
bool deleteFile(const std::string& path);
bool copyFile(const std::string& source, const std::string& destination);
bool moveFile(const std::string& source, const std::string& destination);

// 异步文件操作
std::shared_ptr<AsyncIOOperation> readFileAsync(const std::string& path);
std::shared_ptr<AsyncIOOperation> writeFileAsync(const std::string& path, const std::vector<uint8_t>& data);
```

#### 目录操作
```cpp
std::unique_ptr<DirectoryHandle> openDirectory(const std::string& path);
bool create(bool recursive = false);
bool remove(bool recursive = false);
std::vector<std::string> list() const;
```

### Network功能

#### Socket操作
```cpp
// TCP Socket
std::shared_ptr<TCPSocket> createTCPSocket();
bool connect(const NetworkAddress& address);
bool listen(size_t backlog = 10);
std::shared_ptr<TCPSocket> accept();

// UDP Socket
std::shared_ptr<UDPSocket> createUDPSocket();
NetworkResult sendTo(const std::vector<uint8_t>& data, const NetworkAddress& address);
NetworkResult receiveFrom(size_t maxSize, NetworkAddress& fromAddress);
```

#### HTTP功能
```cpp
// HTTP客户端
std::unique_ptr<HTTPClient> createHTTPClient();
NetworkResult request(const HTTPRequest& request, HTTPResponse& response);
NetworkResult get(const std::string& url, HTTPResponse& response);
NetworkResult post(const std::string& url, const std::vector<uint8_t>& data, HTTPResponse& response);

// HTTP服务器
std::unique_ptr<HTTPServer> createHTTPServer();
void addRoute(const std::string& path, HTTPRequestHandler handler);
bool start(const NetworkAddress& address);
```

#### DNS解析
```cpp
DNSResult resolve(const std::string& hostname);
DNSResult reverseResolve(const std::string& ipAddress);
std::shared_ptr<AsyncNetworkOperation> resolveAsync(const std::string& hostname);
```

### Swift风格API

通过IONetworkBridge类提供Swift风格的接口：

```cpp
// 文件操作
static Value readFile(const std::string& path);
static Value writeFile(const std::string& path, const Value& content);
static Value deleteFile(const std::string& path);

// 网络操作
static Value httpGet(const std::string& url);
static Value httpPost(const std::string& url, const Value& data);
static Value createTCPSocket();
static Value connectTCP(const Value& socket, const std::string& host, uint16_t port);
```

## 使用示例

### 基本文件操作

```cpp
#include "IONetworkIntegration.h"

using namespace miniswift;

int main() {
    // 初始化运行时
    IORuntime::initialize();
    
    // 写入文件
    std::string content = "Hello, MiniSwift!";
    auto writeResult = IONetworkBridge::writeFile("/tmp/test.txt", Value(content));
    
    // 读取文件
    auto readResult = IONetworkBridge::readFile("/tmp/test.txt");
    
    // 清理
    IONetworkBridge::deleteFile("/tmp/test.txt");
    IORuntime::shutdown();
    
    return 0;
}
```

### HTTP请求示例

```cpp
#include "IONetworkIntegration.h"

using namespace miniswift;

int main() {
    // 初始化运行时
    NetworkRuntime::initialize();
    
    // 发送HTTP GET请求
    auto response = IONetworkBridge::httpGet("http://httpbin.org/get");
    
    // 发送HTTP POST请求
    Value postData(std::string("{\"key\": \"value\"}"));
    auto postResponse = IONetworkBridge::httpPost("http://httpbin.org/post", postData);
    
    NetworkRuntime::shutdown();
    return 0;
}
```

### 异步操作示例

```cpp
#include "IONetworkIntegration.h"

using namespace miniswift;

int main() {
    IORuntime::initialize();
    
    // 异步读取文件
    auto asyncOp = IORuntime::getInstance().readFileAsync("/tmp/large_file.txt");
    
    // 设置完成回调
    asyncOp->onCompletion([](const IOResult& result) {
        if (result.success) {
            std::cout << "File read successfully, size: " << result.data.size() << std::endl;
        } else {
            std::cout << "File read failed: " << result.errorMessage << std::endl;
        }
    });
    
    // 等待完成
    auto result = asyncOp->wait();
    
    IORuntime::shutdown();
    return 0;
}
```

## 编译和测试

### 编译要求

- C++20兼容的编译器 (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.20+
- pthread库支持

### 编译步骤

```bash
cd /Users/acproject/workspace/cpp_projects/miniswift/src/interpreter
mkdir build && cd build
cmake ..
make
```

### 运行测试

```bash
./bin/io_network_test
```

## 性能特性

- **异步IO**: 使用线程池和事件循环实现高性能异步IO
- **连接池**: HTTP客户端支持连接复用
- **缓冲区管理**: 智能缓冲区减少内存分配
- **零拷贝**: 在可能的情况下避免数据拷贝

## 错误处理

框架提供统一的错误处理机制：

```cpp
struct IOResult {
    bool success;
    std::string errorMessage;
    size_t bytesProcessed;
    std::vector<uint8_t> data;
    std::chrono::milliseconds duration;
};

struct NetworkResult {
    bool success;
    std::string errorMessage;
    size_t bytesTransferred;
    std::vector<uint8_t> data;
    std::chrono::milliseconds duration;
    std::string remoteAddress;
    uint16_t remotePort;
};
```

## 扩展性

框架设计考虑了扩展性：

- **插件架构**: 支持自定义IO和网络处理器
- **协议扩展**: 易于添加新的网络协议支持
- **平台适配**: 跨平台设计，支持不同操作系统

## 安全性

- **输入验证**: 所有外部输入都经过验证
- **资源限制**: 防止资源耗尽攻击
- **权限检查**: 文件操作前检查权限
- **SSL/TLS支持**: 为HTTPS提供安全传输（预留接口）

## 未来计划

1. **SSL/TLS支持**: 完整的HTTPS和安全Socket支持
2. **WebSocket**: WebSocket协议支持
3. **HTTP/2**: HTTP/2协议支持
4. **文件监控**: 文件系统变化监控
5. **压缩支持**: 数据压缩和解压缩
6. **缓存机制**: 智能缓存提升性能

## 贡献指南

欢迎贡献代码！请遵循以下规范：

1. 代码风格遵循项目现有规范
2. 添加适当的单元测试
3. 更新相关文档
4. 确保线程安全
5. 处理所有错误情况

## 许可证

本框架遵循项目整体许可证。