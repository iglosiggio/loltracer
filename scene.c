#include <stdio.h>
#include "scene.h"

struct scene* scene_new() {
	struct scene* scene = malloc(sizeof(struct scene));
	*scene = (struct scene) {
		.materials	= vector_new(struct material, 16),
		.ambient_color	= (v3) {0, 0, 0},
		.lights		= vector_new(struct light, 16),
		.objects	= vector_new(struct object, 16),
		.camera		= (struct camera) {
			.point		= (v3) {0, 0, 0},
			.nw_corner	= (v3) {-1, 1, 1},
			.se_corner	= (v3) {1, -1, 1}
		}
	};
	return scene;
}

void scene_free(struct scene* scene) {
	vector_free(scene->materials);
	vector_free(scene->lights);
	vector_free(scene->objects);
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


struct material material_from_definition_list(struct vector* properties) {
	/* TODO: Proper error management */
	struct material material;

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case PROP_SHININESS:
		assert("Is a number" && def->value.type == 0);
		material.shininess = def->value.num;
		break;
	case PROP_DIFFUSE:
		assert("Is a vector" && def->value.type == 1);
		material.diffuse = v3_from_vector(def->value.list);
		break;
	case PROP_SPECULAR:
		assert("Is a vector" && def->value.type == 1);
		material.specular = v3_from_vector(def->value.list);
		break;
	case PROP_AMBIENT:
		assert("Is a vector" && def->value.type == 1);
		material.ambient = v3_from_vector(def->value.list);
		break;
	default:
		fprintf(stderr, "Unknown material property\n");
		exit(1);
	}

	return material;
}

struct v3 ambient_from_definition_list(struct vector* properties) {
	/* TODO: Proper error management */
	v3 ambient_color;

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case PROP_COLOR:
		assert("Is a vector" && def->value.type == 1);
		ambient_color = v3_from_vector(def->value.list);
		break;
	default:
		fprintf(stderr, "Unknown ambient property\n");
		exit(1);
	}

	return ambient_color;
}

struct camera camera_from_definition_list(struct vector* properties) {
	/* TODO: Proper error management */
	struct camera camera;

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case PROP_POINT:
		assert("Is a vector" && def->value.type == 1);
		camera.point = v3_from_vector(def->value.list);
		break;
	case PROP_NW_CORNER:
		assert("Is a vector" && def->value.type == 1);
		camera.nw_corner = v3_from_vector(def->value.list);
		break;
	case PROP_SE_CORNER:
		assert("Is a vector" && def->value.type == 1);
		camera.se_corner = v3_from_vector(def->value.list);
		break;
	default:
		fprintf(stderr, "Unknown camera property\n");
		exit(1);
	}

	return camera;
}

struct light point_light_from_definition_list(struct vector* properties) {
	/* TODO: Proper error management */
	struct light light;

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case PROP_POINT:
		assert("Is a vector" && def->value.type == 1);
		light.point = v3_from_vector(def->value.list);
		break;
	case PROP_DIFFUSE_INTENSITY:
		assert("Is a vector" && def->value.type == 1);
		light.diffuse_intensity = v3_from_vector(def->value.list);
		break;
	case PROP_SPECULAR_INTENSITY:
		assert("Is a vector" && def->value.type == 1);
		light.specular_intensity = v3_from_vector(def->value.list);
		break;
	default:
		fprintf(stderr, "Unknown light property\n");
		exit(1);
	}

	return light;
}

struct object sphere_from_definition_list(struct vector* properties) {
	/* TODO: Proper error management */
	struct object obj = { .type = OBJ_SPHERE };

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case PROP_POINT:
		assert("Is a vector" && def->value.type == 1);
		obj.point = v3_from_vector(def->value.list);
		break;
	case PROP_MATERIAL:
		assert("Is an ID" && def->value.type == 2);
		obj.material = def->value.id;
		break;
	case PROP_RADIUS:
		assert("Is a number" && def->value.type == 0);
		obj.sphere.radius = def->value.num;
		break;
	default:
		fprintf(stderr, "Unknown sphere property\n");
		exit(1);
	}

	return obj;
}

struct object box_from_definition_list(struct vector* properties) {
	/* TODO: Proper error management */
	struct object obj = { .type = OBJ_BOX };

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case PROP_POINT:
		assert("Is a vector" && def->value.type == 1);
		obj.point = v3_from_vector(def->value.list);
		break;
	case PROP_MATERIAL:
		assert("Is an ID" && def->value.type == 2);
		obj.material = def->value.id;
		break;
	case PROP_POINT2:
		assert("Is a vector" && def->value.type == 1);
		obj.box.point2 = v3_from_vector(def->value.list);
		break;
	case PROP_RADIUS:
		assert("Is a number" && def->value.type == 0);
		obj.box.radius = def->value.num;
		break;
	default:
		fprintf(stderr, "Unknown box property\n");
		exit(1);
	}

	return obj;
}

struct object plane_from_definition_list(struct vector* properties) {
	/* TODO: Proper error management */
	struct object obj = { .type = OBJ_PLANE };

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case PROP_MATERIAL:
		assert("Is an ID" && def->value.type == 2);
		obj.material = def->value.id;
		break;
	case PROP_Y:
		assert("Is a number" && def->value.type == 0);
		obj.point = (v3) {0, def->value.num, 0};
		break;
	default:
		fprintf(stderr, "Unknown sphere property\n");
		exit(1);
	}

	return obj;
}

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
			point_light_from_definition_list(props);
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
	default:
		fprintf(stderr, "Unknown scene component\n");
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
