#include "ARC.h"
#include "../Value.h"
#include <iostream>
#include <algorithm>
#include <queue>

namespace miniswift {

// 静态成员初始化
std::unique_ptr<ARCManager> ARCManager::instance = nullptr;
std::once_flag ARCManager::initFlag;

// 获取单例实例
ARCManager& ARCManager::getInstance() {
    std::call_once(initFlag, []() {
        instance = std::unique_ptr<ARCManager>(new ARCManager());
    });
    return *instance;
}

// 增加引用计数
void ARCManager::retain(void* object) {
    if (!object) return;
    
    std::lock_guard<std::mutex> lock(arcMutex);
    refCounts[object]++;
    
    #ifdef DEBUG_ARC
    std::cout << "[ARC] Retained object " << object << ", ref count: " << refCounts[object] << std::endl;
    #endif
}

// 减少引用计数
void ARCManager::release(void* object) {
    if (!object) return;
    
    std::lock_guard<std::mutex> lock(arcMutex);
    
    auto it = refCounts.find(object);
    if (it == refCounts.end()) {
        #ifdef DEBUG_ARC
        std::cerr << "[ARC] Warning: Attempting to release object " << object << " with no ref count" << std::endl;
        #endif
        return;
    }
    
    it->second--;
    
    #ifdef DEBUG_ARC
    std::cout << "[ARC] Released object " << object << ", ref count: " << it->second << std::endl;
    #endif
    
    if (it->second <= 0) {
        // 引用计数为0，清理对象
        cleanup(object);
        refCounts.erase(it);
    }
}

// 获取引用计数
int ARCManager::getRefCount(void* object) const {
    if (!object) return 0;
    
    std::lock_guard<std::mutex> lock(arcMutex);
    auto it = refCounts.find(object);
    return (it != refCounts.end()) ? it->second : 0;
}

// 注册弱引用
void ARCManager::registerWeakRef(void* object, std::function<void()> callback) {
    if (!object) return;
    
    std::lock_guard<std::mutex> lock(arcMutex);
    weakRefCallbacks[object].push_back(std::move(callback));
}

// 注册无主引用
void ARCManager::registerUnownedRef(void* object, std::function<void()> callback) {
    if (!object) return;
    
    std::lock_guard<std::mutex> lock(arcMutex);
    unownedRefCallbacks[object].push_back(std::move(callback));
}

// 添加强引用关系
void ARCManager::addStrongReference(void* from, void* to) {
    if (!from || !to || from == to) return;
    
    std::lock_guard<std::mutex> lock(arcMutex);
    strongReferences[from].insert(to);
    
    #ifdef DEBUG_ARC
    std::cout << "[ARC] Added strong reference: " << from << " -> " << to << std::endl;
    #endif
}

// 移除强引用关系
void ARCManager::removeStrongReference(void* from, void* to) {
    if (!from || !to) return;
    
    std::lock_guard<std::mutex> lock(arcMutex);
    auto it = strongReferences.find(from);
    if (it != strongReferences.end()) {
        it->second.erase(to);
        if (it->second.empty()) {
            strongReferences.erase(it);
        }
    }
    
    #ifdef DEBUG_ARC
    std::cout << "[ARC] Removed strong reference: " << from << " -> " << to << std::endl;
    #endif
}

// 检测强引用循环
bool ARCManager::detectCycle(void* startObject) const {
    if (!startObject) return false;
    
    std::lock_guard<std::mutex> lock(arcMutex);
    std::unordered_set<void*> visited;
    std::unordered_set<void*> recursionStack;
    
    return dfsDetectCycle(startObject, startObject, visited, recursionStack);
}

// 深度优先搜索检测循环
bool ARCManager::dfsDetectCycle(void* current, void* target, 
                               std::unordered_set<void*>& visited, 
                               std::unordered_set<void*>& recursionStack) const {
    if (recursionStack.find(current) != recursionStack.end()) {
        // 找到循环
        return true;
    }
    
    if (visited.find(current) != visited.end()) {
        // 已经访问过，但不在当前递归栈中
        return false;
    }
    
    visited.insert(current);
    recursionStack.insert(current);
    
    auto it = strongReferences.find(current);
    if (it != strongReferences.end()) {
        for (void* neighbor : it->second) {
            if (dfsDetectCycle(neighbor, target, visited, recursionStack)) {
                return true;
            }
        }
    }
    
    recursionStack.erase(current);
    return false;
}

// 获取所有强引用循环
std::vector<std::vector<void*>> ARCManager::getAllCycles() const {
    std::lock_guard<std::mutex> lock(arcMutex);
    std::vector<std::vector<void*>> cycles;
    std::unordered_set<void*> globalVisited;
    
    for (const auto& pair : strongReferences) {
        void* startObject = pair.first;
        if (globalVisited.find(startObject) != globalVisited.end()) {
            continue;
        }
        
        std::unordered_set<void*> visited;
        std::unordered_set<void*> recursionStack;
        std::vector<void*> currentPath;
        
        if (findCyclePath(startObject, visited, recursionStack, currentPath, cycles)) {
            // 将路径中的所有对象标记为已访问
            for (void* obj : currentPath) {
                globalVisited.insert(obj);
            }
        }
    }
    
    return cycles;
}

// 查找循环路径的辅助函数
bool ARCManager::findCyclePath(void* current, 
                              std::unordered_set<void*>& visited,
                              std::unordered_set<void*>& recursionStack,
                              std::vector<void*>& currentPath,
                              std::vector<std::vector<void*>>& cycles) const {
    if (recursionStack.find(current) != recursionStack.end()) {
        // 找到循环，提取循环路径
        auto cycleStart = std::find(currentPath.begin(), currentPath.end(), current);
        if (cycleStart != currentPath.end()) {
            std::vector<void*> cycle(cycleStart, currentPath.end());
            cycles.push_back(cycle);
            return true;
        }
    }
    
    if (visited.find(current) != visited.end()) {
        return false;
    }
    
    visited.insert(current);
    recursionStack.insert(current);
    currentPath.push_back(current);
    
    auto it = strongReferences.find(current);
    if (it != strongReferences.end()) {
        for (void* neighbor : it->second) {
            if (findCyclePath(neighbor, visited, recursionStack, currentPath, cycles)) {
                return true;
            }
        }
    }
    
    recursionStack.erase(current);
    currentPath.pop_back();
    return false;
}

// 清理对象
void ARCManager::cleanup(void* object) {
    if (!object) return;
    
    #ifdef DEBUG_ARC
    std::cout << "[ARC] Cleaning up object " << object << std::endl;
    #endif
    
    // 通知所有弱引用
    notifyWeakRefs(object);
    
    // 通知所有无主引用
    notifyUnownedRefs(object);
    
    // 移除所有从此对象发出的强引用
    auto it = strongReferences.find(object);
    if (it != strongReferences.end()) {
        strongReferences.erase(it);
    }
    
    // 移除所有指向此对象的强引用
    for (auto& pair : strongReferences) {
        pair.second.erase(object);
    }
    
    // 清理回调列表
    weakRefCallbacks.erase(object);
    unownedRefCallbacks.erase(object);
}

// 通知弱引用对象已被销毁
void ARCManager::notifyWeakRefs(void* object) {
    auto it = weakRefCallbacks.find(object);
    if (it != weakRefCallbacks.end()) {
        for (auto& callback : it->second) {
            try {
                callback();
            } catch (const std::exception& e) {
                #ifdef DEBUG_ARC
                std::cerr << "[ARC] Exception in weak ref callback: " << e.what() << std::endl;
                #endif
            }
        }
    }
}

// 通知无主引用对象已被销毁
void ARCManager::notifyUnownedRefs(void* object) {
    auto it = unownedRefCallbacks.find(object);
    if (it != unownedRefCallbacks.end()) {
        for (auto& callback : it->second) {
            try {
                callback();
            } catch (const std::exception& e) {
                #ifdef DEBUG_ARC
                std::cerr << "[ARC] Exception in unowned ref callback: " << e.what() << std::endl;
                #endif
            }
        }
    }
}

// 调试信息
void ARCManager::printDebugInfo() const {
    std::lock_guard<std::mutex> lock(arcMutex);
    
    std::cout << "\n=== ARC Debug Information ===" << std::endl;
    std::cout << "Total objects tracked: " << refCounts.size() << std::endl;
    
    std::cout << "\nReference counts:" << std::endl;
    for (const auto& pair : refCounts) {
        std::cout << "  Object " << pair.first << ": " << pair.second << " refs" << std::endl;
    }
    
    std::cout << "\nStrong references:" << std::endl;
    for (const auto& pair : strongReferences) {
        std::cout << "  From " << pair.first << " to: ";
        for (void* target : pair.second) {
            std::cout << target << " ";
        }
        std::cout << std::endl;
    }
    
    std::cout << "\nWeak references tracked: " << weakRefCallbacks.size() << " objects" << std::endl;
    std::cout << "Unowned references tracked: " << unownedRefCallbacks.size() << " objects" << std::endl;
    
    // 检测循环
    auto cycles = getAllCycles();
    if (!cycles.empty()) {
        std::cout << "\n*** DETECTED REFERENCE CYCLES ***" << std::endl;
        for (size_t i = 0; i < cycles.size(); ++i) {
            std::cout << "Cycle " << (i + 1) << ": ";
            for (void* obj : cycles[i]) {
                std::cout << obj << " -> ";
            }
            std::cout << cycles[i][0] << std::endl;
        }
    } else {
        std::cout << "\nNo reference cycles detected." << std::endl;
    }
    
    std::cout << "============================\n" << std::endl;
}

// 获取内存统计信息
ARCManager::MemoryStats ARCManager::getMemoryStats() const {
    std::lock_guard<std::mutex> lock(arcMutex);
    
    MemoryStats stats;
    stats.totalObjects = refCounts.size();
    stats.totalStrongRefs = 0;
    stats.totalWeakRefs = weakRefCallbacks.size();
    stats.totalUnownedRefs = unownedRefCallbacks.size();
    
    for (const auto& pair : strongReferences) {
        stats.totalStrongRefs += pair.second.size();
    }
    
    stats.detectedCycles = getAllCycles().size();
    
    return stats;
}

} // namespace miniswift