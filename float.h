#ifndef __FLOAT_H__
#define __FLOAT_H__

#include <xmmintrin.h>

static inline
float maxf(float a, float b) {
	return _mm_cvtss_f32(_mm_max_ss(_mm_set1_ps(a), _mm_set1_ps(b)));
}

static inline
float minf(float a, float b) {
	return _mm_cvtss_f32(_mm_min_ss(_mm_set1_ps(a), _mm_set1_ps(b)));
}

static inline
float clamp(float v, float min, float max) {
	return _mm_cvtss_f32(
		_mm_min_ss(_mm_max_ss(_mm_set1_ps(v), _mm_set1_ps(min)),
		           _mm_set1_ps(max))
	);
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

