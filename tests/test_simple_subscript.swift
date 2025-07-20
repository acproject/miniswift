struct SimpleMatrix {
    var data: [Double]
    
    init(data: [Double]) {
        self.data = data
    }
    
    subscript(index: Int) -> Double {
        get {
            return data[0]
        }
        set {
            data[0] = newValue
        }
    }
}

var matrix = SimpleMatrix(data: [1.0, 2.0])
var value = matrix[0]
matrix[1] = 5.0