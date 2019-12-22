CFLAGS += $$(sdl2-config --cflags)
LDFLAGS += $$(sdl2-config --libs) -lm

main: main.c vec.h sdf.h

run: main
	env SDL_VIDEO_X11_WMCLASS=raytracer ./main

clean:
	rm -f main

.PHONY: run clean
