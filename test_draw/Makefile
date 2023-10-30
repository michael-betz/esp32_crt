vpath %.c ../src

LDLIBS = -lm -lSDL2

CFLAGS += -Wall
CFLAGS += -I../src

OBJS = draw.o fast_sin.o print.o font_data.o font_draw.o

all: test

test: $(OBJS)

clean:
	rm -rf test $(OBJS)

run: test
	./json_to_bin.py draw_list.json | ./test -
