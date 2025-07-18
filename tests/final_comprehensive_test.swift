// MiniSwift 最终综合功能测试
print("=== MiniSwift 综合功能测试 ===");

// 1. 基本数据类型和转义字符
var greeting = "Hello\nMiniSwift!";
print(greeting);

// 2. 数组功能
var numbers = [1, 2, 3, 4, 5];
print("数组:");
print(numbers);
print("第一个元素:");
print(numbers[0]);
print("最后一个元素:");
print(numbers[4]);

// 3. 字典功能
var person = ["name": "张三", "age": "25", "city": "北京"];
print("\n字典:");
print(person);
print("姓名:");
print(person["name"]);
print("年龄:");
print(person["age"]);

// 4. 嵌套数组
var matrix = [[1, 2], [3, 4], [5, 6]];
print("\n嵌套数组:");
print(matrix);
print("第一行:");
print(matrix[0]);
print("第二行第二列:");
print(matrix[1][1]);

// 5. 字符串数组与转义字符
var messages = ["第一条\t消息", "第二条\n消息", "第三条\"重要\"消息"];
print("\n字符串数组:");
print("消息 1:");
print(messages[0]);
print("消息 2:");
print(messages[1]);
print("消息 3:");
print(messages[2]);

// 6. 复杂数据结构
var config = ["database": "mysql://localhost:3306", "log_path": "C:\\logs\\app.log", "debug": "true"];
print("\n配置信息:");
print("数据库:");
print(config["database"]);
print("日志路径:");
print(config["log_path"]);
print("调试模式:");
print(config["debug"]);

// 7. 类型注解测试
var typed_array: [Int] = [10, 20, 30];
var typed_dict: [String: String] = ["key1": "value1", "key2": "value2"];
print("\n类型注解测试:");
print("整数数组:");
print(typed_array);
print("字符串字典:");
print(typed_dict);

print("\n=== 所有测试完成！===");