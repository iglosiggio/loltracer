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
SDL_atomic_t exiting;
SDL_atomic_t current_line;
SDL_sem* frame_entry_barrier;
SDL_sem* frame_exit_barrier;

static void die(const char* str) {
	perror(str);
	exit(-1);
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

	return (struct world_dist){ dist, id };
}

/* https://iquilezles.org/www/articles/rmshadows/rmshadows.htm */
static float softshadow(v3 ro, v3 rd, size_t max_steps, float max_dist,
                        float w) {

	static const float	EPSILON = 0.001;

	float res = 1.0;
	float dist = 0;

	for (size_t i = 0; i < max_steps; i++) {
		v3 p = v3add(ro, v3scale(rd, dist));
		float scene_dist = sdf(p).dist;
		res = fminf(res, w * scene_dist / dist);
		dist += scene_dist;
		if (res < -1 || dist > max_dist)
			break;
	}
	res = fmaxf(res, 0.0);
	return res;
}

static float in_shadow(v3 p) {
	/* Tengo una sola luz y la muevo un poco */
	v3 light_pos = {-2, 10, 1};
	light_pos.x += sinf(frames / 12.) * 5;
	light_pos.z += cosf(frames / 7. ) * 8;

	v3 dir = v3normalize(v3sub(light_pos, p));
	p = v3add(p, v3scale(dir, 0.04));

	float rval = softshadow(p, dir, 128, 50, 50);
	return rval;
}

static inline Uint32 colorf_to_pixfmt(v3 colorf, const SDL_PixelFormat* fmt) {
	Uint8 r = colorf.x * 255;
	Uint8 g = colorf.y * 255;
	Uint8 b = colorf.z * 255;
	return SDL_MapRGB(fmt, r, g, b);
}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#error "This program doesn't support big endian architectures yet"
#endif /* SDL_BYTEORDER = SDL_BIG_ENDIAN */

struct material {
	float shininess;
	v3 diffuse;
	v3 specular;
	v3 ambient;
};

static struct material get_material(size_t obj_id) {
	static struct material materials[5] = {
		{
			.shininess = 4,
			.diffuse   = { 0, 0, 0 },
			.specular  = { 0, 0, 0 },
			.ambient   = { 0, 0, 0 }
		},
		{
			.shininess = 3,
			.diffuse   = { 0.2, 0, 0 },
			.specular  = { 0.2, 0.2, 0.2 },
			.ambient   = { 0.2, 0, 0 }
		},
		{
			.shininess = 50,
			.diffuse   = { 0, 0.2, 0 },
			.specular  = { 0.2, 0.2, 0.2 },
			.ambient   = { 0, 0.2, 0 }
		},
		{
			.shininess = 2,
			.diffuse   = { 0, 0, 0.2 },
			.specular  = { 0.01, 0.01, 0.01 },
			.ambient   = { 0, 0, 0.2 }
		},
		{
			.shininess = 10,
			.diffuse   = { 0.2, 0.2, 0 },
			.specular  = { 0.001, 0.001, 0.001 },
			.ambient   = { 0.2, 0.2, 0 }
		},
	};
	switch (obj_id) {
	case 1: case 2: case 3: case 4: return materials[obj_id];
	default: return materials[0];
	}
}

static v3 get_normal(v3 p, float dist) {
	static const v3 k0 = { 1, -1, -1};
	static const v3 k1 = {-1, -1,  1};
	static const v3 k2 = {-1,  1, -1};
	static const v3 k3 = { 1,  1,  1};
	const float h = dist / 100;
	const v3 p0 = v3scale(k0, sdf(v3add(p, v3scale(k0, h))).dist);
	const v3 p1 = v3scale(k1, sdf(v3add(p, v3scale(k1, h))).dist);
	const v3 p2 = v3scale(k2, sdf(v3add(p, v3scale(k2, h))).dist);
	const v3 p3 = v3scale(k3, sdf(v3add(p, v3scale(k3, h))).dist);
	return v3normalize(v3add(p0, v3add(p1, v3add(p2, p3))));
}

