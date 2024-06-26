vpath %.c ../src

LDLIBS = -lm -lSDL2 -lstdc++

FLAGS = -Wall -I../src -g
CFLAGS += $(FLAGS)
CPPFLAGS += $(FLAGS)

OBJS = draw.o fast_sin.o print.o font_data.o font_draw.o dds.o test_3d.o

all: test

test: $(OBJS)

clean:
	rm -rf test $(OBJS)

%.c: %.json
	./json_to_bin.py $^ | xxd -i -n dl_test > $@

run: test
	./json_to_bin.py draw_list.json | ./test -
