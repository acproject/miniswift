#include "IONetworkIntegration.h"
#include "IO/IORuntime.h"
#include "Network/NetworkRuntime.h"
#include <iostream>
#include <cassert>

namespace miniswift {

// 测试IO功能
void testIOFunctionality() {
    std::cout << "Testing IO functionality..." << std::endl;
    
    // 初始化IO运行时
    IORuntime::initialize();
    
    // 测试文件写入
    std::string testContent = "Hello, MiniSwift IO!";
    std::string testFile = "/tmp/miniswift_test.txt";
    
    auto& ioRuntime = IORuntime::getInstance();
    auto writeResult = ioRuntime.writeFile(testFile, std::vector<uint8_t>(testContent.begin(), testContent.end()));
    
    if (writeResult.success) {
        std::cout << "✓ File write successful" << std::endl;
    } else {
        std::cout << "✗ File write failed: " << writeResult.errorMessage << std::endl;
    }
    
    // 测试文件读取
    auto readResult = ioRuntime.readFile(testFile);
    if (readResult.success) {
        std::string readContent(readResult.data.begin(), readResult.data.end());
        if (readContent == testContent) {
            std::cout << "✓ File read successful and content matches" << std::endl;
        } else {
            std::cout << "✗ File read content mismatch" << std::endl;
        }
    } else {
        std::cout << "✗ File read failed: " << readResult.errorMessage << std::endl;
    }
    
    // 测试文件删除
    bool deleteResult = ioRuntime.deleteFile(testFile);
    if (deleteResult) {
        std::cout << "✓ File delete successful" << std::endl;
    } else {
        std::cout << "✗ File delete failed" << std::endl;
    }
    
    IORuntime::shutdown();
}

// 测试Network功能
void testNetworkFunctionality() {
    std::cout << "\nTesting Network functionality..." << std::endl;
    
    // 初始化网络运行时
    NetworkRuntime::initialize();
    
    auto& networkRuntime = NetworkRuntime::getInstance();
    
    // 测试DNS解析
    auto dnsResult = networkRuntime.resolveHostname("www.google.com");
    if (dnsResult.success && !dnsResult.addresses.empty()) {
        std::cout << "✓ DNS resolution successful: " << dnsResult.addresses[0] << std::endl;
    } else {
        std::cout << "✗ DNS resolution failed: " << dnsResult.errorMessage << std::endl;
    }
    
    // 测试TCP Socket创建
    auto tcpSocket = networkRuntime.createTCPSocket();
    if (tcpSocket) {
        std::cout << "✓ TCP socket creation successful" << std::endl;
    } else {
        std::cout << "✗ TCP socket creation failed" << std::endl;
    }
    
    // 测试UDP Socket创建
    auto udpSocket = networkRuntime.createUDPSocket();
    if (udpSocket) {
        std::cout << "✓ UDP socket creation successful" << std::endl;
    } else {
        std::cout << "✗ UDP socket creation failed" << std::endl;
    }
    
    // 测试HTTP客户端创建
    auto httpClient = networkRuntime.createHTTPClient();
    if (httpClient) {
        std::cout << "✓ HTTP client creation successful" << std::endl;
    } else {
        std::cout << "✗ HTTP client creation failed" << std::endl;
    }
    
    NetworkRuntime::shutdown();
}

// 测试集成功能
void testIntegrationFunctionality() {
    std::cout << "\nTesting Integration functionality..." << std::endl;
    
    // 初始化运行时
    IORuntime::initialize();
    NetworkRuntime::initialize();
    
    // 创建IONetworkBridge
    IONetworkBridge bridge;
    
    // 测试文件操作的Value转换
    Value testData(std::string("Test data for integration"));
    std::string filename = "/tmp/integration_test.txt";
    
    auto writeResult = bridge.writeFile(filename, testData);
    std::cout << "Integration write test completed" << std::endl;
    
    auto readResult = bridge.readFile(filename);
    std::cout << "Integration read test completed" << std::endl;
    
    // 清理
    bridge.deleteFile(filename);
    
    NetworkRuntime::shutdown();
    IORuntime::shutdown();
}

} // namespace miniswift

int main() {
    std::cout << "MiniSwift IO/Network Framework Test Suite" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    try {
        miniswift::testIOFunctionality();
        miniswift::testNetworkFunctionality();
        miniswift::testIntegrationFunctionality();
        
        std::cout << "\n==========================================" << std::endl;
        std::cout << "All tests completed!" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}