func main() {
    let data = "测试数据"
    print("Data: \(data)")
    
    if data.isEmpty {
        print("Data is empty")
    } else {
        print("Data is not empty")
    }
    
    let emptyData = ""
    if emptyData.isEmpty {
        print("Empty data is empty")
    } else {
        print("Empty data is not empty")
    }
}

main()