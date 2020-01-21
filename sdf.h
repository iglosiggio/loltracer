#ifndef __SDF_H__
#define __SDF_H__

#include "vec.h"
#include "float.h"

/* From https://iquilezles.org/www/articles/distfunctions/distfunctions.htm */
static inline float sdSphere(v3 p, float s) {
	return v3len(p) - s;
}

static inline float sdBox(v3 p, v3 b) {
	v3 q = v3sub(v3abs(p), b);
	v3 clamped_q = { maxf(q.x, 0.f), maxf(q.y, 0.f), maxf(q.z, 0.f) };
	return v3len(clamped_q) + minf(maxf(q.x, maxf(q.y, q.z)), 0.f);
}

static inline float sdRoundBox(v3 p, v3 b, float r) {
	v3 q = v3sub(v3abs(p), b);
	v3 clamped_q = { maxf(q.x, 0.f), maxf(q.y, 0.f), maxf(q.z, 0.f) };
	return v3len(clamped_q) + minf(maxf(q.x, maxf(q.y, q.z)), 0.f) - r;
}

#endif /* __SDF_H__ */
