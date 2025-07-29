#include "../src/interpreter/Network/NetworkRuntime.h"
#include <iostream>
#include <errno.h>
#include <cstring>

using namespace miniswift;

int main() {
    std::cout << "=== 简单TCP连接测试 ===" << std::endl;
    
    // 初始化网络运行时
    NetworkRuntime::initialize();
    auto& runtime = NetworkRuntime::getInstance();
    
    // 1. DNS解析
    std::cout << "1. DNS解析 httpbin.org..." << std::endl;
    DNSResolver resolver;
    auto dnsResult = resolver.resolve("httpbin.org");
    
    if (!dnsResult.success || dnsResult.addresses.empty()) {
        std::cout << "   ✗ DNS解析失败: " << dnsResult.errorMessage << std::endl;
        return 1;
    }
    
    std::string ip = dnsResult.addresses[0];
    std::cout << "   ✓ DNS解析成功: " << ip << std::endl;
    
    // 2. 创建TCP Socket
    std::cout << "2. 创建TCP Socket..." << std::endl;
    auto tcpSocket = runtime.createTCPSocket();
    
    if (!tcpSocket || tcpSocket->getSocketFd() == -1) {
        std::cout << "   ✗ TCP Socket创建失败" << std::endl;
        return 1;
    }
    
    std::cout << "   ✓ TCP Socket创建成功, fd: " << tcpSocket->getSocketFd() << std::endl;
    
    // 3. 尝试连接
    std::cout << "3. 连接到 " << ip << ":80..." << std::endl;
    NetworkAddress address(ip, 80);
    
    bool connected = tcpSocket->connect(address);
    
    if (connected) {
        std::cout << "   ✓ 连接成功!" << std::endl;
        
        // 4. 发送简单的HTTP请求
        std::cout << "4. 发送HTTP GET请求..." << std::endl;
        std::string request = "GET /get HTTP/1.1\r\nHost: httpbin.org\r\nConnection: close\r\n\r\n";
        std::vector<uint8_t> requestData(request.begin(), request.end());
        
        auto sendResult = tcpSocket->send(requestData);
        if (sendResult.success) {
            std::cout << "   ✓ 请求发送成功, 发送了 " << sendResult.bytesTransferred << " 字节" << std::endl;
            
            // 5. 接收响应
            std::cout << "5. 接收响应..." << std::endl;
            auto receiveResult = tcpSocket->receive(1024);
            
            if (receiveResult.success && !receiveResult.data.empty()) {
                std::string response(receiveResult.data.begin(), receiveResult.data.end());
                std::cout << "   ✓ 接收到响应 (" << receiveResult.bytesTransferred << " 字节):" << std::endl;
                std::cout << "   前100个字符: " << response.substr(0, 100) << "..." << std::endl;
            } else {
                std::cout << "   ✗ 接收响应失败: " << receiveResult.errorMessage << std::endl;
            }
        } else {
            std::cout << "   ✗ 请求发送失败: " << sendResult.errorMessage << std::endl;
        }
        
        tcpSocket->close();
    } else {
        std::cout << "   ✗ 连接失败, errno: " << errno << " (" << strerror(errno) << ")" << std::endl;
    }
    
    // 清理
    NetworkRuntime::shutdown();
    
    std::cout << "\n=== 测试完成 ===" << std::endl;
    return connected ? 0 : 1;
}