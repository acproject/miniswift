#include "MemorySafety.h"
#include "../Value.h"

namespace miniswift {

// SafePtr模板类实现
template<typename T>
SafePtr<T>::SafePtr() : ptr(nullptr), isValid(std::make_shared<std::atomic<bool>>(false)) {}

template<typename T>
SafePtr<T>::SafePtr(T* p, const std::string& info)
    : ptr(p), isValid(std::make_shared<std::atomic<bool>>(p != nullptr)), debugInfo(info) {
    if (ptr) {
        MemorySafetyManager::getInstance().registerPointer(ptr, isValid);
    }
}

template<typename T>
SafePtr<T>::SafePtr(const SafePtr& other)
    : ptr(other.ptr), isValid(other.isValid), debugInfo(other.debugInfo) {}

template<typename T>
SafePtr<T>::SafePtr(SafePtr&& other) noexcept
    : ptr(other.ptr), isValid(std::move(other.isValid)), debugInfo(std::move(other.debugInfo)) {
    other.ptr = nullptr;
}

template<typename T>
SafePtr<T>& SafePtr<T>::operator=(const SafePtr& other) {
    if (this != &other) {
        ptr = other.ptr;
        isValid = other.isValid;
        debugInfo = other.debugInfo;
    }
    return *this;
}

template<typename T>
SafePtr<T>& SafePtr<T>::operator=(SafePtr&& other) noexcept {
    if (this != &other) {
        ptr = other.ptr;
        isValid = std::move(other.isValid);
        debugInfo = std::move(other.debugInfo);
        other.ptr = nullptr;
    }
    return *this;
}

template<typename T>
SafePtr<T>::~SafePtr() {
    if (ptr && isValid) {
        MemorySafetyManager::getInstance().unregisterPointer(ptr);
    }
}

template<typename T>
T* SafePtr<T>::get() const {
    checkValidity();
    return ptr;
}

template<typename T>
T& SafePtr<T>::operator*() const {
    checkValidity();
    if (!ptr) {
        throw DanglingPointerException("Dereferencing null pointer: " + debugInfo);
    }
    return *ptr;
}

template<typename T>
T* SafePtr<T>::operator->() const {
    checkValidity();
    if (!ptr) {
        throw DanglingPointerException("Accessing null pointer: " + debugInfo);
    }
    return ptr;
}

template<typename T>
SafePtr<T>::operator bool() const {
    return ptr != nullptr && isValid && isValid->load();
}

template<typename T>
bool SafePtr<T>::operator==(const SafePtr& other) const {
    return ptr == other.ptr;
}

template<typename T>
bool SafePtr<T>::operator!=(const SafePtr& other) const {
    return ptr != other.ptr;
}

template<typename T>
void SafePtr<T>::reset(T* newPtr, const std::string& info) {
    if (ptr && isValid) {
        MemorySafetyManager::getInstance().unregisterPointer(ptr);
    }
    
    ptr = newPtr;
    isValid = std::make_shared<std::atomic<bool>>(newPtr != nullptr);
    debugInfo = info;
    
    if (ptr) {
        MemorySafetyManager::getInstance().registerPointer(ptr, isValid);
    }
}

template<typename T>
void SafePtr<T>::checkValidity() const {
    if (!isValid || !isValid->load()) {
        throw DanglingPointerException("Accessing deallocated memory: " + debugInfo);
    }
}

// 显式实例化常用类型
template class SafePtr<Value>;
template class SafePtr<int>;
template class SafePtr<double>;
template class SafePtr<char>;

} // namespace miniswift
#include <iostream>
#include <algorithm>

namespace miniswift {

// 静态成员初始化
std::unique_ptr<MemorySafetyManager> MemorySafetyManager::instance = nullptr;
std::once_flag MemorySafetyManager::initFlag;

// 获取单例实例
MemorySafetyManager& MemorySafetyManager::getInstance() {
    std::call_once(initFlag, []() {
        instance = std::unique_ptr<MemorySafetyManager>(new MemorySafetyManager());
    });
    return *instance;
}

// 注册指针
void MemorySafetyManager::registerPointer(void* ptr, std::shared_ptr<std::atomic<bool>> validity) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(safetyMutex);
    trackedPointers[ptr] = validity;
    
    if (debugMode.load()) {
        std::cout << "[MemSafety] Registered pointer: " << ptr << std::endl;
    }
}

