vpath %.c ../src
vpath %.c ../src/fonts

LDLIBS = -lm -lSDL2
CFLAGS += -Wall -I../src -I. -g

FNT_SRCS = $(wildcard ../src/fonts/*.c)

SRCS = test.c draw.c fast_sin.c font_draw.c wireframe_draw.c dds.c demo_mode.c \
	json_settings.c meteo_swiss.c cJSON.c $(FNT_SRCS)

all: meteo_data.json test

# Native build using CLANG
test: $(SRCS)
	clang $(CFLAGS) $^ $(LDLIBS) -o $@

# Web-assembly build
test.html: $(SRCS)
	emcc -s USE_SDL=2 -s TOTAL_MEMORY=32MB --preload-file meteo_data.json $(CFLAGS) $^ $(LDLIBS) -o $@

meteo_data.json:
	curl -o $@ "https://app-prod-ws.meteoswiss-app.ch/v1/plzDetail?plz=120200"

clean:
	rm -rf test $(OBJS) meteo_data.json test.js test.html test.wasm test.data
