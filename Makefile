CC ?= clang
CFLAGS ?= -std=c11 -Wall -Wextra -Werror
DEBUGFLAGS = -g -O0
VERBOSEFLAGS = -DVERBOSE=1

.PHONY: all test clean debug verbose

# Update targets to be placed in the bin directory
BIN_DIR = bin

all: $(BIN_DIR)/displaymode

$(BIN_DIR)/displaymode: displaymode.c displaymode_parse.c
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -lm -framework CoreFoundation -framework CoreGraphics -o $(BIN_DIR)/displaymode displaymode.c displaymode_parse.c

debug: clean $(BIN_DIR)/displaymode

verbose: clean $(BIN_DIR)/displaymode

tests: $(BIN_DIR)/tests/test_parse $(BIN_DIR)/tests/test_format
	./$(BIN_DIR)/tests/test_parse
	./$(BIN_DIR)/tests/test_format

$(BIN_DIR)/tests/test_parse: displaymode_parse.c tests/test_parse.c
	mkdir -p $(BIN_DIR)/tests
	$(CC) $(CFLAGS) -o $(BIN_DIR)/tests/test_parse displaymode_parse.c tests/test_parse.c

$(BIN_DIR)/tests/test_format: tests/test_format.c displaymode_format.c displaymode_format.h
	mkdir -p $(BIN_DIR)/tests
	$(CC) $(CFLAGS) -o $(BIN_DIR)/tests/test_format tests/test_format.c displaymode_format.c

clean:
	rm -rf $(BIN_DIR)
