/* Loltracer scene description (.lol files) syntax. */

%code requires {
	#include "vector.h"
	#include "vec.h"

	struct definition_value {
		int	type;
		union {
			float		num;
			struct vector*	list;
			size_t		id;
		};
	};

	struct definition {
		int			prop;
		struct definition_value	value;
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
		int type;
		v3	point;
		size_t	material;
		union {
			struct {
				float	radius;
			} sphere;

			struct {
				v3	point2;
				float	radius;
			} box;
		};
	};

	struct camera {
		v3	point;
		v3	nw_corner;
		v3	se_corner;
	};

	struct scene {
		struct vector*	materials;
		v3		ambient_color;
		struct vector*	lights;
		struct vector*	objects;
		struct camera	camera;
	};

	struct scene* scene_new();
	void scene_free(struct scene*);
	void scene_add_component_from_definition_list(struct scene*, int,
		struct vector*);

	struct material material_from_definition_list(struct vector*);

	extern int yylex(void);
	extern void yyerror(struct scene**, const char*);
}

%define api.value.type union
%token	<float>		NUM
%token	<size_t>	ID

/* Section tokens */
%token	MATERIALS	"materials"
%token	SCENE		"scene"

/* Type tokens */
%token	AMBIENT		"ambient"
%token	CAMERA     	"camera"
%token	POINT_LIGHT	"point_light"
%token	SPHERE     	"sphere"
%token	BOX        	"box"
%token	PLANE      	"plane"

/* Property tokens */
%token	SHININESS         	"shininess"
%token	DIFFUSE           	"diffuse"
%token	SPECULAR          	"specular"
/* 	AMBIENT			"ambient" */
%token	COLOR             	"color"
%token	POINT             	"point"
%token	NW_CORNER         	"nw_corner"
%token	SE_CORNER         	"se_corner"
%token	DIFFUSE_INTENSITY 	"diffuse_intensity"
%token	SPECULAR_INTENSITY	"specular_intensity"
%token	RADIUS            	"radius"
%token	MATERIAL          	"material"
%token	POINT2            	"point2"
%token	Y                 	"y"

/* Vector of numbers */
%nterm	<struct vector*>	num_list
%nterm	<struct vector*>	numbers

/* Vector of materials */
%nterm	<struct vector*>	materials
%nterm	<struct vector*>	material_list

/* Vector of definitions */
%nterm	<struct vector*>	definition_list

%nterm	<struct material>		material
%nterm	<struct scene*>			scene
%nterm	<struct scene*>			scene_info
%nterm	<struct definition_value>	value
%nterm	<int>				type
%nterm	<int>				property

%parse-param {struct scene** scene}

%% /* Grammar rules and actions follow. */
input:
	materials scene
		{ vector_free($2->materials);
		  $2->materials = $1;
		  *scene = $2; }
;

materials:
	MATERIALS '{' material_list '}'
		{ $$ = $3; }
;

scene:
	SCENE '{' scene_info '}'
		{ $$ = $3; }
;

material_list:
	material_list ',' material
		{ vector_add(struct material, $1) = $3;
		  $$ = $1; }
|	material
		{ $$ = vector_new(struct material, 16);
		  vector_add(struct material, $$) = $1; }
;

material:
	'{' definition_list '}'
		{ $$ = material_from_definition_list($2); }
;

scene_info:
	scene_info ',' type '{' definition_list '}'
		{ scene_add_component_from_definition_list($1, $3, $5);
		  $$ = $1; }
|	type '{' definition_list '}'
		{ $$ = scene_new();
		  scene_add_component_from_definition_list($$, $1, $3); }
;

definition_list:
	definition_list ',' property '=' value
		{ vector_add(struct definition, $1) =
			(struct definition) {$3, $5};
		  $$ = $1;  }
|	property '=' value
		{ $$ = vector_new(struct definition, 16);
		  vector_add(struct definition, $$) =
			(struct definition) {$1, $3}; }
;

value:
	NUM
		{ $$ = (struct definition_value) { .type = 0, .num = $1 }; }
|	numbers
		{ $$ = (struct definition_value) { .type = 1, .list = $1 }; }
|	ID
		{ $$ = (struct definition_value) { .type = 2, .id = $1}; }
;

numbers:
	'(' num_list ')'
		{ $$ = $2; }
;

num_list:
	num_list ',' NUM
		{ vector_add(float, $1) = $3;
		  $$ = $1; }
