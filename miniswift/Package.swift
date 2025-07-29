// swift-tools-version:5.5
import PackageDescription

let package = Package(
    name: "MiniSwift",
    products: [
        .library(
            name: "MiniSwift",
            targets: ["MiniSwift"]
        ),
    ],
    targets: [
        .target(
            name: "MiniSwift",
            dependencies: []
        ),
    ]
)