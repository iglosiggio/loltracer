%{
	#include "scene-parser.h"
%}

%option noyywrap

%%

[ \r\n\t]*	{ continue; /* Skip blanks. */ }
[-.0-9]+	{ sscanf(yytext, "%f", &yylval.NUM); return NUM; }
#[0-9]+		{ sscanf(yytext + 1, "%d", &yylval.ID); return ID; }

"materials"	{ return MATERIALS; }
"scene"		{ return SCENE; }

"ambient"	{ return AMBIENT; }
"camera"	{ return CAMERA; }
"point-light"	{ return POINT_LIGHT; }
"point_light"	{ return POINT_LIGHT; }
"sphere"	{ return SPHERE; }
"box"		{ return BOX; }
"plane"		{ return PLANE; }

"shininess"		{ return SHININESS; }
"diffuse"		{ return DIFFUSE; }
"specular"		{ return SPECULAR; }
 /* "ambient"		{ return AMBIENT; } */
"color"			{ return COLOR; }
"point"			{ return POINT; }
"nw_corner"		{ return NW_CORNER; }
"nw-corner"		{ return NW_CORNER; }
"se_corner"		{ return SE_CORNER; }
"se-corner"		{ return SE_CORNER; }
"diffuse_intensity"	{ return DIFFUSE_INTENSITY; }
"diffuse-intensity"	{ return DIFFUSE_INTENSITY; }
"specular_intensity"	{ return SPECULAR_INTENSITY; }
"specular-intensity"	{ return SPECULAR_INTENSITY; }
"radius"		{ return RADIUS; }
"material"		{ return MATERIAL; }
"point2"		{ return POINT2; }
"y"			{ return Y; }

[,(){}=]	{ return yytext[0]; }

.	{ continue; /* Ignore unexpected characters. */ }

%%
