CC= gcc
CFLAGS= -g -Wall -Werror -std=gnu99 -Iinclude
LD= gcc
LDFLAGS= -L.
AR= ar
ARFLAGS= rcs
TARGETS= bin/server

all: $(TARGETS)

clean:
	@echo Cleaning
	@rm -f $(TARGETS) lib/*.a src/*.o *.log *.input

.PHONY:	all test clean

bin/server: src/server.o
	@echo Linking bin/server...
	@$(LD) $(LDFLAGS) -o $@ $^

src/%.o: src/%.c include/server.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<
