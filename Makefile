LUA ?= luajit

CFLAGS += $$(sdl2-config --cflags)
LDFLAGS += $$(sdl2-config --libs) -lm

SCENE ?= scene.lol
THREADS ?= 8

main: main.c vec.h sdf.h float.h scene-parser.c scene-lexer.c scene.c naive_renderer.c

tracing: main.c vec.h sdf.h float.h scene-parser.c scene-lexer.c scene.c tracing_jit_renderer.c jitdump.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

run: main
	env SDL_VIDEO_X11_WMCLASS=raytracer ./main $(THREADS) $(SCENE)

%.c: %.dasc
	$(LUA) dynasm/dynasm.lua -o $@ $<

scene-parser.h scene-parser.c: scene-parser.y
	bison -t -d -o $@ $<

scene-lexer.c: scene-lexer.l scene-parser.h
	flex -o $@ $<

clean:
	rm -f main
	rm -f scene-parser.c scene-parser.h scene-lexer.c scene-parser
	rm -f tracing_jit_renderer.c

.PHONY: run clean
