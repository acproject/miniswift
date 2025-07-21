func testGeneric<T>(value: T) -> T {
    let temporaryA = value
    return temporaryA
}

let result = testGeneric(value: 42)
print(result)