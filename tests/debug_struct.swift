struct Person {
    var name: String
    var age: Int
}

var person: Person? = Person(name: "Alice", age: 30)
var nilPerson: Person? = nil

print(person?.name)
print(nilPerson?.name)

var numbers: [Int]? = [1, 2, 3, 4, 5]
var nilArray: [Int]? = nil

print(numbers?[0])