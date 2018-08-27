#!/usr/bin/env sh
cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null
mkdir ../build >/dev/null 2>&1
gcc -lm -Wall -Wextra -Wno-unused-function -g3 main.c -o ../build/interpreter
