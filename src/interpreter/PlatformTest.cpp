#include "Network/NetworkRuntime.h"
#include <iostream>

int main() {
    std::cout << "Platform Compatibility Test" << std::endl;
    std::cout << "===========================" << std::endl;
    
#ifdef _WIN32
    std::cout << "Platform: Windows" << std::endl;
    std::cout << "Socket type: SOCKET (" << sizeof(socket_t) << " bytes)" << std::endl;
#else
    std::cout << "Platform: Unix/Linux/macOS" << std::endl;
    std::cout << "Socket type: int (" << sizeof(socket_t) << " bytes)" << std::endl;
#endif
    
    // 测试NetworkRuntime初始化
    try {
        miniswift::NetworkRuntime::initialize();
        std::cout << "✓ NetworkRuntime initialization successful" << std::endl;
        
        auto& runtime = miniswift::NetworkRuntime::getInstance();
        auto tcpSocket = runtime.createTCPSocket();
        if (tcpSocket) {
            std::cout << "✓ TCP socket creation successful" << std::endl;
        }
        
        miniswift::NetworkRuntime::shutdown();
        std::cout << "✓ NetworkRuntime shutdown successful" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "✗ Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "===========================" << std::endl;
    std::cout << "Platform compatibility test passed!" << std::endl;
    
    return 0;
}