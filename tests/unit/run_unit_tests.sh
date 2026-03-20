#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "==> Configuring..."
cmake -B "$BUILD_DIR" "$SCRIPT_DIR" -DCMAKE_BUILD_TYPE=Debug 2>/dev/null

echo "==> Building..."
cmake --build "$BUILD_DIR" 2>/dev/null

echo "==> Running tests..."
"$BUILD_DIR/unit_tests"

echo "==> Generating coverage report..."
cmake --build "$BUILD_DIR" --target coverage 2>/dev/null

echo ""
echo "Coverage report: $BUILD_DIR/coverage_html/index.html"

if command -v open >/dev/null 2>&1; then
	echo "Opening coverage report in default browser..."
	open "$BUILD_DIR/coverage_html/index.html"
else
	echo "Open with: $BUILD_DIR/coverage_html/index.html"
fi
