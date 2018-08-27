#!/usr/bin/env sh
cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null
gcc -lm -Wall -Wextra -Wno-unused-function -g3 main.c -o main.out
