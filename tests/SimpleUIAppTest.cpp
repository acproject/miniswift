#include <iostream>
#include <memory>
#include "../src/ui/UIIntegration.h"
#include "../src/ui/TGUIBackend.h"

// Simple UI Application Test in C++
// Testing complete UI app with UIIntegration API

int main() {
    std::cout << "Starting C++ UI Application Test..." << std::endl;
    
    try {
        // Initialize UI system with TGUI backend
        auto& uiIntegration = MiniSwift::UI::UIIntegration::getInstance();
        uiIntegration.setBackend(MiniSwift::UI::UIIntegration::Backend::TGUI);
        if (!uiIntegration.initialize()) {
            std::cerr << "Failed to initialize UI system" << std::endl;
            return 1;
        }
        
        // Create UI components using TGUI factory functions
        using namespace MiniSwift::UI::TGUI;
        
        auto titleText = createTGUIText("Welcome to MiniSwift UI!");
        auto subtitleText = createTGUIText("A SwiftUI-like framework for MiniSwift");
        
        auto clickButton = createTGUIButton("Click Me");
        auto secondButton = createTGUIButton("Another Button");
        
        // Create feature texts
        auto feature1 = createTGUIText("✓ Cross-platform UI");
        auto feature2 = createTGUIText("✓ TGUI backend");
        auto feature3 = createTGUIText("✓ SwiftUI-like syntax");
        auto footerText = createTGUIText("Built with MiniSwift");
        
        // Create layout containers
        auto buttonStack = createTGUIHStack(10); // spacing: 10
        auto featureList = createTGUIVStack(5);  // spacing: 5
        auto mainStack = createTGUIVStack(20);   // spacing: 20
        
        // Add components to containers
        uiIntegration.addChildToWidget(buttonStack, clickButton);
        uiIntegration.addChildToWidget(buttonStack, secondButton);
        
        uiIntegration.addChildToWidget(featureList, feature1);
        uiIntegration.addChildToWidget(featureList, feature2);
        uiIntegration.addChildToWidget(featureList, feature3);
        
        uiIntegration.addChildToWidget(mainStack, titleText);
        uiIntegration.addChildToWidget(mainStack, subtitleText);
        uiIntegration.addChildToWidget(mainStack, buttonStack);
        uiIntegration.addChildToWidget(mainStack, featureList);
        uiIntegration.addChildToWidget(mainStack, footerText);
        
        std::cout << "UI components created successfully" << std::endl;
        
        // Set the main view and run the application
        uiIntegration.setMainView(mainStack);
        
        std::cout << "Starting UI application..." << std::endl;
        
        // Run the UI application
        uiIntegration.runUIApplication();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Application finished successfully" << std::endl;
    return 0;
}