// 测试UI扩展功能的简单示例

// 创建基本UI组件
let textWidget = Text("Hello, MiniSwift UI!")
let buttonWidget = Button("Click Me")

// 创建布局容器
let vstack = VStack {
    textWidget
    buttonWidget
}

// 应用修饰符
let styledText = textWidget
    .font(18)
    .foregroundColor("blue")
    .padding(10)

let styledButton = buttonWidget
    .background("red")
    .cornerRadius(8)
    .padding(5)

// 创建完整的UI结构
let mainView = VStack {
    styledText
    styledButton
    HStack {
        Text("Left")
        Spacer()
        Text("Right")
    }
}

// 显示UI
UIApplication.shared.setRootView(mainView)