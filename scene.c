#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"

void object_free(void* obj_ptr) {
	struct object* obj = obj_ptr;

	switch (obj->type) {
	case OBJ_SMOOTH_UNION:
		free(obj->smooth_op.a);
		free(obj->smooth_op.b);
		break;
	}
}

void definition_free(void* def_ptr) {
	struct definition* def = def_ptr;

	switch (def->value.type) {
	case VAL_LIST:
		vector_free(def->value.list, NULL);
		break;
	case VAL_OBJ:
		object_free(&def->value.obj);
		break;
	}
}

struct scene* scene_new() {
	struct scene* scene = malloc(sizeof(struct scene));
	*scene = (struct scene) {
		.materials	= vector_new(struct material, 16),
		.ambient_color	= (v3) {0, 0, 0},
		.lights		= vector_new(struct light, 16),
		.objects	= vector_new(struct object, 16),
		.camera		= (struct camera) {
			.point		= (v3) {0, 0, 0},
			.direction	= (v3) {0, 0, 1},
			.fov		= M_PI / 2
		}
	};
	return scene;
}

void scene_free(struct scene* scene) {
	vector_free(scene->materials, NULL);
	vector_free(scene->lights, NULL);
	vector_free(scene->objects, object_free);
	free(scene);
}

static inline
v3 v3_from_vector(struct vector* vec) {
	assert(vec->size == 3);
	return (v3) {
		vector_get(float, vec, 0),
		vector_get(float, vec, 1),
		vector_get(float, vec, 2)
	};
}

/* TODO: Proper error management */

static inline
float prop_check_num(const struct definition* def) {
	assert("Is a number" && def->value.type == VAL_NUM);
	return def->value.num;
}

static inline
v3 prop_check_v3(const struct definition* def) {
	assert("Is a list" && def->value.type == VAL_LIST);
	return v3_from_vector(def->value.list);
}

static inline
size_t prop_check_id(const struct definition* def) {
	assert("Is an ID" && def->value.type == VAL_ID);
	return def->value.id;
}

struct object* prop_check_obj(const struct definition* def) {
	struct object* rval;
	assert("Is an object" && def->value.type == VAL_OBJ), \
	rval = malloc(sizeof(struct object));
	*rval = def->value.obj;
	return rval;
}

#define PROP_CASE(prop, field) \
	case PROP_##prop: \
		obj.field = _Generic((obj.field), \
			float:		prop_check_num, \
			v3:		prop_check_v3, \
			size_t:		prop_check_id, \
			struct object*:	prop_check_obj \
		)(def); \
		break;

#define NAMED_PROPERTY_EXTRACTOR_BEGIN(typename, name) \
struct typename name##_from_definition_list(struct vector* properties) { \
	const char* extractor_type = #name; \
	struct typename obj; \
	memset(&obj, 0, sizeof(obj)); \

#define PROPERTY_EXTRACTOR_BEGIN(typename) \
struct typename typename##_from_definition_list(struct vector* properties) { \
	const char* extractor_type = #typename; \
	struct typename obj; \
	memset(&obj, 0, sizeof(obj)); \

#define SWITCH_BEGIN \
	vector_foreach(struct definition, properties, def) \
	switch (def->prop) {

#define SWITCH_END \
	default: \
		fprintf(stderr, "Unknown %s property\n", extractor_type); \
		exit(1); \
	} \

#define PROPERTY_EXTRACTOR_END \
	return obj; \
}

PROPERTY_EXTRACTOR_BEGIN(material)
	SWITCH_BEGIN
		PROP_CASE(SHININESS,	shininess);
		PROP_CASE(DIFFUSE,	diffuse);
		PROP_CASE(SPECULAR,	specular);
		PROP_CASE(AMBIENT,	ambient);
	SWITCH_END
PROPERTY_EXTRACTOR_END

struct v3 ambient_from_definition_list(struct vector* properties) {
	/* TODO: Proper error management */
	v3 ambient_color;

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case PROP_COLOR:
		ambient_color = prop_check_v3(def);
		break;
	default:
		fprintf(stderr, "Unknown ambient property\n");
		exit(1);
	}

	return ambient_color;
}

