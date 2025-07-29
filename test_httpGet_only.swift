import MiniSwift

print("测试httpGet函数...")

// 测试httpGet函数
let url = "https://jsonplaceholder.typicode.com/posts/1"

print("正在发送HTTP GET请求到: " + url)
let response = httpGet(url)
print("HTTP请求完成!")
print("响应: " + response)