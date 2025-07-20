struct Matrix {
    var data: Int = 0
    var rows: Int = 2
    var cols: Int = 2
    
    subscript(row: Int, col: Int) -> Int {
        get {
            return data + row * cols + col
        }
        set {
            data = newValue - row * cols - col
        }
    }
}

var matrix = Matrix()
print("Initial matrix[0,0]:")
print(matrix[0, 0])

print("Setting matrix[1,1] = 15")
matrix[1, 1] = 15

print("After setting, matrix[1,1]:")
print(matrix[1, 1])

print("Matrix data value:")
print(matrix.data)