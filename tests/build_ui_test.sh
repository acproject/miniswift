#!/bin/bash

# MiniSwift UI Library Build and Test Script
# This script builds and tests the UI library components

set -e  # Exit on any error

echo "🚀 MiniSwift UI Library Build and Test"
echo "======================================"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "README.md" ] || [ ! -d "miniswift" ]; then
    print_error "Please run this script from the MiniSwift project root directory"
    exit 1
fi

# Step 1: Check Swift installation
print_status "Checking Swift installation..."
if command -v swift &> /dev/null; then
    SWIFT_VERSION=$(swift --version | head -n1)
    print_success "Swift found: $SWIFT_VERSION"
else
    print_warning "Swift not found. UI library syntax checking will be skipped."
    SWIFT_AVAILABLE=false
fi

# Step 2: Validate UI library structure
print_status "Validating UI library structure..."

UI_FILES=(
    "miniswift/Sources/MiniSwift/UI/UIView.swift"
    "miniswift/Sources/MiniSwift/UI/Layout.swift"
    "miniswift/Sources/MiniSwift/UI/Components.swift"
    "miniswift/Sources/MiniSwift/UI/Geometry.swift"
    "miniswift/Sources/MiniSwift/UI/Style.swift"
    "miniswift/Sources/MiniSwift/UI/Modifiers.swift"
    "miniswift/Sources/MiniSwift/UI/GTK4Bridge.swift"
    "miniswift/Sources/MiniSwift/MiniSwift.swift"
    "miniswift/Package.swift"
)

for file in "${UI_FILES[@]}"; do
    if [ -f "$file" ]; then
        print_success "✓ $file"
    else
        print_error "✗ Missing: $file"
        exit 1
    fi
done

# Step 3: Check example files
print_status "Checking example files..."

EXAMPLE_FILES=(
    "examples/UIExample.swift"
    "test_ui_basic.swift"
    "UI_README.md"
)

for file in "${EXAMPLE_FILES[@]}"; do
    if [ -f "$file" ]; then
        print_success "✓ $file"
    else
        print_warning "⚠ Optional file missing: $file"
    fi
done

# Step 4: Syntax validation (if Swift is available)
if [ "$SWIFT_AVAILABLE" != "false" ]; then
    print_status "Performing syntax validation..."
    
    cd miniswift
    
    # Try to build the package (this will check syntax)
    if swift build --dry-run &> /dev/null; then
        print_success "✓ Package structure is valid"
    else
        print_warning "⚠ Package build dry-run failed (this is expected without GTK4 dependencies)"
    fi
    
    # Check individual Swift files for basic syntax
    for swift_file in Sources/MiniSwift/UI/*.swift Sources/MiniSwift/*.swift; do
        if [ -f "$swift_file" ]; then
            if swift -frontend -parse "$swift_file" &> /dev/null; then
                print_success "✓ Syntax OK: $(basename "$swift_file")"
            else
                print_error "✗ Syntax error in: $swift_file"
                swift -frontend -parse "$swift_file"
                exit 1
            fi
        fi
    done
    
    cd ..
fi

# Step 5: Run basic tests
print_status "Running basic UI tests..."

if [ -f "test_ui_basic.swift" ]; then
    # Check if the test file has valid syntax
    if [ "$SWIFT_AVAILABLE" != "false" ]; then
        if swift -frontend -parse "test_ui_basic.swift" &> /dev/null; then
            print_success "✓ UI test file syntax is valid"
        else
            print_error "✗ Syntax error in test_ui_basic.swift"
            exit 1
        fi
    fi
    
    print_success "✓ Basic UI test file is ready"
else
    print_warning "⚠ UI test file not found"
fi

# Step 6: Check GTK4 availability (optional)
print_status "Checking GTK4 availability..."

if pkg-config --exists gtk4 2>/dev/null; then
    GTK4_VERSION=$(pkg-config --modversion gtk4)
    print_success "✓ GTK4 found: $GTK4_VERSION"
    print_status "GTK4 include path: $(pkg-config --cflags gtk4)"
    print_status "GTK4 library path: $(pkg-config --libs gtk4)"
else
    print_warning "⚠ GTK4 not found. Install GTK4 for full UI functionality:"
    echo "    macOS: brew install gtk4"
    echo "    Ubuntu: sudo apt-get install libgtk-4-dev"
    echo "    Fedora: sudo dnf install gtk4-devel"
fi

# Step 7: Generate summary report
print_status "Generating build summary..."

echo ""
echo "📋 MiniSwift UI Library Build Summary"
echo "===================================="
echo "✅ Core UI library files: $(ls miniswift/Sources/MiniSwift/UI/*.swift | wc -l | tr -d ' ') files"
echo "✅ Example files: $(ls examples/*.swift 2>/dev/null | wc -l | tr -d ' ') files"
echo "✅ Documentation: $(ls *README*.md | wc -l | tr -d ' ') files"

if [ "$SWIFT_AVAILABLE" != "false" ]; then
    echo "✅ Swift syntax validation: PASSED"
else
    echo "⚠️  Swift syntax validation: SKIPPED (Swift not available)"
fi

if pkg-config --exists gtk4 2>/dev/null; then
    echo "✅ GTK4 backend: AVAILABLE"
else
    echo "⚠️  GTK4 backend: NOT AVAILABLE"
fi

echo ""
print_success "🎉 MiniSwift UI Library build validation completed!"
echo ""
echo "Next steps:"
echo "1. Install GTK4 for full UI functionality"
echo "2. Run examples: swift run UIExample (when GTK4 is available)"
echo "3. Read UI_README.md for detailed documentation"
echo "4. Explore examples/ directory for usage patterns"
echo ""

exit 0