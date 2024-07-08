vpath %.c ../src
vpath %.c ../src/fonts

LDLIBS = -lm -lSDL2 -lstdc++

FLAGS = -Wall -I../src -g
CFLAGS += $(FLAGS)
CPPFLAGS += $(FLAGS)

FNT_OBJS = $(patsubst %.c,%.o,$(wildcard ../src/fonts/*.c))

OBJS = draw.o fast_sin.o print.o font_draw.o wireframe_draw.o dds.o test_3d.o $(FNT_OBJS)

all: test

test: $(OBJS)

clean:
	rm -rf test $(OBJS)
