vpath %.c ../src
vpath %.c ../src/fonts

LDLIBS = -lm -lSDL2 -lcjson

FLAGS = -Wall -I../src -I. -g
CFLAGS += $(FLAGS)
CPPFLAGS += $(FLAGS)

FNT_OBJS = $(patsubst %.c,%.o,$(wildcard ../src/fonts/*.c))

OBJS = draw.o fast_sin.o font_draw.o wireframe_draw.o dds.o demo_mode.o \
	json_settings.o meteo_swiss.o $(FNT_OBJS)

all: meteo_data.json test

test: $(OBJS)

meteo_data.json:
	curl -o $@ "https://app-prod-ws.meteoswiss-app.ch/v1/plzDetail?plz=120200"

clean:
	rm -rf test $(OBJS) meteo_data.json
