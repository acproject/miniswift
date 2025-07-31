#include "UIRuntime.h"
#include "../interpreter/Concurrency/ConcurrencyRuntime.h"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <future>

namespace MiniSwift {
    namespace UI {
        
        // TextWidget implementation
        void TextWidget::render() {
            // Platform-specific text rendering will be implemented here
            // For now, just output to console for debugging
            std::cout << "[TextWidget] Rendering text: \"" << text_ << "\"" << std::endl;
        }
        
        void TextWidget::layout() {
            // Calculate text size based on font and content
            // This is a simplified implementation
            double textWidth = text_.length() * font_.size * 0.6; // Rough estimation
            double textHeight = font_.size * 1.2;
            
            Size contentSize(textWidth, textHeight);
            Size totalSize(contentSize.width + padding_.left + padding_.right,
                          contentSize.height + padding_.top + padding_.bottom);
            
            frame_.size = totalSize;
        }
        
        // ButtonWidget implementation
        void ButtonWidget::render() {
            std::cout << "[ButtonWidget] Rendering button: \"" << title_ << "\"" << std::endl;
        }
        
        void ButtonWidget::layout() {
            // Calculate button size based on title and padding
            double buttonWidth = title_.length() * 14.0 * 0.6 + 20; // Text + padding
            double buttonHeight = 44.0; // Standard button height
            
            Size contentSize(buttonWidth, buttonHeight);
            Size totalSize(contentSize.width + padding_.left + padding_.right,
                          contentSize.height + padding_.top + padding_.bottom);
            
            frame_.size = totalSize;
        }
        
        // VStackWidget implementation
        void VStackWidget::render() {
            std::cout << "[VStackWidget] Rendering VStack with " << children_.size() << " children" << std::endl;
            
            // Render all children
            for (auto& child : children_) {
                if (child) {
                    child->render();
                }
            }
        }
        
        void VStackWidget::layout() {
            if (children_.empty()) {
                frame_.size = Size(0, 0);
                return;
            }
            
            // Layout children vertically
            double totalHeight = 0;
            double maxWidth = 0;
            double currentY = padding_.top;
            
            for (size_t i = 0; i < children_.size(); ++i) {
                auto& child = children_[i];
                if (!child) continue;
                
                // Layout child first
                child->layout();
                
                // Position child
                Rect childFrame = child->getFrame();
                childFrame.origin.x = padding_.left;
                childFrame.origin.y = currentY;
                child->setFrame(childFrame);
                
                // Update totals
                maxWidth = std::max(maxWidth, childFrame.size.width);
                currentY += childFrame.size.height;
                
                // Add spacing (except for last child)
                if (i < children_.size() - 1) {
                    currentY += spacing_;
                }
            }
            
            totalHeight = currentY + padding_.bottom;
            
            // Set our frame size
            frame_.size = Size(maxWidth + padding_.left + padding_.right, totalHeight);
        }
        
        // HStackWidget implementation
        void HStackWidget::render() {
            std::cout << "[HStackWidget] Rendering HStack with " << children_.size() << " children" << std::endl;
            
            // Render all children
            for (auto& child : children_) {
                if (child) {
                    child->render();
                }
            }
        }
        
        void HStackWidget::layout() {
            if (children_.empty()) {
                frame_.size = Size(0, 0);
                return;
            }
            
            // Layout children horizontally
            double totalWidth = 0;
            double maxHeight = 0;
            double currentX = padding_.left;
            
            for (size_t i = 0; i < children_.size(); ++i) {
                auto& child = children_[i];
                if (!child) continue;
                
                // Layout child first
                child->layout();
                
                // Position child
                Rect childFrame = child->getFrame();
                childFrame.origin.x = currentX;
                childFrame.origin.y = padding_.top;
                child->setFrame(childFrame);
                
                // Update totals
                maxHeight = std::max(maxHeight, childFrame.size.height);
                currentX += childFrame.size.width;
                
                // Add spacing (except for last child)
                if (i < children_.size() - 1) {
                    currentX += spacing_;
                }
            }
            
            totalWidth = currentX + padding_.right;
            
            // Set our frame size
            frame_.size = Size(totalWidth, maxHeight + padding_.top + padding_.bottom);
        }
        
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
        std::shared_ptr<TextWidget> createText(const std::string& text) {
            return std::make_shared<TextWidget>(text);
        }
        
        std::shared_ptr<ButtonWidget> createButton(const std::string& title, UICallback callback) {
            return std::make_shared<ButtonWidget>(title, callback);
        }
        
        std::shared_ptr<VStackWidget> createVStack(double spacing) {
            return std::make_shared<VStackWidget>(spacing);
        }
        
        std::shared_ptr<HStackWidget> createHStack(double spacing) {
            return std::make_shared<HStackWidget>(spacing);
        }
        
    } // namespace UI
} // namespace MiniSwift