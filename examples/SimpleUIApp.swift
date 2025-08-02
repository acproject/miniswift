// Minimal GTK4 Layout Test - Swift Version
// Testing VStack and HStack layouts

func createAndRunApp() {
    print("开始测试UI")
    
    // Create UI components
    let titleText = Text("布局测试")
    let descText = Text("测试 VStack 和 HStack 布局")
    
    let button1 = Button("按钮 ☆")
    let button2 = Button("Button 2")
    
    let statusText = Text("状态: Layout components created")
    
    // Create layout containers
    let buttonStack = HStack(spacing: 15) // horizontal stack with 15px spacing
    let mainStack = VStack(spacing: 20)   // vertical stack with 20px spacing
    
    // Add buttons to horizontal stack
    buttonStack.addChild(button1)
    buttonStack.addChild(button2)
    
    // Add all components to main vertical stack
    mainStack.addChild(titleText)
    mainStack.addChild(descText)
    mainStack.addChild(buttonStack)
    mainStack.addChild(statusText)
    
    print("布局组件创建成功！")
    
    // Set the main view and run the application
    print("应用启动")
    UIApplication.shared.setRootView(mainStack)
    UIApplication.shared.run()
    
    print("应用完成结束")
}

// Run the app
createAndRunApp()