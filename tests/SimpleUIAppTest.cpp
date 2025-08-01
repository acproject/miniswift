#include <iostream>
#include <memory>
#include "../src/ui/UIIntegration.h"
#include "../src/ui/GTK4Backend.h"

// Simple UI Application Test in C++
// Testing complete UI app with UIIntegration API

int main() {
    std::cout << "Starting C++ UI Application Test..." << std::endl;
    
    try {
        // Initialize UI system
        auto& uiIntegration = MiniSwift::UI::UIIntegration::getInstance();
        if (!uiIntegration.initialize()) {
            std::cerr << "Failed to initialize UI system" << std::endl;
            return 1;
        }
        
        // Create UI components using GTK4 factory functions
        using namespace MiniSwift::UI::GTK4;
        
        auto titleText = createGTK4Text("Welcome to MiniSwift UI!");
        auto subtitleText = createGTK4Text("A SwiftUI-like framework for MiniSwift");
        
        auto clickButton = createGTK4Button("Click Me");
        auto secondButton = createGTK4Button("Another Button");
        
        // Create feature texts
        auto feature1 = createGTK4Text("✓ Cross-platform UI");
        auto feature2 = createGTK4Text("✓ GTK4 backend");
        auto feature3 = createGTK4Text("✓ SwiftUI-like syntax");
        auto footerText = createGTK4Text("Built with MiniSwift");
        
        // Create layout containers
        auto buttonStack = createGTK4HStack(10); // spacing: 10
        auto featureList = createGTK4VStack(5);  // spacing: 5
        auto mainStack = createGTK4VStack(20);   // spacing: 20
        
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