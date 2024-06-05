# compiler to work with
GCC = gcc
# show all warnings and every warning is error
CFLAGS = -Wall -Werror
# level of optimization is 3
CFLAGS += -O3
#CFLAGS += -DDEBUG

.PHONY: all
all:
	$(GCC) $(CFLAGS) *.c -o huffman

.PHONY: clean
clean:
	rm -rf huffman out*
