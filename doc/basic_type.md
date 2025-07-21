1. 整数类型
有符号整数：

Int：平台相关（64位系统为64位，32位系统为32位）

Int8, Int16, Int32, Int64：明确指定位数（如 Int8 范围：-128 ~ 127）

无符号整数：

UInt：平台相关

UInt8, UInt16, UInt32, UInt64（如 UInt8 范围：0 ~ 255）

```swift
let age: Int = 30
let byte: UInt8 = 0xFF
```

2. 浮点数类型
Float：32位浮点数（精度约6位小数）

Double：64位浮点数（精度约15位小数，默认推断类型）

```swift
let pi: Double = 3.1415926535
let temperature: Float = 23.5
```

3. 布尔类型
Bool：仅 true 或 false

```swift
let isSwiftAwesome: Bool = true
```

4. 字符串与字符
String：文本（如 "Hello"）

Character：单个字符（如 "A"）

```swift
let greeting: String = "Hello, Swift!"
let letter: Character = "A"
```

5. 元组（Tuple）
组合多个值为一个复合值（类型可不同）：
```swift
let person: (String, Int) = ("Alice", 25)
print(person.0) // "Alice"
```

6. 可选类型（Optional）
表示值可能存在（值）或不存在（nil）：

```swift
var optionalInt: Int? = 42  // 可赋值为 nil
```

7. 集合类型
- 数组（Array）：有序集合

```swift
let numbers: [Int] = [1, 2, 3]
```

- 字典（Dictionary）：键值对集合

```swift
let user: [String: Any] = ["name": "Bob", "age": 30]
```

- 集合（Set）：无序唯一值

```swift
let uniqueNumbers: Set<Int> = [1, 2, 2, 3] // 结果为 [1, 2, 3]
```

8. 特殊类型
- Any：可表示任何类型（不推荐滥用）

```swift
var anything: Any = 3.14
anything = "Now a string"
```
- Void：表示无值（等同于空元组 ()）