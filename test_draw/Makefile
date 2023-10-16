vpath %.c ../src

LDLIBS = -lm -lSDL2

CFLAGS += -Wall
CFLAGS += -I../src

OBJS = draw.o fast_sin.o

all: test

test: $(OBJS)

clean:
	rm -rf test $(OBJS)
