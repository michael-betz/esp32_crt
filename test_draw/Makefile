vpath %.c ../src
vpath %.c ../src/fonts

LDLIBS = -lm -lSDL2
CFLAGS += -Wall -I../src -I. -g

FNT_SRCS = $(wildcard ../src/fonts/*.c)

SRCS = test.c draw.c fast_sin.c font_draw.c wireframe_draw.c dds.c demo_mode.c \
	meteo_swiss.c pdjson.c $(FNT_SRCS)

all: test

# Native build using CLANG
test: meteo_data.json $(SRCS)
	clang $(CFLAGS) $(filter-out meteo_data.json,$^) $(LDLIBS) -o $@

# Web-assembly build. Run it locally with `emrun test.html`
test.html: meteo_data.json $(SRCS)
	emcc -s USE_SDL=2 -s TOTAL_MEMORY=32MB --preload-file $< $(CFLAGS) $(filter-out meteo_data.json,$^) $(LDLIBS) -o $@

meteo_data.json:
	curl -o $@ "https://app-prod-ws.meteoswiss-app.ch/v1/plzDetail?plz=120200"

clean:
	rm -rf test $(OBJS) meteo_data.json test.js test.html test.wasm test.data
