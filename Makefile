CC ?= gcc
CFLAGS = -g3 -lm -Wall -Wextra -Wstrict-prototypes -Wno-unused-function -ICommon/
BUILD_DIR ?= build

interpreter = $(BUILD_DIR)/interpreter
vm = $(BUILD_DIR)/vm
compiler = $(BUILD_DIR)/compiler

all:  $(interpreter) $(vm) $(compiler)

$(interpreter): $(wildcard Interpreter/*) Common/common.c Common/lexer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) Interpreter/main.c -o $(interpreter)

$(vm): $(wildcard VirtualMachine/*) Common/common.c Common/instruction_table.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) VirtualMachine/main.c -o $(vm)

$(compiler): $(wildcard Compiler/*) Common/common.c Common/instruction_table.h Common/lexer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) Compiler/main.c -o $(compiler)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -r $(BUILD_DIR)

.PHONY: all clean
