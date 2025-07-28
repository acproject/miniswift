func processUserAsync(userId: Int) async -> String {
    return "test"
}

@main
func main() {
    let result = await processUserAsync(userId: 42)
    print(result)
}

main()