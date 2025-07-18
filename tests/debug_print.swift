print("Starting test")
let greet = { () -> Void in
    print("Inside closure")
}
print("Closure created")
print(greet)
print("About to call closure")