# Note -Iinclude means to automatically include the 'include' directory
# in the preprocessor. -L. means to include the current directory in the linker
CC= gcc
CFLAGS= -g -Wall -Werror -std=gnu99 -Iinclude
LD= gcc
LDFLAGS= -L.
AR= ar
ARFLAGS= rcs
TARGETS= bin/server bin/client

all: $(TARGETS)

test: $(TARGETS)
	@EXIT=0; 
	./bin/test_server.sh; 
	@EXIT=$$(($$EXIT+$$?)); 
	@exit $$EXIT

clean:
	@echo Cleaning...
	@rm -f $(TARGETS) lib/*.a src/server/*.o src/client/*.o *.log *.input

.PHONY:	all test clean

bin/server: src/server/server.o lib/libserver.a
	@echo Linking bin/server...
	@mkdir -p bin
	@$(LD) $(LDFLAGS) -o $@ $^

bin/client: src/client/client.o lib/libclient.a
	@echo Creating bin/client...
	@mkdir -p bin
	@$(LD) $(LDFLAGS) -o $@ $^

src/server/%.o: src/server/%.c include/server.h include/macros.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

src/client/%.o: src/client/%.c include/client.h include/macros.h
	@echo "Compiling $@..."
	@$(CC) $(CFLAGS) -c -o $@ $<

lib/libserver.a: src/server/socket.o src/server/request.o src/server/handler.o src/server/utils.o include/server.h include/macros.h
	@echo Creating lib/libserver.a...
	@mkdir -p lib
	@$(AR) $(ARFLAGS) -o $@ $^

lib/libclient.a: src/client/handler.o src/client/cli.o include/client.h include/macros.h
	@echo Creating lib/libclient.a...
	@$(AR) $(ARFLAGS) -o $@ $^

