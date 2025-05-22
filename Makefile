LUA ?= luajit

CFLAGS += -O3 $$(sdl2-config --cflags)
LDFLAGS += $$(sdl2-config --libs) -lm

BIN ?= main
SCENE ?= scene.lol
THREADS ?= 8

main: main.c scene-parser.c scene-lexer.c scene.c naive_renderer.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

tracing: main.c scene-parser.c scene-lexer.c scene.c tracing_jit_renderer.c jitdump.c
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

run: $(BIN)
	env SDL_VIDEO_X11_WMCLASS=raytracer ./$(BIN) $(THREADS) examples/$(SCENE)

%.c: %.dasc
	$(LUA) dynasm/dynasm.lua -o $@ $<

scene-parser.h scene-parser.c: scene-parser.y
	bison -t -d -o $@ $<

scene-lexer.c: scene-lexer.l scene-parser.h
	flex -o $@ $<

clean:
	rm -f main
	rm -f scene-parser.c scene-parser.h scene-lexer.c scene-parser
	rm -f tracing tracing_jit_renderer.c

.PHONY: run clean
