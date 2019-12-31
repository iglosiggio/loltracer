/* Reverse Polish Notation calculator. */

%{
	#include <stdio.h>
	
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

%nterm	<const char*>	type
%nterm	<const char*>	property

%% /* Grammar rules and actions follow. */
input:
	materials scene
;

materials:
	MATERIALS '{' material_list '}' { puts("materials"); }
;

scene:
	SCENE '{' scene_info '}' { puts("materials"); }
;

material_list:
	material ',' material_list	{ puts("material"); }
|	material			{ puts("material"); }
;

material:
	'{' definition_list '}' { puts("material"); }
;

scene_info:
	scene_info ',' type '{' definition_list '}'	{ puts("data"); }
|	type '{' definition_list '}'			{ puts("data"); }
;

definition_list:
	definition_list ',' property '=' value	{ printf("assignation to %s\n", $3); }
|	property '=' value			{ printf("assignation to %s\n", $1); }
;

value:
	NUM	{ printf("number %f\n", $1); }
|	numbers	{ puts("list value"); }
;

numbers:
	'(' num_list ')'	{ puts("number list"); }
;

num_list:
	num_list ',' NUM	{ printf("number %f\n", $3); }
|	NUM			{ printf("number %f\n", $1); }
;

type:
	AMBIENT		{ $$ = "ambient"; }
|	CAMERA		{ $$ = "camera"; }
|	POINT_LIGHT	{ $$ = "point_light"; }
|	SPHERE		{ $$ = "sphere"; }
|	BOX		{ $$ = "box"; }
|	PLANE		{ $$ = "plane"; }
;

property:
	ID			{ $$ = "id"; }
|	SHININESS		{ $$ = "shininess"; }
|	DIFFUSE			{ $$ = "diffuse"; }
|	SPECULAR		{ $$ = "specular"; }
|	AMBIENT			{ $$ = "ambient"; }
|	COLOR			{ $$ = "color"; }
|	POINT			{ $$ = "point"; }
|	NW_CORNER		{ $$ = "nw_corner"; }
|	SE_CORNER		{ $$ = "se_corner"; }
|	DIFFUSE_INTENSITY	{ $$ = "diffuse_intensity"; }
|	SPECULAR_INTENSITY	{ $$ = "specular_intensity"; }
|	RADIUS			{ $$ = "radius"; }
|	MATERIAL		{ $$ = "material"; }
|	POINT2			{ $$ = "point2"; }
|	Y			{ $$ = "y"; }
;

%%

int main(int argc, char* argv[]) {
	yyparse();
	return 0;
}
