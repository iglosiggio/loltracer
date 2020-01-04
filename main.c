#include <assert.h>
#include <math.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include "scene-parser.h"
#include "vec.h"
#include "sdf.h"
#include "renderer.h"

#define TOSTR(text) #text
#define LOG(format, ...) printf("[" __FILE__ ":%d] " format "\n", \
                                __LINE__ __VA_OPT__(,) __VA_ARGS__)

size_t frames;
SDL_atomic_t exiting;
SDL_atomic_t current_line;
SDL_sem* frame_entry_barrier;
SDL_sem* frame_exit_barrier;

static void die(const char* str) {
	perror(str);
	exit(-1);
}

int render_scene(const struct scene* scene, size_t num_threads) {
	SDL_Window*	win;
	SDL_Event	event;
	bool		paused = false;

	int width = 320;
	int height = 240;

	SDL_Thread** threads;
	struct render_data data = {.scene = scene};

	threads = malloc(sizeof(SDL_Thread*) * num_threads);

	LOG("Inicializando threads = %d", num_threads);
	frame_entry_barrier = SDL_CreateSemaphore(0);
	frame_exit_barrier  = SDL_CreateSemaphore(0);
	for (size_t i = 0; i < num_threads; i++)
		threads[i] = SDL_CreateThread(render_thread, "renderer",
		                              &data);

	LOG("Inicializando SDL");
	if (SDL_Init(SDL_INIT_VIDEO))
		die(SDL_GetError());

	win = SDL_CreateWindow("Loltracer", SDL_WINDOWPOS_CENTERED,
	                       SDL_WINDOWPOS_CENTERED, width, height,
	                       SDL_WINDOW_RESIZABLE);
	if (win == NULL)
		die(SDL_GetError());

	while (1) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				SDL_AtomicSet(&exiting, 1);
				for (size_t i = 0; i < num_threads; i++)
					SDL_SemPost(frame_entry_barrier);
				for (size_t i = 0; i < num_threads; i++)
					SDL_WaitThread(threads[i], NULL);
				goto exit;
			}
			if (event.type == SDL_MOUSEBUTTONUP)
				paused = !paused;
		}

		data.surf = SDL_GetWindowSurface(win);
		if (data.surf == NULL)
			die(SDL_GetError());
		data.stretch = false;

		if (SDL_MUSTLOCK(data.surf))
			SDL_LockSurface(data.surf);

		SDL_AtomicSet(&current_line, 0);
		for (size_t i = 0; i < num_threads; i++)
			SDL_SemPost(frame_entry_barrier);
		for (size_t i = 0; i < num_threads; i++)
			SDL_SemWait(frame_exit_barrier);
		frames++;

		if (SDL_MUSTLOCK(data.surf))
			SDL_UnlockSurface(data.surf);

		SDL_UpdateWindowSurface(win);
		SDL_FreeSurface(data.surf);
	}
exit:
	LOG("Cerrando");
	free(threads);
	SDL_DestroySemaphore(frame_entry_barrier);
	SDL_DestroySemaphore(frame_exit_barrier);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}

int main(int argc, char* argv[]) {
	const char*	filename = NULL;
	size_t		num_threads = 1;
	struct scene*	scene = NULL;

	if (argc > 1)
		num_threads = atoi(argv[1]);

	if (argc > 2)
		filename = argv[2];

	scene = scene_parse(filename);
	assert(scene && scene_validate_materials(scene));

	render_scene(scene, num_threads);

	return 0;
}
