// Simple Button Test - 测试GTK4窗口是否能正常显示
// 这个测试只包含一个按钮，用于验证GTK4基本功能

class SimpleButtonApp {
    func run() {
        print("Starting Simple Button Test...")
        
        // 创建一个简单的按钮
        let testButton = Button("Click Me!")
            .background(Color.blue)
            .foregroundColor(Color.white)
            .padding()
        
        print("Button created successfully")
        
        // 创建主视图 - 只包含一个按钮
        let mainView = VStack()
        mainView.addChild(testButton)
        
        print("Main view created with button")
        
        // 设置应用程序
        UIApplication.shared.setRootView(mainView)
        print("Root view set")
        
        // 运行应用程序
        print("Starting UI application...")
        UIApplication.shared.run()
        print("UI application finished")
    }
}

// 应用程序入口点
print("=== Simple Button Test Start ===")

@main
func main() {
    let app = SimpleButtonApp()
    app.run()
}

main()

print("=== Simple Button Test End ===")