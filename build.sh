#!/bin/bash

# 1. Only configure the project if the build folder doesn't exist yet
if [ ! -d "build" ]; then
    echo "Configuring project for the first time..."
    cmake -S . -B build
    echo "Configuration complete!"
    echo ""
fi

echo "Building project..."
# 2. This command natively detects exactly what changed and compiles incrementally
cmake --build build

# Check if the build succeeded
if [ $? -eq 0 ]; then
    echo ""
    echo "Done! Launching KairoEngine..."
    echo "--------------------------------"
    ./build/KairoEngine
else
    echo ""
    echo "Build failed. Check the errors above."
fi