|	NUM
		{ $$ = vector_new(float, 4);
		  vector_add(float, $$) = $1; }
;

type:
	AMBIENT		{ $$ = AMBIENT; }
|	CAMERA		{ $$ = CAMERA; }
|	POINT_LIGHT	{ $$ = POINT_LIGHT; }
|	SPHERE		{ $$ = SPHERE; }
|	BOX		{ $$ = BOX; }
|	PLANE		{ $$ = PLANE; }
;

property:
	SHININESS		{ $$ = SHININESS; }
|	DIFFUSE			{ $$ = DIFFUSE; }
|	SPECULAR		{ $$ = SPECULAR; }
|	AMBIENT			{ $$ = AMBIENT; }
|	COLOR			{ $$ = COLOR; }
|	POINT			{ $$ = POINT; }
|	NW_CORNER		{ $$ = NW_CORNER; }
|	SE_CORNER		{ $$ = SE_CORNER; }
|	DIFFUSE_INTENSITY	{ $$ = DIFFUSE_INTENSITY; }
|	SPECULAR_INTENSITY	{ $$ = SPECULAR_INTENSITY; }
|	RADIUS			{ $$ = RADIUS; }
|	MATERIAL		{ $$ = MATERIAL; }
|	POINT2			{ $$ = POINT2; }
|	Y			{ $$ = Y; }
;

%%

void yyerror(struct scene** scene, const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

int main(int argc, char* argv[]) {
	struct scene*	scene;
	yyparse(&scene);
	return 0;
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
	case SHININESS:
		assert("Is a number" && def->value.type == 0);
		material.shininess = def->value.num;
		break;
	case DIFFUSE:
		assert("Is a vector" && def->value.type == 1);
		material.diffuse = v3_from_vector(def->value.list);
		break;
	case SPECULAR:
		assert("Is a vector" && def->value.type == 1);
		material.specular = v3_from_vector(def->value.list);
		break;
	case AMBIENT:
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
	case COLOR:
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
	case POINT:
		assert("Is a vector" && def->value.type == 1);
		camera.point = v3_from_vector(def->value.list);
		break;
	case NW_CORNER:
		assert("Is a vector" && def->value.type == 1);
		camera.nw_corner = v3_from_vector(def->value.list);
		break;
	case SE_CORNER:
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
	case POINT:
		assert("Is a vector" && def->value.type == 1);
		light.point = v3_from_vector(def->value.list);
		break;
	case DIFFUSE_INTENSITY:
		assert("Is a vector" && def->value.type == 1);
		light.diffuse_intensity = v3_from_vector(def->value.list);
		break;
	case SPECULAR_INTENSITY:
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
	struct object obj = { .type = 0 };

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case POINT:
		assert("Is a vector" && def->value.type == 1);
		obj.point = v3_from_vector(def->value.list);
		break;
	case MATERIAL:
		assert("Is an ID" && def->value.type == 2);
		obj.material = def->value.id;
		break;
	case RADIUS:
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
	struct object obj = { .type = 1 };

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case POINT:
		assert("Is a vector" && def->value.type == 1);
		obj.point = v3_from_vector(def->value.list);
		break;
	case MATERIAL:
		assert("Is an ID" && def->value.type == 2);
		obj.material = def->value.id;
		break;
	case POINT2:
		assert("Is a vector" && def->value.type == 1);
		obj.box.point2 = v3_from_vector(def->value.list);
		break;
	case RADIUS:
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
	struct object obj = { .type = 2 };

	vector_foreach(struct definition, properties, def)
	switch (def->prop) {
	case MATERIAL:
		assert("Is an ID" && def->value.type == 2);
		obj.material = def->value.id;
		break;
	case Y:
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
	case AMBIENT:
		scene->ambient_color = ambient_from_definition_list(props);
		break;
	case CAMERA:
		scene->camera = camera_from_definition_list(props);
		break;
	case POINT_LIGHT:
		vector_add(struct light, scene->lights) =
			point_light_from_definition_list(props);
		break;
	case SPHERE:
		vector_add(struct object, scene->objects) =
			sphere_from_definition_list(props);
		break;
	case BOX:
		vector_add(struct object, scene->objects) =
			box_from_definition_list(props);
		break;
	case PLANE:
		vector_add(struct object, scene->objects) =
			plane_from_definition_list(props);
		break;
	default:
		fprintf(stderr, "Unknown scene component\n");
		exit(1);
	}
}
