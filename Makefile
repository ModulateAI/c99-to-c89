EXT =

all: c99conv$(EXT) c99wrap$(EXT)

SRC_MAIN_ROOT=src/main
SRC_TEST_ROOT=src/test

CC=/opt/local/bin/clang-mp-3.2
LD=$(CC)
CFLAGS=-I/opt/local/libexec/llvm-3.2/include -g
LDFLAGS=-L/opt/local/libexec/llvm-3.2/lib -g
LIBS=-lclang

clean:
	rm -f c99conv$(EXT) c99wrap$(EXT) $(SRC_MAIN_ROOT)/convert.o $(SRC_MAIN_ROOT)/compilewrap.o
	rm -f $(SRC_TEST_ROOT)/unit.c.c $(SRC_TEST_ROOT)/unit2.c.c

test1: c99conv$(EXT)
	$(CC) -E $(SRC_TEST_ROOT)/unit.c -o $(SRC_TEST_ROOT)/unit.prev.c
	./c99conv $(SRC_TEST_ROOT)/unit.prev.c $(SRC_TEST_ROOT)/unit.post.c
	diff -u $(SRC_TEST_ROOT)/unit.{prev,post}.c || :

test2: c99conv$(EXT)
	$(CC) -E $(SRC_TEST_ROOT)/unit2.c -o $(SRC_TEST_ROOT)/unit2.prev.c
	./c99conv $(SRC_TEST_ROOT)/unit2.prev.c $(SRC_TEST_ROOT)/unit2.post.c
	diff -u $(SRC_TEST_ROOT)/unit2.{prev,post}.c || :

test3: c99conv$(EXT)
	$(CC) $(CFLAGS) -E -o $(SRC_MAIN_ROOT)/convert.prev.c $(SRC_MAIN_ROOT)/convert.c
	./c99conv $(SRC_MAIN_ROOT)/convert.prev.c $(SRC_MAIN_ROOT)/convert.post.c
	diff -u $(SRC_MAIN_ROOT)/convert.{prev,post}.c || :

c99conv$(EXT): $(SRC_MAIN_ROOT)/convert.o
	$(CC) -o $@ $< $(LDFLAGS) $(LIBS)

c99wrap$(EXT): $(SRC_MAIN_ROOT)/compilewrap.o
	$(CC) -o $@ $< $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<
