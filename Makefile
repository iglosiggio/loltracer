CFLAGS += $$(sdl2-config --cflags)
LDFLAGS += $$(sdl2-config --libs) -lm

main: main.c vec.h sdf.h

run: main
	env SDL_VIDEO_X11_WMCLASS=raytracer ./main

scene-parser.h scene-parser.c: scene-parser.y
	bison -t -d -o $@ $<

scene-lexer.c: scene-lexer.l scene-parser.h
	flex -o $@ $<

scene-parser: scene-parser.c scene-lexer.c
	$(CC) $(CFLAGS) -lfl -lm -o $@ $^

clean:
	rm -f main
	rm -f scene-parser.c scene-parser.h scene-lexer.c scene-parser

.PHONY: run clean
