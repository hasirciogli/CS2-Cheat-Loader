#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print messages
print_message() {
    local color="$1"
    local message="$2"
    echo -e "${color}${message}${NC}"
}

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    print_message $YELLOW "Creating build directory..."
    mkdir build
fi

# Navigate to build directory
cd build

# Run CMake to configure the project
print_message $YELLOW "Configuring the project with CMake..."
if cmake ..; then
    print_message $GREEN "Configuration successful."
else
    print_message $RED "Configuration failed."
    exit 1
fi

# Build the project
print_message $YELLOW "Building the project..."
if cmake --build .; then
    print_message $GREEN "Build successful."
else
    print_message $RED "Build failed."
    exit 1
fi

print_message $GREEN "Build process completed successfully."