CC ?= clang
CFLAGS ?= -std=c11 -Wall -Wextra -Werror
DEBUGFLAGS = -g -O0
VERBOSEFLAGS = -DVERBOSE=1

.PHONY: all test clean debug verbose

# Default build
all: displaymode

displaymode: displaymode.c displaymode_parse.c
	$(CC) $(CFLAGS) -lm -framework CoreFoundation -framework CoreGraphics -o displaymode displaymode.c displaymode_parse.c

# Debug build
debug: CFLAGS += $(DEBUGFLAGS)
debug: clean displaymode

# Verbose build (define VERBOSE macro)
verbose: CFLAGS += $(VERBOSEFLAGS)
verbose: clean displaymode

tests/test_parse: displaymode_parse.c tests/test_parse.c
	$(CC) $(CFLAGS) -o tests/test_parse displaymode_parse.c tests/test_parse.c

# Run tests
tests: tests/test_parse tests/test_format
	./tests/test_parse
	./tests/test_format

tests/test_parse: displaymode_parse.c tests/test_parse.c
	$(CC) $(CFLAGS) -o tests/test_parse displaymode_parse.c tests/test_parse.c

tests/test_format: tests/test_format.c displaymode_format.c displaymode_format.h
	$(CC) $(CFLAGS) -o tests/test_format tests/test_format.c displaymode_format.c

clean:
	rm -f displaymode tests/test_parse tests/test_format