/* Basado en el modelo Phong (wiki:Phong_reflection_model) */
static v3 get_light(v3 p, v3 n, size_t obj_id) {
	/* Es común a todo el ambiente */
	v3 light_ambient_intensity = {0.03, 0.03, 0.03};

	float shadow = in_shadow(p);

	struct material mat = get_material(obj_id);
	
	/* ... por cada luz ... */
	v3 light_pos = {-2, 10, 1};
	light_pos.x += sinf(frames / 12.) * 5;
	light_pos.z += cosf(frames / 7. ) * 8;

	v3 light_dir = v3normalize(v3sub(light_pos, p));
	v3 reflected_dir = v3sub(v3scale(n, 2 * v3dot(light_dir, n)), 
	                         light_dir);
	v3 camera_dir = v3normalize(v3sub((v3){0, 1, 0}, p));

	v3 light_diffuse_intensity = {4, 4, 4};
	v3 light_specular_intensity = {4, 4, 4};

	/* Ajusto la iluminación mate según ángulo y sombra */
	float diffuse_incidence = shadow *fmaxf(0, v3dot(n, light_dir));
	light_diffuse_intensity = v3scale(light_diffuse_intensity,
	                                  diffuse_incidence);
	light_diffuse_intensity = v3mul(light_diffuse_intensity, mat.diffuse);

	/* Ajusto la iluminación especular según ángulo y sombra */
	float specular_incidence = shadow * fmaxf(0, powf(
		v3dot(reflected_dir, camera_dir),
		mat.shininess
	));
	light_specular_intensity = v3scale(light_specular_intensity,
	                                   specular_incidence);
	light_specular_intensity = v3mul(light_specular_intensity,
	                                 mat.specular);

	light_ambient_intensity = v3mul(light_ambient_intensity,
	                                mat.ambient);

	v3 total_light = light_ambient_intensity;
	total_light = v3add(total_light, light_diffuse_intensity);
	total_light = v3add(total_light, light_specular_intensity);

	return v3clamp(total_light, 0, 1);
}


struct render_data  {
	SDL_Surface* surf;
};

int render_thread(void* ptr) {
	struct render_data* data = ptr;
	int width;
	int height;

	while (true) {
		SDL_SemWait(frame_entry_barrier);
		if (SDL_AtomicGet(&exiting))
			return 0;

		SDL_Surface* surf = data->surf;
		Uint8 bytes_per_pixel = surf->format->BytesPerPixel;
		width  = surf->w;
		height = surf->h;

		int y;
		while ((y = SDL_AtomicAdd(&current_line, 1)) < height)
		for (int x = 0; x < width; x++) {
			v3 ro = {0, 1, 0};
			v3 rd = v3normalize((v3){
				(x - width / 2) / (float)height,
				-(y - height / 2) / (float)height,
				1
			});
			struct world_dist intersect = get_intersection(ro, rd);
			v3 p = v3add(ro, v3scale(rd, intersect.dist));
			v3 n = get_normal(p, intersect.dist);
			v3 colorf = get_light(p, n, intersect.id);
			/* gamma correction */
			colorf = v3pow(colorf, 1.0/2.2);
			Uint32 colori = colorf_to_pixfmt(colorf, surf->format);
			*((Uint32*)(surf->pixels
			            + x * bytes_per_pixel
			            + y * surf->pitch)) = colori;
		}

		SDL_SemPost(frame_exit_barrier);
	}
}

int main(int argc, char* argv[]) {
	SDL_Window*	win;
	SDL_Event	event;
	bool		paused = false;

	int width = 320;
	int height = 240;

	size_t num_threads = 1;
	SDL_Thread** threads;
	struct render_data data;

	if (argc > 1)
		num_threads = atoi(argv[1]);

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
