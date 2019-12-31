/* Reverse Polish Notation calculator. */

%{
	#include <stdio.h>
	#include "vector.h"
	
	extern int yylex(void);
	extern void yyerror(const char*);
%}

%define api.value.type union
%token	<float>		NUM

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
%token	ID                	"id"
%token	SHININESS         	"shininess"
%token	DIFFUSE           	"diffuse"
%token	SPECULAR          	"specular"
/* 	AMBIENT			"ambient"*/
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

/* Vector of scene objects */
%nterm	<struct vector*>	scene_info

/* Vector of definitions */
%nterm	<struct vector*>	definition_list

%nterm	<struct material>		material
%nterm	<struct scene*>			scene
%nterm	<struct definition_value>	value
%nterm	<int>				type
%nterm	<int>				property

%% /* Grammar rules and actions follow. */
input:
	materials scene
;

materials:
	MATERIALS '{' material_list '}'
		{ $$ = $3; }
;

scene:
	SCENE '{' scene_info '}'
		{ $$ = scene_from_object_list($3); }
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
		{ vector_add(struct scene_object, $1) =
			scene_object_from_definition_list($3, $5);
		  $$ = $1; }
|	type '{' definition_list '}'
		{ $$ = vector_new(struct scene_object, 16);
		  vector_add(struct scene_object, $1) =
			scene_object_from_definition_list($1, $3); }
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
		{ $$ = (struct definition_value) { .TYPE = 0, .num = $1 }; }
|	numbers
		{ $$ = (struct definition_value) { .TYPE = 1, .list = $1 }; }
;

numbers:
	'(' num_list ')'
		{ $$ = $2 }
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
	ID			{ $$ = ID; }
|	SHININESS		{ $$ = SHININESS; }
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

int main(int argc, char* argv[]) {
	yyparse();
	return 0;
}
