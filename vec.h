#ifndef __VEC_H__
#define __VEC_H__

#include <math.h>
#include "float.h"

typedef struct v2 {
	float x;
	float y;
} v2;
static inline v2 v2add(v2 a, v2 b)
{ return (v2){ a.x + b.x, a.y + b.y }; }
static inline v2 v2sub(v2 a, v2 b)
{ return (v2){ a.x - b.x, a.y - b.y }; }
static inline v2 v2mul(v2 a, v2 b)
{ return (v2){ a.x * b.x, a.y * b.y }; }
static inline v2 v2div(v2 a, v2 b)
{ return (v2){ a.x / b.x, a.y / b.y }; }
static inline float v2dot(v2 a, v2 b)
{ return a.x * b.x + a.y * b.y; }
static inline float v2len(v2 a)
{ return sqrtf(a.x * a.x + a.y * a.y); }
static inline v2 v2fill(float v)
{ return (v2){v, v}; }
static inline v2 v2scale(v2 v, float f)
{ return (v2){v.x * f, v.y * f}; }
static inline v2 v2normalize(v2 v)
{ return v2scale(v, 1 / v2len(v)); }

typedef struct v3 {
	float x;
	float y;
	float z;
	float _unused;
} v3;
static inline v3 v3add(v3 a, v3 b)
{ return (v3){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z}; }
static inline v3 v3sub(v3 a, v3 b)
{ return (v3){.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z}; }
static inline v3 v3mul(v3 a, v3 b)
{ return (v3){.x = a.x * b.x, .y = a.y * b.y, .z = a.z * b.z}; }
static inline v3 v3div(v3 a, v3 b)
{ return (v3){.x = a.x / b.x, .y = a.y / b.y, .z = a.z / b.z}; }
static inline float v3dot(v3 a, v3 b)
{ return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline float v3len(v3 a)
{ return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z); }
static inline v3 v3fill(float v)
{ return (v3){v, v, v}; }
static inline v3 v3scale(v3 v, float f)
{ return (v3){.x = v.x * f, .y = v.y * f, .z = v.z * f}; }
static inline v3 v3normalize(v3 v)
{ return v3scale(v, 1 / v3len(v)); }
static const int abs_mask = 0x7FFFFFFF;
static inline v3 v3abs(v3 v)
{ return (v3){ .x = fabs(v.x), .y = fabs(v.y), .z = fabs(v.z) }; }
static inline v3 v3clamp(v3 v, float min, float max)
{ return(v3){ .x = fmax(fmin(v.x, max), min), .y = fmax(fmin(v.y, max), min), .z = fmax(fmin(v.z, max), min) }; }
static inline v3 v3pow(v3 v, float pow)
{ return (v3){ powf(v.x, pow), powf(v.y, pow), powf(v.z, pow) }; }
static inline v3 v3cross(v3 a, v3 b)
{ return (v3){ a.y * b.z - a.z * b.y,
               a.z * b.x - a.x * b.z,
               a.x * b.y - a.y * b.x }; }

#endif /* __VEC_H__ */
