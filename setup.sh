#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

pause() {
	read -n 1 -s -r -p "Press any key to continue..."
	echo
}

on_error() {
	echo
	echo "Setup failed."
	pause
	exit 1
}

trap on_error ERR

echo "=== Updating submodules ==="
git submodule update --init --recursive

echo
echo "=== Configuring CMake ==="
cmake -S . -B build

echo
echo "=== Building pixey-sample ==="
cmake --build build --config Debug --target pixey-sample

echo
echo "Done. Executable is under ./build/pixey-sample/."
echo
pause
