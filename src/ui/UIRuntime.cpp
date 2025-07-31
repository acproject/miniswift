#include "UIRuntime.h"
#include "TextWidget.h"
#include "ButtonWidget.h"
#include "VStackWidget.h"
#include "HStackWidget.h"
#include "../interpreter/Concurrency/ConcurrencyRuntime.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <future>

namespace MiniSwift {
    namespace UI {
        
        // Widget implementations moved to separate files
        
        // VStackWidget implementation moved to VStackWidget.cpp
        
        // HStackWidget implementation moved to HStackWidget.cpp
        
        // UIApplication implementation
        UIApplication& UIApplication::getInstance() {
            static UIApplication instance;
            return instance;
        }
        
        void UIApplication::setRootWidget(std::shared_ptr<UIWidget> root) {
            rootWidget_ = root;
        }
        
        bool UIApplication::initialize() {
            std::cout << "[UIApplication] Initializing UI system..." << std::endl;
            std::cout << "[UIApplication] Thread ID: " << std::this_thread::get_id() << std::endl;
            
            // Initialize concurrency runtime if not already done
            try {
                miniswift::initialize_concurrency_runtime();
                std::cout << "[UIApplication] Concurrency runtime initialized successfully" << std::endl;
            } catch (const std::exception& e) {
                std::cout << "[UIApplication] Warning: Concurrency runtime already initialized or failed: " << e.what() << std::endl;
            }
            
            // Add a small delay to simulate initialization work
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            std::cout << "[UIApplication] UI system initialization completed" << std::endl;
            return true;
        }
        
        void UIApplication::run() {
            std::cout << "[UIApplication] Starting run() method..." << std::endl;
            std::cout << "[UIApplication] Main thread ID: " << std::this_thread::get_id() << std::endl;
            
            if (!initialize()) {
                std::cerr << "[UIApplication] Failed to initialize UI system" << std::endl;
                return;
            }
            
            if (!rootWidget_) {
                std::cerr << "[UIApplication] No root widget set" << std::endl;
                return;
            }
            
            std::cout << "[UIApplication] Starting UI application with multithreading support..." << std::endl;
            running_ = true;
            
            // Use async task for UI layout and rendering
            try {
                auto& runtime = miniswift::ConcurrencyRuntime::get_instance();
                
                // Layout task
                std::cout << "[UIApplication] Submitting layout task..." << std::endl;
                auto layout_future = runtime.async(miniswift::TaskPriority::UserInteractive, [this]() {
                    std::cout << "[UIApplication] Layout task running on thread: " << std::this_thread::get_id() << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate layout work
                    rootWidget_->layout();
                    std::cout << "[UIApplication] Layout task completed" << std::endl;
                });
                
                // Wait for layout to complete
                std::cout << "[UIApplication] Waiting for layout task..." << std::endl;
                layout_future.wait();
                std::cout << "[UIApplication] Layout task finished" << std::endl;
                
                // Render task
                std::cout << "[UIApplication] Submitting render task..." << std::endl;
                auto render_future = runtime.async(miniswift::TaskPriority::UserInteractive, [this]() {
                    std::cout << "[UIApplication] Render task running on thread: " << std::this_thread::get_id() << std::endl;
                    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate render work
                    rootWidget_->render();
                    std::cout << "[UIApplication] Render task completed" << std::endl;
                });
                
                // Wait for render to complete
                std::cout << "[UIApplication] Waiting for render task..." << std::endl;
                render_future.wait();
                std::cout << "[UIApplication] Render task finished" << std::endl;
                
            } catch (const std::exception& e) {
                std::cerr << "[UIApplication] Error during async UI operations: " << e.what() << std::endl;
                // Fallback to synchronous execution
                std::cout << "[UIApplication] Falling back to synchronous execution..." << std::endl;
                rootWidget_->layout();
                rootWidget_->render();
            }
            
            std::cout << "[UIApplication] UI application started successfully" << std::endl;
            std::cout << "[UIApplication] Root widget frame: " 
                      << rootWidget_->getFrame().size.width << "x" 
                      << rootWidget_->getFrame().size.height << std::endl;
            
            // Simulate event loop with timeout
            std::cout << "[UIApplication] Starting event loop simulation..." << std::endl;
            auto start_time = std::chrono::steady_clock::now();
            while (running_) {
                auto current_time = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time);
                
                if (elapsed.count() >= 2) { // Run for 2 seconds
                    std::cout << "[UIApplication] Event loop timeout reached, stopping..." << std::endl;
                    break;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::cout << "[UIApplication] Event loop tick (" << elapsed.count() << "s)" << std::endl;
            }
            
            std::cout << "[UIApplication] Event loop completed" << std::endl;
        }
        
        void UIApplication::quit() {
            std::cout << "[UIApplication] Quitting UI application..." << std::endl;
            running_ = false;
            cleanup();
        }
        
        void UIApplication::cleanup() {
            std::cout << "[UIApplication] Cleaning up UI system..." << std::endl;
            rootWidget_.reset();
        }
        
        // Factory functions
        // Factory functions moved to respective widget files
        
    } // namespace UI
} // namespace MiniSwift