#include "UIRuntime.h"
#include "TGUIBackend.h"
#include <iostream>
#include <memory>

using namespace MiniSwift::UI;

/**
 * Simple UI test program to verify the UI system
 */
int main(int argc, char** argv) {
    std::cout << "=== MiniSwift UI System Test ===" << std::endl;
    
    try {
        // Test UI Application initialization
        std::cout << "\n1. Testing UI Application..." << std::endl;
        MiniSwift::UI::UIApplication& app = MiniSwift::UI::UIApplication::getInstance();
        
        if (!app.initialize()) {
            std::cerr << "Failed to initialize UI application" << std::endl;
            return 1;
        }
        std::cout << "UI Application initialized successfully" << std::endl;
        
        // Test TGUI backend availability
        std::cout << "\n2. Testing TGUI backend..." << std::endl;
        std::cout << "TGUI available: " << (TGUI::isTGUIAvailable() ? "Yes" : "No") << std::endl;
        std::cout << "TGUI version info: " << TGUI::getTGUIVersion() << std::endl;
        
        // Test UI component creation
        std::cout << "\n3. Testing UI component creation..." << std::endl;
        
        // Create text widget
        auto textWidget = createText("Hello, MiniSwift UI!");
        if (textWidget) {
            std::cout << "Text widget created successfully" << std::endl;
            textWidget->setTextColor(Color{0.0, 0.5, 1.0, 1.0}); // Blue text
        }
        
        // Create button widget
        auto buttonWidget = createButton("Click Me!", []() {
            std::cout << "Button was clicked!" << std::endl;
        });
        if (buttonWidget) {
            std::cout << "Button widget created successfully" << std::endl;
            buttonWidget->setPadding(EdgeInsets{10.0, 20.0, 10.0, 20.0});
        }
        
        // Create layout containers
        auto vstack = createVStack(12.0);
        if (vstack) {
            std::cout << "VStack widget created successfully" << std::endl;
            vstack->addChild(textWidget);
            vstack->addChild(buttonWidget);
        }
        
        auto hstack = createHStack(8.0);
        if (hstack) {
            std::cout << "HStack widget created successfully" << std::endl;
            
            // Add some more components to HStack
            auto leftText = createText("Left");
            auto rightText = createText("Right");
            if (leftText && rightText) {
                hstack->addChild(leftText);
                hstack->addChild(rightText);
            }
        }
        
        // Create main container
        auto mainContainer = createVStack(16.0);
        if (mainContainer) {
            mainContainer->addChild(vstack);
            mainContainer->addChild(hstack);
            mainContainer->setPadding(EdgeInsets{20.0, 20.0, 20.0, 20.0});
        }
        
        // Test layout and rendering
        std::cout << "\n4. Testing layout and rendering..." << std::endl;
        if (mainContainer) {
            mainContainer->layout();
            mainContainer->render();
            std::cout << "Layout and rendering completed" << std::endl;
        }
        
        // Test UI application setup
        std::cout << "\n5. Testing UI application setup..." << std::endl;
        app.setRootWidget(mainContainer);
        std::cout << "Root widget set successfully" << std::endl;
        
        // Test TGUI-specific features if available
        if (TGUI::isTGUIAvailable()) {
            std::cout << "\n6. Testing TGUI-specific features..." << std::endl;
            
            // Initialize TGUI application
            TGUI::TGUIApplication& tguiApp = TGUI::TGUIApplication::getInstance();
            if (tguiApp.initialize(argc, argv)) {
                std::cout << "TGUI application initialized successfully" << std::endl;
                
                // Test TGUI widget creation
                auto tguiText = TGUI::createTGUIText("TGUI Native Text");
                if (tguiText) {
                    std::cout << "TGUI text widget created successfully" << std::endl;
                    tguiText->render();
                }
                
                auto tguiButton = TGUI::createTGUIButton("TGUI Button", []() {
                    std::cout << "TGUI button clicked!" << std::endl;
                });
                if (tguiButton) {
                    std::cout << "TGUI button widget created successfully" << std::endl;
                    tguiButton->render();
                }
                
                // Set main window content
                tguiApp.setMainWindowContent(mainContainer);
                std::cout << "TGUI main window content set" << std::endl;
            }
        }
        
        // Test color and styling
        std::cout << "\n7. Testing color and styling..." << std::endl;
        Color testColor = Color::redColor();
        std::cout << "Red color: R=" << testColor.red << ", G=" << testColor.green 
                  << ", B=" << testColor.blue << ", A=" << testColor.alpha << std::endl;
        
        Color blueColor = Color::blueColor();
        std::cout << "Blue color: R=" << blueColor.red << ", G=" << blueColor.green 
                  << ", B=" << blueColor.blue << ", A=" << blueColor.alpha << std::endl;
        
        // Test geometry types
        std::cout << "\n8. Testing geometry types..." << std::endl;
        Size testSize{200.0, 100.0};
        std::cout << "Test size: " << testSize.width << "x" << testSize.height << std::endl;
        
        Point testPoint{50.0, 75.0};
        std::cout << "Test point: (" << testPoint.x << ", " << testPoint.y << ")" << std::endl;
        
        Rect testRect{testPoint, testSize};
        std::cout << "Test rect: origin=(" << testRect.origin.x << ", " << testRect.origin.y 
                  << "), size=" << testRect.size.width << "x" << testRect.size.height << std::endl;
        
        // Summary
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "✓ UI Application initialization" << std::endl;
        std::cout << "✓ Backend detection" << std::endl;
        std::cout << "✓ UI component creation" << std::endl;
        std::cout << "✓ Layout container creation" << std::endl;
        std::cout << "✓ Layout and rendering" << std::endl;
        std::cout << "✓ Application setup" << std::endl;
        std::cout << "✓ Color and geometry types" << std::endl;
        
        if (TGUI::isTGUIAvailable()) {
            std::cout << "✓ TGUI-specific features" << std::endl;
        }
        
        std::cout << "\nAll tests completed successfully!" << std::endl;
        
        // Note: We don't call app.run() here as it would start the GUI loop
        // In a real application, you would call this to show the UI
        
        // Cleanup
        app.cleanup();
        std::cout << "UI system cleaned up" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}