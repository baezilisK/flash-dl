CC = gcc
NOWARN = -Wno-parentheses -Wno-sign-compare -Wno-unused-result
CFLAGS = -std=gnu89 -O4 -Wall -Werror -Wextra -pedantic $(NOWARN)
INCLUDE = -Iinclude
MODULES = util
MODULES_BIN = $(patsubst %, bin/%.o, $(MODULES))

flash-dl: src/main.c $(MODULES_BIN)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $^

bin/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	rm -f main bin/*