PROPERTY_EXTRACTOR_BEGIN(camera)
	SWITCH_BEGIN
		PROP_CASE(POINT,	point);
		PROP_CASE(DIRECTION,	direction);
		PROP_CASE(FOV,		fov);
	SWITCH_END

	obj.direction = v3normalize(obj.direction);
	obj.fov = obj.fov / 180 * M_PI;
PROPERTY_EXTRACTOR_END

PROPERTY_EXTRACTOR_BEGIN(light)
	SWITCH_BEGIN
		PROP_CASE(POINT,		point);
		PROP_CASE(DIFFUSE_INTENSITY,	diffuse_intensity);
		PROP_CASE(SPECULAR_INTENSITY,	specular_intensity);
	SWITCH_END
PROPERTY_EXTRACTOR_END

NAMED_PROPERTY_EXTRACTOR_BEGIN(object, sphere)
	obj.type = OBJ_SPHERE;

	SWITCH_BEGIN
		PROP_CASE(POINT,	point);
		PROP_CASE(MATERIAL,	material);
		PROP_CASE(RADIUS,	sphere.radius);
	SWITCH_END
PROPERTY_EXTRACTOR_END

NAMED_PROPERTY_EXTRACTOR_BEGIN(object, box)
	obj.type = OBJ_BOX;

	SWITCH_BEGIN
		PROP_CASE(POINT,	point);
		PROP_CASE(MATERIAL,	material);
		PROP_CASE(POINT2,	box.point2);
		PROP_CASE(RADIUS,	box.radius);
	SWITCH_END
PROPERTY_EXTRACTOR_END


NAMED_PROPERTY_EXTRACTOR_BEGIN(object, plane)
	obj.type = OBJ_PLANE;

	SWITCH_BEGIN
		PROP_CASE(MATERIAL,	material);
		PROP_CASE(Y,		plane.y);
	SWITCH_END

	obj.point = (v3) {0, obj.plane.y, 0};
PROPERTY_EXTRACTOR_END

NAMED_PROPERTY_EXTRACTOR_BEGIN(object, smooth_union)
	obj.type = OBJ_SMOOTH_UNION;

	SWITCH_BEGIN
		PROP_CASE(MATERIAL,	material);
		PROP_CASE(SMOOTHNESS,	smooth_op.smoothness);
		PROP_CASE(A,		smooth_op.a);
		PROP_CASE(B,		smooth_op.b);
	SWITCH_END
PROPERTY_EXTRACTOR_END

void scene_add_component_from_definition_list(struct scene* scene, int type,
	struct vector* props) {

	/* TODO: Proper error management */
	switch (type) {
	case OBJ_AMBIENT:
		scene->ambient_color = ambient_from_definition_list(props);
		break;
	case OBJ_CAMERA:
		scene->camera = camera_from_definition_list(props);
		break;
	case OBJ_POINT_LIGHT:
		vector_add(struct light, scene->lights) =
			light_from_definition_list(props);
		break;
	case OBJ_SPHERE:
		vector_add(struct object, scene->objects) =
			sphere_from_definition_list(props);
		break;
	case OBJ_BOX:
		vector_add(struct object, scene->objects) =
			box_from_definition_list(props);
		break;
	case OBJ_PLANE:
		vector_add(struct object, scene->objects) =
			plane_from_definition_list(props);
		break;
	case OBJ_SMOOTH_UNION:
		vector_add(struct object, scene->objects) =
			smooth_union_from_definition_list(props);
		break;
	default:
		fprintf(stderr, "Unknown scene component\n");
		exit(1);
	}
}

struct object object_from_definition_list(int type, struct vector* props) {
	/* TODO: Proper error management */
	switch (type) {
	case OBJ_SPHERE:
		return sphere_from_definition_list(props);
	case OBJ_BOX:
		return box_from_definition_list(props);
	case OBJ_PLANE:
		return plane_from_definition_list(props);
	case OBJ_SMOOTH_UNION:
		return smooth_union_from_definition_list(props);
	default:
		fprintf(stderr, "Unknown scene object\n");
		exit(1);
	}
}


bool scene_validate_materials(const struct scene* scene) {
	const size_t material_count = scene->materials->size;

	vector_foreach(struct object, scene->objects, obj)
		if (obj->material >= material_count)
			return false;

	return true;
}
