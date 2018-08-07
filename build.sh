#!/usr/bin/env sh
cd "$(dirname "$1")"
filename=$(basename "$1")
filename=${filename%.*}
gcc -Wall -Wextra -g3 "$1" -o "$filename.out"
