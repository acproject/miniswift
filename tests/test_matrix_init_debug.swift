struct Matrix {
    let rows: Int
    let columns: Int
    var grid: [Double]
    init(rows: Int, columns: Int) {
        self.rows = rows
        self.columns = columns
        grid = [Double]()
    }
}

var matrix = Matrix(rows: 2, columns: 2)
print("Matrix created")