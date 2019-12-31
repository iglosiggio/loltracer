#ifndef __VECTOR_H__
#define __VECTOR_H__
#include <stdlib.h>
#include <assert.h>

struct vector {
	size_t size;
	size_t capacity;
	size_t elem_size;
	void* data;
};

inline
struct vector* vector_new(size_t elem_size, size_t initial_capacity) {
	struct vector* rval = malloc(sizeof(struct vector));
	rval->size = 0;
	rval->capacity = initial_capacity;
	rval->data = malloc(initial_capacity * elem_size);

	return rval;
}

inline
void vector_free(struct vector* vec) {
	free(vec->data);
	free(vec);
}

inline
void* vector_add(struct vector* vec) {
	if (vec->size >= vec->capacity) {
		vec->capacity = vec->capacity * vec->elem_size * 2;
		vec->data = realloc(vec->data, vec->capacity);
	}

	return vec->data + vec->size++ * vec->elem_size;
}

inline
void* vector_get(const struct vector* vec, size_t idx) {
	assert(idx < vec->size);

	return vec->data + idx * vec->elem_size;
}

#endif /* __VECTOR_H__ */
