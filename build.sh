#!/bin/bash
# Build script for vqSPIHT

set -e

cd "$(dirname "$0")"

echo "Building vqSPIHT..."
cd src
make clean 2>/dev/null || true
make
echo "Build complete. Executable: src/vqSPIHT"
