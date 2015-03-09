#include <malloc.h>

#ifndef VECTOR_TYPE
#define VECTOR_TYPE char
#endif

int vectorCheckNull(void *pointer) {
    if (pointer == NULL) {
        fprintf(stderr, "Wrong memory alloc.\n");
        return 1;
    } else
        return 0;
}

struct vector {
    size_t size;
    size_t capacity;
    VECTOR_TYPE **array;
};

int vectorAdd(struct vector *vect, VECTOR_TYPE *item) {
    if (vect->size == vect->capacity) {
        vect->capacity *= 2;
        vect->array = realloc(vect->array, vect->capacity * sizeof(VECTOR_TYPE *));
        if (vectorCheckNull(vect->array)) {
            return 1;
        }
    }
    vect->array[vect->size++] = item;
    return 0;
}

void vectorClear(struct vector *vect) {
    for (size_t i = 0; i < vect->size; ++i) {
        free(vect->array[i]);
    }
    vect->size = 0;
}

void vectorFree(struct vector *vect) {
    for (size_t i = 0; i < vect->size; ++i) {
        free(vect->array[i]);
    }
    free(vect->array);
    free(vect);
}

struct vector *vectorCreate() {
    struct vector *value = malloc(sizeof(struct vector));
    if (value != NULL) {
        value->capacity = 10;
        value->size = 0;
        value->array = malloc(value->capacity * sizeof(VECTOR_TYPE *));
        if (!(value->array)) {
            free(value);
            value = NULL;
        }
    }
    return value;
}