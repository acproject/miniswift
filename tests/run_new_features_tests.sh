#!/bin/bash

# Test script for new features
# This script runs all the new feature tests

echo "Running New Features Tests for MiniSwift"
echo "========================================="
echo

# Build the project first
echo "Building MiniSwift..."
cd ../build
make
if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi
echo "✅ Build successful!"
echo

# Go back to tests directory
cd ../tests

# Test files to run
test_files=(
    "basic_new_features_test.swift"
    "result_builder_test.swift"
    "custom_operator_test.swift"
    "operator_precedence_test.swift"
    "advanced_features_test.swift"
)

# Run each test
for test_file in "${test_files[@]}"; do
    echo "Running $test_file..."
    echo "------------------------"
    
    # Run the test
    ../build/miniswift "$test_file"
    
    if [ $? -eq 0 ]; then
        echo "✅ $test_file passed!"
    else
        echo "❌ $test_file failed!"
    fi
    
    echo
    echo "Press Enter to continue to next test..."
    read
done

echo "All new features tests completed!"
echo "=================================="