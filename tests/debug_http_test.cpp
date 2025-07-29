#include "../src/interpreter/Network/NetworkRuntime.h"
#include <iostream>
#include <errno.h>
#include <cstring>

using namespace miniswift;

int main() {
    std::cout << "=== HTTPClient 详细调试测试 ===" << std::endl;
    
    // 初始化网络运行时
    NetworkRuntime::initialize();
    auto& runtime = NetworkRuntime::getInstance();
    
    // 创建HTTPClient
    std::cout << "1. 创建HTTPClient..." << std::endl;
    auto httpClient = runtime.createHTTPClient();
    
    if (!httpClient) {
        std::cout << "   ✗ HTTPClient创建失败" << std::endl;
        return 1;
    }
    
    std::cout << "   ✓ HTTPClient创建成功" << std::endl;
    
    // 准备HTTP请求
    std::cout << "2. 准备HTTP请求..." << std::endl;
    HTTPRequest request;
    request.method = HTTPMethod::GET;
    request.url = "http://httpbin.org/get";
    request.version = "HTTP/1.1";
    
    std::cout << "   ✓ HTTP请求准备完成" << std::endl;
    std::cout << "   URL: " << request.url << std::endl;
    
    // 执行请求
    std::cout << "3. 执行HTTP请求..." << std::endl;
    HTTPResponse response;
    auto result = httpClient->request(request, response);
    
    if (result.success) {
        std::cout << "   ✓ HTTP请求成功!" << std::endl;
        std::cout << "   状态码: " << response.statusCode << std::endl;
        std::cout << "   状态消息: " << response.statusMessage << std::endl;
        std::cout << "   响应体长度: " << response.body.size() << " 字节" << std::endl;
        
        if (!response.body.empty()) {
            std::string bodyStr = response.getBodyAsString();
            std::cout << "   响应体前100个字符: " << bodyStr.substr(0, 100) << "..." << std::endl;
        }
    } else {
        std::cout << "   ✗ HTTP请求失败: " << result.errorMessage << std::endl;
        
        // 尝试手动测试连接步骤
        std::cout << "\n4. 手动测试连接步骤..." << std::endl;
        
        // DNS解析
        std::cout << "   4.1 DNS解析..." << std::endl;
        DNSResolver resolver;
        auto dnsResult = resolver.resolve("httpbin.org");
        
        if (dnsResult.success && !dnsResult.addresses.empty()) {
            std::cout << "       ✓ DNS解析成功: " << dnsResult.addresses[0] << std::endl;
            
            // 创建TCP Socket
            std::cout << "   4.2 创建TCP Socket..." << std::endl;
            auto tcpSocket = runtime.createTCPSocket();
            
            if (tcpSocket && tcpSocket->getSocketFd() != -1) {
                std::cout << "       ✓ TCP Socket创建成功, fd: " << tcpSocket->getSocketFd() << std::endl;
                
                // 尝试连接
                std::cout << "   4.3 尝试连接..." << std::endl;
                NetworkAddress address(dnsResult.addresses[0], 80);
                bool connected = tcpSocket->connect(address);
                
                if (connected) {
                    std::cout << "       ✓ 连接成功!" << std::endl;
                    tcpSocket->close();
                } else {
                    std::cout << "       ✗ 连接失败, errno: " << errno << " (" << strerror(errno) << ")" << std::endl;
                }
            } else {
                std::cout << "       ✗ TCP Socket创建失败" << std::endl;
            }
        } else {
            std::cout << "       ✗ DNS解析失败: " << dnsResult.errorMessage << std::endl;
        }
    }
    
    // 清理
    NetworkRuntime::shutdown();
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return result.success ? 0 : 1;
}