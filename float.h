#ifndef __FLOAT_H__
#define __FLOAT_H__

#include <math.h>

static inline
float clamp(float v, float min, float max) {
	v = fmaxf(v, min);
	return fminf(v, max);
}

static inline
float lerp(float from, float to, float ratio) {
	return from + (to - from) * ratio;
}

static inline
float sminf(float a, float b, float k) {
	float h = clamp(.5f + .5f * (b - a) / k, 0.f, 1.f);
	return lerp(b, a, h) - k * h * (1.f - h);
}

#endif /* __FLOAT_H__ */

