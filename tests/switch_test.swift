// Switch 语句测试
print("=== Switch 语句测试 ===")

// 测试整数 switch
let number = 2
switch number {
case 1:
    print("数字是 1")
case 2:
    print("数字是 2")
case 3:
    print("数字是 3")
default:
    print("数字是其他值")
}

// 测试字符串 switch
let fruit = "apple"
switch fruit {
case "apple":
    print("这是苹果")
case "banana":
    print("这是香蕉")
case "orange":
    print("这是橙子")
default:
    print("未知水果")
}

// 测试布尔值 switch
let isTrue = true
switch isTrue {
case true:
    print("值为真")
case false:
    print("值为假")
}

print("Switch 测试完成")