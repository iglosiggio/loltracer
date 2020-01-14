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

static inline
struct vector* _vector_new(size_t elem_size, size_t initial_capacity) {
	struct vector* rval = malloc(sizeof(struct vector));
	rval->size = 0;
	rval->capacity = initial_capacity;
	rval->elem_size = elem_size;
	rval->data = malloc(initial_capacity * elem_size);

	return rval;
}

static inline
void vector_free(struct vector* vec) {
	free(vec->data);
	free(vec);
}

static inline
void* _vector_add(struct vector* vec) {
	if (vec->size >= vec->capacity) {
		vec->capacity = vec->capacity * vec->elem_size * 2 + vec->elem_size;
		vec->data = realloc(vec->data, vec->capacity * vec->elem_size);
		assert(vec->data != NULL);
	}

	return vec->data + vec->size++ * vec->elem_size;
}

static inline
void* _vector_get(const struct vector* vec, size_t idx) {
	assert(idx < vec->size);

	return vec->data + idx * vec->elem_size;
}

#define vector_add(type, vec)		(*(type*) _vector_add((vec)))
#define vector_get(type, vec, idx) 	(*(type*) _vector_get((vec), (idx)))
#define vector_new(type, capacity)	_vector_new(sizeof(type), capacity)
#define vector_foreach(type, vec, var)	\
	for (type* var = (type*)(vec)->data; \
	     var != (type*)(vec)->data + (vec)->size; \
	     var++)

#endif /* __VECTOR_H__ */
