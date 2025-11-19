CC ?= clang
CFLAGS ?= -std=c11 -Wall -Wextra -Werror

.PHONY: all test clean

all: displaymode

displaymode: displaymode.c displaymode_parse.c
	$(CC) $(CFLAGS) -lm -framework CoreFoundation -framework CoreGraphics -o displaymode displaymode.c displaymode_parse.c

test: tests/test_parse
	./tests/test_parse

tests/test_parse: displaymode_parse.c tests/test_parse.c
	$(CC) $(CFLAGS) -o tests/test_parse displaymode_parse.c tests/test_parse.c

clean:
	rm -f displaymode tests/test_parse
