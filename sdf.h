#ifndef __SDF_H__
#define __SDF_H__

#include <math.h>

#include "vec.h"

/* From https://iquilezles.org/www/articles/distfunctions/distfunctions.htm */
static inline float sdSphere(v3 p, float s) {
	return v3len(p) - s;
}

static inline float sdBox(v3 p, v3 b) {
	v3 q = v3sub(v3abs(p), b);
	v3 clamped_q = { fmaxf(q.x, 0), fmaxf(q.y, 0), fmaxf(q.z, 0) };
	return v3len(clamped_q) + fminf(fmaxf(q.x, fmaxf(q.y, q.z)), 0.0);
}

static inline float sdRoundBox(v3 p, v3 b, float r) {
	v3 q = v3sub(v3abs(p), b);
	v3 clamped_q = { fmaxf(q.x, 0), fmaxf(q.y, 0), fmaxf(q.z, 0) };
	return v3len(clamped_q) + fminf(fmaxf(q.x, fmaxf(q.y, q.z)), 0.0) - r;
}

#endif /* __SDF_H__ */
