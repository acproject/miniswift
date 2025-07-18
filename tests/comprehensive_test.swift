// 综合测试 MiniSwift 集合类型功能

// 1. 基本数组操作
var numbers: [Int] = [1, 2, 3, 4, 5];
print(numbers);
print(numbers[0]);
print(numbers[4]);

// 2. 字典操作
var person: [String: String] = ["name": "Alice", "age": "30", "city": "Beijing"];
print(person);
print(person["name"]);
print(person["age"]);

// 3. 嵌套数组
var matrix: [[Int]] = [[1, 2, 3], [4, 5, 6], [7, 8, 9]];
print(matrix);
print(matrix[0]);
print(matrix[1][2]);
print(matrix[2][0]);

// 4. 混合类型数组（字符串）
var fruits: [String] = ["apple", "banana", "orange"];
print(fruits);
print(fruits[1]);

// 5. 空数组
var emptyArray: [Int] = [];
print(emptyArray);