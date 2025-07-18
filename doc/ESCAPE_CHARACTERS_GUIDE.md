# MiniSwift 转义字符支持指南

## 概述

MiniSwift 现在完全支持字符串中的转义字符，使得字符串处理更加灵活和强大。

## 支持的转义字符

| 转义序列 | 描述 | 示例 |
|---------|------|------|
| `\n` | 换行符 | `"第一行\n第二行"` |
| `\t` | 制表符 | `"姓名:\t张三"` |
| `\r` | 回车符 | `"行1\r行2"` |
| `\\` | 反斜杠 | `"路径: C:\\Users"` |
| `\"` | 双引号 | `"他说:\"你好\""` |
| `\0` | 空字符 | `"字符串\0结束"` |

## 使用示例

### 基本转义字符

```swift
// 换行符
var message = "Hello\nWorld";
print(message);
// 输出:
// Hello
// World

// 制表符
var formatted = "姓名:\t张三\n年龄:\t25";
print(formatted);
// 输出:
// 姓名:    张三
// 年龄:    25

// 引号
var quote = "她说:\"今天天气真好！\"";
print(quote);
// 输出: 她说:"今天天气真好！"
```

### 文件路径处理

```swift
// Windows 路径
var winPath = "C:\\Program Files\\MyApp\\config.txt";
print(winPath);
// 输出: C:\Program Files\MyApp\config.txt

// Unix 路径（不需要转义）
var unixPath = "/home/user/documents/file.txt";
print(unixPath);
// 输出: /home/user/documents/file.txt
```

### 复杂字符串格式化

```swift
// 多行文本格式化
var report = "=== 报告 ===\n\t标题: 月度总结\n\t作者: \"张三\"\n\t路径: D:\\reports\\monthly.txt\n=== 结束 ===";
print(report);
// 输出:
// === 报告 ===
//     标题: 月度总结
//     作者: "张三"
//     路径: D:\reports\monthly.txt
// === 结束 ===
```

## 交互模式支持

转义字符在交互模式（REPL）中同样有效：

```
> print("Hello\nWorld");
Hello
World
> var tab_test = "Name:\tAlice";
> print(tab_test);
Name:   Alice
```

## 技术实现

转义字符的处理在词法分析器（Lexer）的 `stringLiteral()` 函数中实现：

- 检测到反斜杠 `\` 时，读取下一个字符
- 根据转义字符类型进行相应的转换
- 未知的转义序列会保留原始的反斜杠

## 注意事项

1. **未知转义序列**: 如果使用了不支持的转义序列（如 `\x`），反斜杠会被保留
2. **字符串插值**: 转义字符与字符串插值 `\(expression)` 兼容
3. **Unicode 支持**: 支持中文和其他 Unicode 字符与转义字符的混合使用

## 测试文件

项目中包含以下测试文件来验证转义字符功能：

- `escape_test.swift` - 基本转义字符测试
- `interactive_escape_test.swift` - 交互模式转义字符测试
- `test_interactive_escape.sh` - 自动化交互模式测试脚本

运行测试：

```bash
# 基本测试
./build/miniswift escape_test.swift

# 交互模式测试
./build/miniswift interactive_escape_test.swift

# 自动化交互测试
./test_interactive_escape.sh
```

这些改进使得 MiniSwift 在字符串处理方面更加完善和实用。