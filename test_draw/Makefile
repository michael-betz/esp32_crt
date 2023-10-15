vpath %.c ../src

LDLIBS = -lSDL2

CFLAGS += -Wall
CFLAGS += -I../src

OBJS = draw.o

all: test

test: $(OBJS)

clean:
	rm -rf test $(OBJS)
