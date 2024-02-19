BIN=aesdsocket
SRCS=aesdsocket.c
OBJS=$(patsubst %.c,%.o,$(SRCS))
CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc
CFLAGS=-Wall -Werror -g

all: $(OBJS)
	$(CC) $^ -o $(BIN) $(CFLAGS)
	$(RM) *.o

%.o: %.c
	$(CC) $< -c $(CFLAGS)

.PHONY: clean

clean:
	$(RM) *.o $(BIN)