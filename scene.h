#ifndef __SCENE_H__
#define __SCENE_H__
#include <stdbool.h>
#include "vector.h"
#include "vec.h"

enum property {
	PROP_SHININESS,
	PROP_DIFFUSE,
	PROP_SPECULAR,
	PROP_AMBIENT,
	PROP_COLOR,
	PROP_POINT,
	PROP_DIRECTION,
	PROP_FOV,
	PROP_DIFFUSE_INTENSITY,
	PROP_SPECULAR_INTENSITY,
	PROP_RADIUS,
	PROP_MATERIAL,
	PROP_POINT2,
	PROP_Y,
	PROP_SMOOTHNESS,
	PROP_A,
	PROP_B
};

enum components {
	OBJ_AMBIENT,
	OBJ_CAMERA,
	OBJ_POINT_LIGHT,
	OBJ_SPHERE,
	OBJ_BOX,
	OBJ_PLANE,
	OBJ_SMOOTH_UNION
};

enum value_type {
	VAL_NUM,
	VAL_LIST,
	VAL_ID,
	VAL_OBJ
};

struct material {
	float	shininess;
	v3	diffuse;
	v3	specular;
	v3	ambient;
};


struct light {
	v3	point;
	v3	diffuse_intensity;
	v3	specular_intensity;
};

struct object {
	enum components	type;
	v3		point;
	size_t		material;
	union {
		struct {
			float	radius;
		} sphere;

		struct {
			v3	point2;
			float	radius;
		} box;

		struct {
			float y;
		} plane;

		struct {
			float smoothness;
			struct object* a;
			struct object* b;
		} smooth_op;
	};
};

struct camera {
	v3	point;
	v3	direction;
	float	fov;
};

struct scene {
	struct vector*	materials;
	v3		ambient_color;
	struct vector*	lights;
	struct vector*	objects;
	struct camera	camera;
};


struct definition_value {
	enum value_type	type;
	union {
		float		num;
		struct vector*	list;
		size_t		id;
		struct object	obj;
	};
};

struct definition {
	enum property		prop;
	struct definition_value	value;
};

struct scene* scene_new();
void scene_free(struct scene*);
void scene_add_component_from_definition_list(struct scene*, int,
	struct vector*);
bool scene_validate_materials(const struct scene*);

struct object object_from_definition_list(int type, struct vector* props);

struct material material_from_definition_list(struct vector*);

#endif /* __SCENE_H__ */
