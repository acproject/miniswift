struct Matrix {
    let rows: Int
    let columns: Int
    var grid: [Double]
    func indexIsValid(row: Int, column: Int) -> Bool {
        return row >= 0 && column >= 0
    }
}

var matrix = Matrix(rows: 2, columns: 2)
print("Matrix created")