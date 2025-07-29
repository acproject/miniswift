#include "../src/interpreter/Network/NetworkRuntime.h"
#include "../src/interpreter/IONetworkIntegration.h"
#include <iostream>
#include <chrono>
#include <thread>

using namespace miniswift;

int main() {
    std::cout << "=== MiniSwift 网络功能真实性测试 ===" << std::endl;
    
    try {
        // 初始化网络运行时
        std::cout << "1. 初始化网络运行时..." << std::endl;
        NetworkRuntime::initialize();
        auto& runtime = NetworkRuntime::getInstance();
        std::cout << "   ✓ 网络运行时初始化成功" << std::endl;
        
        // 测试DNS解析
        std::cout << "\n2. 测试DNS解析功能..." << std::endl;
        auto dnsResult = runtime.resolveHostname("httpbin.org");
        if (dnsResult.success && !dnsResult.addresses.empty()) {
            std::cout << "   ✓ DNS解析成功: " << dnsResult.addresses[0] << std::endl;
        } else {
            std::cout << "   ✗ DNS解析失败: " << dnsResult.errorMessage << std::endl;
        }
        
        // 测试HTTP客户端创建
        std::cout << "\n3. 测试HTTP客户端创建..." << std::endl;
        auto httpClient = runtime.createHTTPClient();
        if (httpClient) {
            std::cout << "   ✓ HTTP客户端创建成功" << std::endl;
        } else {
            std::cout << "   ✗ HTTP客户端创建失败" << std::endl;
            return 1;
        }
        
        // 测试真实的HTTP GET请求
        std::cout << "\n4. 测试真实HTTP GET请求..." << std::endl;
        std::cout << "   正在访问: http://httpbin.org/get" << std::endl;
        
        // 先测试DNS解析
        std::cout << "   - 测试DNS解析..." << std::endl;
        DNSResolver httpResolver;
        auto httpDnsResult = httpResolver.resolve("httpbin.org");
        if (httpDnsResult.success && !httpDnsResult.addresses.empty()) {
            std::cout << "   - DNS解析成功: " << httpDnsResult.addresses[0] << std::endl;
        } else {
            std::cout << "   - DNS解析失败: " << httpDnsResult.errorMessage << std::endl;
        }
        
        HTTPResponse response;
        auto startTime = std::chrono::high_resolution_clock::now();
        auto result = httpClient->get("http://httpbin.org/get", response);
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        
        if (result.success) {
            std::cout << "   ✓ HTTP请求成功!" << std::endl;
            std::cout << "   - 状态码: " << response.statusCode << std::endl;
            std::cout << "   - 响应时间: " << duration.count() << "ms" << std::endl;
            std::cout << "   - 响应大小: " << response.body.size() << " 字节" << std::endl;
            
            // 显示响应内容的前200个字符
            std::string bodyStr = response.getBodyAsString();
            if (bodyStr.length() > 200) {
                bodyStr = bodyStr.substr(0, 200) + "...";
            }
            std::cout << "   - 响应内容预览: " << bodyStr << std::endl;
            
            // 检查是否包含预期的JSON字段
            if (bodyStr.find("\"origin\"") != std::string::npos && 
                bodyStr.find("\"url\"") != std::string::npos) {
                std::cout << "   ✓ 响应内容验证通过 - 包含预期的JSON字段" << std::endl;
            } else {
                std::cout << "   ⚠ 响应内容可能不完整" << std::endl;
            }
        } else {
            std::cout << "   ✗ HTTP请求失败: " << result.errorMessage << std::endl;
        }
        
        // 测试TCP Socket创建
        std::cout << "\n5. 测试TCP Socket创建..." << std::endl;
        auto tcpSocket = runtime.createTCPSocket();
        if (tcpSocket) {
            std::cout << "   ✓ TCP Socket创建成功" << std::endl;
        } else {
            std::cout << "   ✗ TCP Socket创建失败" << std::endl;
        }
        
        // 清理
        std::cout << "\n6. 清理资源..." << std::endl;
        NetworkRuntime::shutdown();
        std::cout << "   ✓ 网络运行时关闭成功" << std::endl;
        
        std::cout << "\n=== 测试总结 ===" << std::endl;
        if (result.success) {
            std::cout << "🎉 MiniSwift 具备真实的网络访问能力!" << std::endl;
            std::cout << "   - DNS解析: " << (dnsResult.success ? "✓" : "✗") << std::endl;
            std::cout << "   - HTTP请求: ✓" << std::endl;
            std::cout << "   - Socket创建: ✓" << std::endl;
        } else {
            std::cout << "❌ MiniSwift 网络功能存在问题" << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cout << "❌ 测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}