// 注销指针
void MemorySafetyManager::unregisterPointer(void* ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(safetyMutex);
    trackedPointers.erase(ptr);
    
    if (debugMode.load()) {
        std::cout << "[MemSafety] Unregistered pointer: " << ptr << std::endl;
    }
}

// 使指针无效
void MemorySafetyManager::invalidatePointer(void* ptr) {
    if (!ptr) return;
    
    std::lock_guard<std::mutex> lock(safetyMutex);
    auto it = trackedPointers.find(ptr);
    if (it != trackedPointers.end()) {
        auto validity = it->second.lock();
        if (validity) {
            validity->store(false);
        }
        
        if (debugMode.load()) {
            std::cout << "[MemSafety] Invalidated pointer: " << ptr << std::endl;
        }
    }
}

// 检查指针是否有效
bool MemorySafetyManager::isPointerValid(void* ptr) const {
    if (!ptr) return false;
    
    std::lock_guard<std::mutex> lock(safetyMutex);
    auto it = trackedPointers.find(ptr);
    if (it != trackedPointers.end()) {
        auto validity = it->second.lock();
        return validity && validity->load();
    }
    return false;
}

// 注册内存区域
void MemorySafetyManager::registerMemoryRegion(void* start, size_t size, const std::string& debugInfo) {
    if (!start || size == 0) return;
    
    std::lock_guard<std::mutex> lock(safetyMutex);
    memoryRegions[start] = MemoryRegion(start, size, debugInfo);
    
    if (debugMode.load()) {
        std::cout << "[MemSafety] Registered memory region: " << start 
                  << " (size: " << size << ", info: " << debugInfo << ")" << std::endl;
    }
}

// 注销内存区域
void MemorySafetyManager::unregisterMemoryRegion(void* start) {
    if (!start) return;
    
    std::lock_guard<std::mutex> lock(safetyMutex);
    auto it = memoryRegions.find(start);
    if (it != memoryRegions.end()) {
        it->second.isValid = false;
        
        if (debugMode.load()) {
            std::cout << "[MemSafety] Unregistered memory region: " << start << std::endl;
        }
        
        memoryRegions.erase(it);
    }
}

// 检查内存区域是否有效
bool MemorySafetyManager::isMemoryRegionValid(void* ptr) const {
    if (!ptr) return false;
    
    std::lock_guard<std::mutex> lock(safetyMutex);
    for (const auto& pair : memoryRegions) {
        const MemoryRegion& region = pair.second;
        if (region.isValid && region.contains(ptr)) {
            return true;
        }
    }
    return false;
}

// 查找内存区域
MemoryRegion* MemorySafetyManager::findMemoryRegion(void* ptr) const {
    if (!ptr) return nullptr;
    
    std::lock_guard<std::mutex> lock(safetyMutex);
    for (auto& pair : memoryRegions) {
        MemoryRegion& region = const_cast<MemoryRegion&>(pair.second);
        if (region.isValid && region.contains(ptr)) {
            return &region;
        }
    }
    return nullptr;
}

// 检查数组边界
void MemorySafetyManager::checkArrayBounds(void* array, size_t index, size_t size, const std::string& debugInfo) {
    if (!array) {
        throw ArrayBoundsException("Null array pointer: " + debugInfo);
    }
    
    if (index >= size) {
        std::string message = "Index " + std::to_string(index) + 
                             " out of bounds for array of size " + std::to_string(size);
        if (!debugInfo.empty()) {
            message += ": " + debugInfo;
        }
        throw ArrayBoundsException(message);
    }
    
    // 在严格模式下，还要检查内存区域的有效性
    if (strictMode.load()) {
        if (!isMemoryRegionValid(array)) {
            throw UseAfterFreeException("Accessing deallocated array: " + debugInfo);
        }
    }
}

// 检查悬空指针
void MemorySafetyManager::checkDanglingPointer(void* ptr, const std::string& debugInfo) {
    if (!ptr) {
        throw DanglingPointerException("Null pointer access: " + debugInfo);
    }
    
    // 检查指针是否在跟踪列表中且有效
    std::lock_guard<std::mutex> lock(safetyMutex);
    auto it = trackedPointers.find(ptr);
    if (it != trackedPointers.end()) {
        auto validity = it->second.lock();
        if (!validity || !validity->load()) {
            throw DanglingPointerException("Accessing deallocated memory: " + debugInfo);
        }
    } else if (strictMode.load()) {
        // 在严格模式下，所有指针都应该被跟踪
        if (!isMemoryRegionValid(ptr)) {
            throw DanglingPointerException("Accessing untracked or invalid memory: " + debugInfo);
        }
    }
}

