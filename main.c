#include <assert.h>
#include <math.h>
#include <sched.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>

#include "vec.h"
#include "sdf.h"

#define TOSTR(text) #text
#define LOG(format, ...) printf("[" __FILE__ ":%d] " format "\n", \
                                __LINE__ __VA_OPT__(,) __VA_ARGS__)

size_t frames;

static void die(const char* str) {
	perror(str);
	exit(-1);
}

static Uint8 rand_byte() {
	return rand() % 256;
}

struct world_dist { float dist; Uint32 id; };

static inline struct world_dist sdf(v3 p) {
	struct world_dist rval = { INFINITY, 0 };

	{
		v3 pos = v3sub(p, (v3){0, 1, 6});
		pos.x += sinf(frames / 10.) * 5;
		float r = 1;

		size_t obj_id = 1;
		float obj_dist = sdSphere(pos, 1);

		if (obj_dist < rval.dist)
			rval = (struct world_dist){obj_dist, obj_id };
	}

	{
		v3 pos = v3sub(p, (v3){-1, -0.5, 3});
		float r = 1;

		size_t obj_id = 2;
		float obj_dist = sdSphere(pos, 1);

		if (obj_dist < rval.dist)
			rval = (struct world_dist){obj_dist, obj_id };
	}

	{
		v3 pos = v3sub(p, (v3){2, 2, 10});
		float r = 0.6;

		size_t obj_id = 3;
		float obj_dist = sdRoundBox(pos, (v3){2, 2, 2}, r);

		if (obj_dist < rval.dist)
			rval = (struct world_dist){obj_dist, obj_id };
	}

	{
		v3 pos = v3sub(p, (v3){0, -1, 0});

		size_t obj_id = 4;
		float obj_dist = pos.y;

		if (obj_dist < rval.dist)
			rval = (struct world_dist){ obj_dist, obj_id };
	}

	return rval;
}

/* ro = ray origin, rd = ray direction */
static struct world_dist get_intersection(v3 ro, v3 rd) {
	static const size_t	MAX_STEPS = 256;
	static const float	EPSILON = 0.001;
	static const float	MAX_DIST = 100;

	size_t	id = 0;
	float	dist = 0;
	
	for (size_t i = 0; i < MAX_STEPS; i++) {
		v3 p = v3add(ro, v3scale(rd, dist));
		struct world_dist scene_dist = sdf(p);
		dist += scene_dist.dist;
		id = scene_dist.id;
		if (scene_dist.dist < EPSILON || dist > MAX_DIST)
			break;
	}

	if (dist >= MAX_DIST)
		id = 0;

	/* TODO: Give a correct Object ID */
	return (struct world_dist){ dist, id };
}

static float in_shadow(v3 p) {
	/* Tengo una sola luz y la muevo un poco */
	v3 light_pos = {-2, 10, 1};
	light_pos.x += sinf(frames / 12.) * 5;
	light_pos.z += cosf(frames / 7. ) * 8;

	v3 dir = v3normalize(v3sub(light_pos, p));
	p = v3add(p, v3scale(dir, 0.04));

	return get_intersection(p, dir).id == 0;
}

static inline Uint32 colorf_to_colori(v3 colorf) {
	unsigned char r = colorf.x * 255;
	unsigned char g = colorf.y * 255;
	unsigned char b = colorf.z * 255;
	return r << 24 | g << 16 | b << 8 | 0xFF;
}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#error "This program doesn't support big endian architectures yet"
#endif /* SDL_BYTEORDER = SDL_BIG_ENDIAN */

static v3 get_color(v3 p, size_t obj_id) {
	v3 color;
	switch (obj_id) {
	case 1:  return color = (v3){1, 0, 0};
	case 2:  return color = (v3){0, 1, 0};
	case 3:  return color = (v3){0, 0, 1};
	case 4:  return color = (v3){1, 1, 0};
	default: return color = (v3){0, 0, 0};
	}
}

static v3 get_normal(v3 p) {
	static const float h = 0.0001;
	static const v3 k0 = { 1, -1, -1};
	static const v3 k1 = {-1, -1,  1};
	static const v3 k2 = {-1,  1, -1};
	static const v3 k3 = { 1,  1,  1};
	const v3 p0 = v3scale(k0, sdf(v3add(p, v3scale(k0, h))).dist);
	const v3 p1 = v3scale(k1, sdf(v3add(p, v3scale(k1, h))).dist);
	const v3 p2 = v3scale(k2, sdf(v3add(p, v3scale(k2, h))).dist);
	const v3 p3 = v3scale(k3, sdf(v3add(p, v3scale(k3, h))).dist);
	return v3normalize(v3add(p0, v3add(p1, v3add(p2, p3))));
}

