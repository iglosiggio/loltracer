#ifndef __RENDERER_H__
#define __RENDERER_H__
#include <SDL.h>
#include "scene.h"

extern SDL_atomic_t	exiting;
extern SDL_atomic_t	current_line;
extern SDL_sem*		frame_entry_barrier;
extern SDL_sem*		frame_exit_barrier;

struct render_data  {
	SDL_Surface* surf;
	bool stretch;
	const struct scene* scene;
};

static inline Uint32 colorf_to_pixfmt(v3 colorf, const SDL_PixelFormat* fmt) {
	Uint8 r = colorf.x * 255;
	Uint8 g = colorf.y * 255;
	Uint8 b = colorf.z * 255;
	return SDL_MapRGB(fmt, r, g, b);
}

int render_thread(void* ptr);

#endif /* __RENDERER_H__ */
