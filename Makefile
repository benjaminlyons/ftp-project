# Note -Iinclude means to automatically include the 'include' directory
# in the preprocessor. -L. means to include the current directory in the linker
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

bin/server: src/server.o lib/libserver.a
	@echo Linking bin/server...
	@$(LD) $(LDFLAGS) -o $@ $^

src/%.o: src/%.c include/server.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

lib/libserver.a: src/socket.o include/server.h
	@echo Creating lib/libserver.a...
	@$(AR) $(ARFLAGS) -o $@ $^