/* Basado en el modelo Phong (wiki:Phong_reflection_model) */
static v3 get_light(v3 p, v3 n, size_t obj_id) {
	/* Es común a todo el ambiente */
	v3 light_ambient_intensity = {0.2, 0.2, 0.2};

	float shadow = in_shadow(p);

	/* Información del material
	   TODO: Utilizar obj_id para extraerla */
	float material_shininess = 25;
	float material_diffuse_reflection = 0.3;
	float material_specular_reflection = 0.7;
	float material_ambient_reflection = 1;
	
	/* ... por cada luz ... */
	v3 light_pos = {-2, 10, 1};
	light_pos.x += sinf(frames / 12.) * 5;
	light_pos.z += cosf(frames / 7. ) * 8;

	v3 light_dir = v3normalize(v3sub(light_pos, p));
	v3 reflected_dir = v3sub(v3scale(n, 2 * v3dot(light_dir, n)), 
	                         light_dir);
	v3 camera_dir = v3normalize(v3sub((v3){0, 1, 0}, p));

	v3 light_diffuse_intensity = {1, 1, 1};
	v3 light_specular_intensity = {1, 1, 1};

	/* Ajusto la iluminación mate según ángulo y sombra */
	float diffuse_incidence = shadow * v3dot(n, light_dir);
	light_diffuse_intensity = v3scale(light_diffuse_intensity,
	                                  diffuse_incidence);
	light_diffuse_intensity = v3scale(light_diffuse_intensity,
	                                  material_diffuse_reflection);

	/* Ajusto la iluminación especular según ángulo y sombra */
	float specular_incidence = shadow * powf(
		v3dot(reflected_dir, camera_dir) * shadow,
		material_shininess
	);
	light_specular_intensity = v3scale(light_diffuse_intensity,
	                                   specular_incidence);

	light_ambient_intensity = v3scale(light_ambient_intensity,
	                                  material_ambient_reflection);

	v3 light = v3add(light_diffuse_intensity, light_specular_intensity);

	return v3clamp(v3add(light_ambient_intensity, light), 0, 1);
}

int main(int argc, char* argv[]) {
	static const int	WIDTH = 320;
	static const int	HEIGHT = 240;

	SDL_Window*	win;
	SDL_Renderer*	ctx;
	SDL_Event	event;
	bool		paused = false;

	SDL_Texture*	tex;
	void*		texbuf;
	int		width;
	

	LOG("Inicializando");
	if (SDL_Init(SDL_INIT_VIDEO))
		die(SDL_GetError());

	if (SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_RESIZABLE,
	                                &win, &ctx))
		die(SDL_GetError());

	tex = SDL_CreateTexture(ctx, SDL_PIXELFORMAT_RGBA8888,
	                        SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

	if (tex == NULL)
		die(SDL_GetError());


	while (1) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				goto exit;
			if (event.type == SDL_MOUSEBUTTONUP)
				paused = !paused;
		}

		SDL_RenderClear(ctx);
		SDL_LockTexture(tex, NULL, &texbuf, &width);
		for (int y = 0; y < HEIGHT; y++)
		for (int x = 0; x < WIDTH; x++) {
			v3 ro = {0, 1, 0};
			v3 rd = v3normalize((v3){
				(x - WIDTH / 2) / (float)HEIGHT,
				-(y - HEIGHT / 2) / (float)HEIGHT,
				1
			});
			struct world_dist intersect = get_intersection(ro, rd);
			v3 p = v3add(ro, v3scale(rd, intersect.dist));
			v3 colorf = get_color(p, intersect.id);
			v3 light = get_light(p, get_normal(p), intersect.id);
			colorf = v3mul(colorf, light);
			Uint32 colori = colorf_to_colori(colorf);
			((Uint32*) texbuf)[x + width / 4 * y] = colori;
		}
		frames++;
		SDL_UnlockTexture(tex);
		SDL_RenderCopy(ctx, tex, NULL, NULL);
		SDL_RenderPresent(ctx);
	}
exit:
	LOG("Cerrando");
	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ctx);
	SDL_DestroyWindow(win);
	SDL_Quit();
	return 0;
}
