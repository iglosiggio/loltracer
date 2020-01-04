/* Loltracer scene description (.lol files) syntax. */

%code requires {
	#include <stdio.h>
	#include "scene.h"

	extern FILE* yyin;

	struct scene* scene_parse(const char*);

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
		{ $$ = (struct definition_value) {
			.type = VAL_NUM,
			.num = $1 }; }
|	numbers
		{ $$ = (struct definition_value) {
			.type = VAL_LIST,
			.list = $1 }; }
|	ID
		{ $$ = (struct definition_value) {
			.type = VAL_ID,
			.id = $1}; }
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
	AMBIENT		{ $$ = OBJ_AMBIENT; }
|	CAMERA		{ $$ = OBJ_CAMERA; }
|	POINT_LIGHT	{ $$ = OBJ_POINT_LIGHT; }
|	SPHERE		{ $$ = OBJ_SPHERE; }
|	BOX		{ $$ = OBJ_BOX; }
|	PLANE		{ $$ = OBJ_PLANE; }
;

property:
	SHININESS		{ $$ = PROP_SHININESS; }
|	DIFFUSE			{ $$ = PROP_DIFFUSE; }
|	SPECULAR		{ $$ = PROP_SPECULAR; }
|	AMBIENT			{ $$ = PROP_AMBIENT; }
|	COLOR			{ $$ = PROP_COLOR; }
|	POINT			{ $$ = PROP_POINT; }
|	NW_CORNER		{ $$ = PROP_NW_CORNER; }
|	SE_CORNER		{ $$ = PROP_SE_CORNER; }
|	DIFFUSE_INTENSITY	{ $$ = PROP_DIFFUSE_INTENSITY; }
|	SPECULAR_INTENSITY	{ $$ = PROP_SPECULAR_INTENSITY; }
|	RADIUS			{ $$ = PROP_RADIUS; }
|	MATERIAL		{ $$ = PROP_MATERIAL; }
|	POINT2			{ $$ = PROP_POINT2; }
|	Y			{ $$ = PROP_Y; }
;

%%

void yyerror(struct scene** scene, const char* msg) {
    fprintf(stderr, "Error: %s\n", msg);
}

struct scene* scene_parse(const char* filename) {
	struct scene*	scene;

	if (filename)
		yyin = fopen(filename, "r");
	else
		yyin = stdin;

	if (yyin == NULL)
		return NULL;

	yyparse(&scene);

	return scene;
}
