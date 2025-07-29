import MiniSwift

// 从网络获取新闻的测试
print("开始获取今日新闻...")

// 使用一个提供新闻API的服务
// 这里使用JSONPlaceholder作为示例API
let newsUrl = "https://jsonplaceholder.typicode.com/posts/1"

print("正在从以下URL获取新闻: " + newsUrl)

// 发送HTTP GET请求获取新闻
let newsResponse = httpGet(newsUrl)

print("新闻获取完成!")
print("响应内容: " + newsResponse)

// 将新闻保存到文件
let filename = "today_news.txt"
print("正在将新闻保存到文件: " + filename)

writeFile(filename, newsResponse)

print("新闻已成功保存到 " + filename)
print("测试完成!")