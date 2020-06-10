SRCS = $(shell ls *.c)
OBJS = $(SRCS:%.c=%.o)
DEPENDS = Makefile.deps

CC = clang
CFLAGS = -O0 -g -std=c89 -D _GNU_SOURCE

atto: $(OBJS)

clean:
	rm -f *.o atto $(DEPENDS)

depend:
	$(CC) -MM -MF - $(CFLAGS) $(SRCS) > $(DEPENDS)

-include $(DEPENDS)