// 获取泄漏的内存区域
std::vector<MemoryRegion> MemorySafetyManager::getLeakedRegions() const {
    std::lock_guard<std::mutex> lock(safetyMutex);
    std::vector<MemoryRegion> leakedRegions;
    
    for (const auto& pair : memoryRegions) {
        const MemoryRegion& region = pair.second;
        if (region.isValid) {
            // 检查是否有对应的跟踪指针
            bool hasValidPointer = false;
            auto it = trackedPointers.find(region.start);
            if (it != trackedPointers.end()) {
                auto validity = it->second.lock();
                hasValidPointer = validity && validity->load();
            }
            
            if (!hasValidPointer) {
                leakedRegions.push_back(region);
            }
        }
    }
    
    return leakedRegions;
}

// 打印内存报告
void MemorySafetyManager::printMemoryReport() const {
    std::lock_guard<std::mutex> lock(safetyMutex);
    
    std::cout << "\n=== Memory Safety Report ===" << std::endl;
    std::cout << "Tracked pointers: " << trackedPointers.size() << std::endl;
    std::cout << "Memory regions: " << memoryRegions.size() << std::endl;
    std::cout << "Strict mode: " << (strictMode.load() ? "enabled" : "disabled") << std::endl;
    std::cout << "Debug mode: " << (debugMode.load() ? "enabled" : "disabled") << std::endl;
    
    // 统计有效和无效的指针
    size_t validPointers = 0;
    size_t invalidPointers = 0;
    
    for (const auto& pair : trackedPointers) {
        auto validity = pair.second.lock();
        if (validity && validity->load()) {
            validPointers++;
        } else {
            invalidPointers++;
        }
    }
    
    std::cout << "Valid pointers: " << validPointers << std::endl;
    std::cout << "Invalid pointers: " << invalidPointers << std::endl;
    
    // 统计有效和无效的内存区域
    size_t validRegions = 0;
    size_t invalidRegions = 0;
    
    for (const auto& pair : memoryRegions) {
        if (pair.second.isValid) {
            validRegions++;
        } else {
            invalidRegions++;
        }
    }
    
    std::cout << "Valid memory regions: " << validRegions << std::endl;
    std::cout << "Invalid memory regions: " << invalidRegions << std::endl;
    
    // 检查内存泄漏
    auto leakedRegions = getLeakedRegions();
    if (!leakedRegions.empty()) {
        std::cout << "\n*** POTENTIAL MEMORY LEAKS ***" << std::endl;
        for (const auto& region : leakedRegions) {
            std::cout << "Leaked region: " << region.start 
                      << " (size: " << region.size 
                      << ", info: " << region.debugInfo << ")" << std::endl;
        }
    } else {
        std::cout << "\nNo memory leaks detected." << std::endl;
    }
    
    // 详细的指针信息（仅在调试模式下）
    if (debugMode.load()) {
        std::cout << "\n--- Detailed Pointer Information ---" << std::endl;
        for (const auto& pair : trackedPointers) {
            auto validity = pair.second.lock();
            std::cout << "Pointer " << pair.first << ": " 
                      << (validity && validity->load() ? "valid" : "invalid") << std::endl;
        }
        
        std::cout << "\n--- Detailed Memory Region Information ---" << std::endl;
        for (const auto& pair : memoryRegions) {
            const MemoryRegion& region = pair.second;
            std::cout << "Region " << region.start 
                      << " (size: " << region.size 
                      << ", valid: " << (region.isValid ? "yes" : "no")
                      << ", info: " << region.debugInfo << ")" << std::endl;
        }
    }
    
    std::cout << "============================\n" << std::endl;
}

// 获取跟踪的指针数量
size_t MemorySafetyManager::getTrackedPointersCount() const {
    std::lock_guard<std::mutex> lock(safetyMutex);
    return trackedPointers.size();
}

// 获取跟踪的内存区域数量
size_t MemorySafetyManager::getTrackedRegionsCount() const {
    std::lock_guard<std::mutex> lock(safetyMutex);
    return memoryRegions.size();
}

} // namespace miniswift