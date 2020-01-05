#include "renderer.h"
#include "sdf.h"
#include "vec.h"

struct world_dist {
	float dist;
	Uint32 id;
};

static inline
struct world_dist sdf(const struct scene* scene, v3 p) {
	size_t obj_id = 0;
	float obj_dist = INFINITY;
	struct world_dist rval = {obj_dist, obj_id};

	vector_foreach(struct object, scene->objects, obj) {
		v3 point = v3sub(p, obj->point);
		switch (obj->type) {
		case OBJ_SPHERE:
			obj_dist = sdSphere(point, obj->sphere.radius);
			break;
		case OBJ_BOX:
			obj_dist = sdRoundBox(point, obj->box.point2,
			                      obj->box.radius);
			break;
		case OBJ_PLANE:
			obj_dist = point.y;
			break;
		default:
			fprintf(stderr, "Unknown scene object");
		}

		obj_id += 1;
		if (obj_dist < rval.dist)
			rval = (struct world_dist) {obj_dist, obj_id};
	}

	return rval;
}

/* ro = ray origin, rd = ray direction */
static
struct world_dist get_intersection(const struct scene* scene, v3 ro, v3 rd) {
	static const size_t	MAX_STEPS = 256;
	static const float	EPSILON = 0.001;
	static const float	MAX_DIST = 100;

	size_t	id = 0;
	float	dist = 0;
	
	for (size_t i = 0; i < MAX_STEPS; i++) {
		v3 p = v3add(ro, v3scale(rd, dist));
		struct world_dist scene_dist = sdf(scene, p);
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
static
float softshadow(const struct scene* scene, v3 ro, v3 rd, size_t max_steps,
	float max_dist, float w) {

	static const float	EPSILON = 0.001;

	float res = 1.0;
	float dist = 0;

	for (size_t i = 0; i < max_steps; i++) {
		v3 p = v3add(ro, v3scale(rd, dist));
		float scene_dist = sdf(scene, p).dist;
		res = fminf(res, w * scene_dist / dist);
		dist += scene_dist;
		if (res < -1 || dist > max_dist)
			break;
	}
	res = fmaxf(res, 0.0);
	return res;
}

static
float in_shadow(const struct scene* scene, const struct light* light, v3 p) {
	float rval = 1;

	v3 dir = v3normalize(v3sub(light->point, p));
	p = v3add(p, v3scale(dir, 0.04));

	rval += softshadow(scene, p, dir, 128, 50, 50);

	return rval;
}

static inline
struct material get_material(const struct scene* scene, size_t obj_id) {
	size_t material_id;
	if (obj_id)
		material_id = vector_get(struct object, scene->objects,
		                         obj_id - 1).material;
	else
		material_id = 0;

	return vector_get(struct material, scene->materials, material_id);
}

static v3 get_normal(const struct scene* scene, v3 p, float dist) {
	static const v3 k0 = { 1, -1, -1};
	static const v3 k1 = {-1, -1,  1};
	static const v3 k2 = {-1,  1, -1};
	static const v3 k3 = { 1,  1,  1};
	const float h = dist / 100;
	const v3 p0 = v3scale(k0, sdf(scene, v3add(p, v3scale(k0, h))).dist);
	const v3 p1 = v3scale(k1, sdf(scene, v3add(p, v3scale(k1, h))).dist);
	const v3 p2 = v3scale(k2, sdf(scene, v3add(p, v3scale(k2, h))).dist);
	const v3 p3 = v3scale(k3, sdf(scene, v3add(p, v3scale(k3, h))).dist);
	return v3normalize(v3add(p0, v3add(p1, v3add(p2, p3))));
}

/* Basado en el modelo Phong (wiki:Phong_reflection_model) */
static
v3 get_light(const struct scene* scene, v3 p, v3 n, size_t obj_id) {
	struct material mat = get_material(scene, obj_id);
	v3 total_light = {0, 0, 0};
	v3 cam_pos = scene->camera.point;
	
	/* ... por cada luz ... */
	vector_foreach(struct light, scene->lights, light) {
		float shadow = in_shadow(scene, light, p);
		v3 light_pos = light->point;
		v3 light_diffuse_intensity = light->diffuse_intensity;
		v3 light_specular_intensity = light->specular_intensity;

		v3 light_dir = v3normalize(v3sub(light_pos, p));
		v3 reflected_dir = v3sub(v3scale(n, 2 * v3dot(light_dir, n)), 
					 light_dir);
		v3 camera_dir = v3normalize(v3sub(cam_pos, p));

		/* Ajusto la iluminación mate según ángulo y sombra */
		float diffuse_incidence = clamp(v3dot(n, light_dir), 0, 1)
		                        * shadow;

		light_diffuse_intensity =
			v3scale(light_diffuse_intensity, diffuse_incidence);
		light_diffuse_intensity =
			v3mul(light_diffuse_intensity, mat.diffuse);

		total_light = v3add(total_light, light_diffuse_intensity);

		/* Ajusto la iluminación especular según ángulo */
		float specular_incidence = powf(
			clamp(v3dot(reflected_dir, camera_dir), 0, 1),
			mat.shininess
		);

		light_specular_intensity =
			v3scale(light_specular_intensity, specular_incidence);
		light_specular_intensity =
			v3mul(light_specular_intensity, mat.specular);

		total_light = v3add(total_light, light_specular_intensity);
	}

	v3 light_ambient_intensity = v3mul(scene->ambient_color, mat.ambient);
	total_light = v3add(total_light, light_ambient_intensity);

	return v3clamp(total_light, 0, 1);
}

static
v3 get_camera_ray(struct camera cam, v2 view_pos) {
	v3 view_pos_3d = {view_pos.x, view_pos.y, v2len(view_pos) / sqrtf(2)};
	v3 dest_pos = v3add(
		v3mul(view_pos_3d, v3sub(cam.se_corner, cam.nw_corner)),
		cam.nw_corner
	);
	return v3normalize(v3sub(dest_pos, cam.point));
}

int render_thread(void* ptr) {
	struct render_data* data = ptr;
	int width;
	int height;
	float fwidth;
	float fheight;
	bool stretch;

	while (true) {
		SDL_SemWait(frame_entry_barrier);
		if (SDL_AtomicGet(&exiting))
			return 0;

		SDL_Surface* surf = data->surf;
		Uint8 bytes_per_pixel = surf->format->BytesPerPixel;
		fwidth = width  = surf->w;
		fheight = height = surf->h;
		stretch = data->stretch;
		const struct scene* scene = data->scene;
		v3 ro = scene->camera.point;

		int y;
		while ((y = SDL_AtomicAdd(&current_line, 1)) < height)
		for (int x = 0; x < width; x++) {
			v2 view_pos;

			if (stretch)
				view_pos = (v2){
					x / fwidth,
					y / fheight,
				};
			else
				view_pos = (v2){
					x / fheight - (fwidth/fheight - 1) / 2,
					y / fheight
				};
			v3 rd = get_camera_ray(scene->camera, view_pos);
			struct world_dist intersect =
				get_intersection(scene, ro, rd);
			v3 p = v3add(ro, v3scale(rd, intersect.dist));
			v3 n = get_normal(scene, p, intersect.dist);
			v3 colorf = get_light(scene, p, n, intersect.id);
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
