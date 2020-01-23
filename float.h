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

/* Code from the LLVM source tree:
 * https://github.com/llvm-mirror/clang/commit/4cab28e73ee08b69fb97f11ccbd887d8ee3d524e */
/** Cast a 32-bit float value to a 32-bit unsigned integer value
 *
 *  \headerfile <x86intrin.h>
 *  This intrinsic corresponds to the <c> VMOVD / MOVD </c> instruction in x86_64,
 *  and corresponds to the <c> VMOVL / MOVL </c> instruction in ia32.
 *
 *  \param __A
 *     A 32-bit float value.
 *  \returns a 32-bit unsigned integer containing the converted value.
 */
static inline
unsigned int _castf32_u32(float __A) {
	unsigned int D;
	__builtin_memcpy(&D, &__A, sizeof(__A));
	return D;
}

#endif /* __FLOAT_H__ */

