BIN=writer
SRCS=writer.c
OBJS=$(patsubst %.c,%.o,$(SRCS))
CROSS_COMPILE=
CC=$(CROSS_COMPILE)gcc

all: $(OBJS)
	$(CC) $^ -o $(BIN)

%.o: %.c
	$(CC) $< -c

.PHONY: clean

clean:
	$(RM) *.o $(BIN)