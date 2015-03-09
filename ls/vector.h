#include <malloc.h>

#ifndef VECTOR_TYPE
#define VECTOR_TYPE char
#endif

struct vector {
    size_t size;
    size_t capacity;
    VECTOR_TYPE **array;
};

int vectorAdd(struct vector *vect, VECTOR_TYPE *item);

void vectorClear(struct vector *vect);

void vectorFree(struct vector *vect);

struct vector *vectorCreate();