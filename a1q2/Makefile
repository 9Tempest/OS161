.PHONY: all clean

CC=clang
CFLAGS+=-Wall -Werror -lpthread -DSOLUTION

all: a1q2 

a1q2: a1q2.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o a1q2
