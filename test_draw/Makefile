vpath %.c ../src
vpath %.c ../src/fonts

LDLIBS = -lm -lSDL2 -lstdc++

FLAGS = -Wall -I../src -I. -g
CFLAGS += $(FLAGS)
CPPFLAGS += $(FLAGS)

FNT_OBJS = $(patsubst %.c,%.o,$(wildcard ../src/fonts/*.c))

OBJS = draw.o fast_sin.o font_draw.o wireframe_draw.o dds.o demo_mode.o $(FNT_OBJS)

all: test

test: $(OBJS)

clean:
	rm -rf test $(OBJS)
