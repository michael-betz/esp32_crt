vpath %.c ../src
vpath %.c ../src/fonts

LDLIBS = -lm -lSDL2
CFLAGS += -Wall -I../src -I. -g

FNT_SRCS = $(wildcard ../src/fonts/*.c)

SRCS = test.c draw.c fast_sin.c font_draw.c wireframe_draw.c dds.c demo_mode.c \
	meteo_swiss.c lwjson.c http_client.c http_client_extra.c $(FNT_SRCS)

all: test

# Native build using CLANG
test: $(SRCS)
	clang $(CFLAGS) $^ $(LDLIBS) -lcurl -o $@

# Web-assembly build. Run it locally with `emrun index.html`
wasm: meteo_data.json $(SRCS)
	mkdir -p dist && \
	emcc -s USE_SDL=2 -s TOTAL_MEMORY=32MB --preload-file $< $(CFLAGS) $(filter-out meteo_data.json,$^) $(LDLIBS) -o dist/index.html

meteo_data.json:
	curl -o $@ "https://app-prod-ws.meteoswiss-app.ch/v1/plzDetail?plz=120200"

clean:
	rm -rf dist/ test $(OBJS) meteo_data.json test.js test.html test.wasm test.data
