#include "interp.h"

pack_vector *pack_vector_empty(pack_state *state) {
    pack_vector *ret = gc_malloc(sizeof(pack_vector));
    ret->alloc = 4;
    ret->count = 0;
    ret->values = gc_malloc(sizeof(pack_value) * ret->alloc);
    return ret;
}


void pack_vector_reserve_more(pack_state *state, pack_vector *vec, size_t more) {
    size_t newsize = vec->count + more;
    if (newsize + 4 >= vec->alloc) {
        vec->alloc = newsize * 2 + 2;
        vec->values = gc_realloc(vec->values, sizeof(pack_value) * vec->alloc);
    }
}

size_t pack_vector_index_convert(pack_state *state, pack_vector *vec, int64_t ind) {
    int64_t arg = ind;
    if (ind < 0) {
        ind += vec->count;
    }
    if (ind < 0) {
        printf("vector index too low (%ld <= %ld)\n", arg, -(int64_t)vec->count);
        exit(1);
    }
    return ind;
}

void pack_vector_realloc(pack_state *state, pack_vector *vec) {
    if (vec->count + 4 > vec->alloc) {
        vec->alloc = vec->alloc * 2 + 2;
        vec->values = gc_realloc(vec->values, sizeof(pack_value) * vec->alloc);
    }
}

pack_value pack_vector_get(pack_state *state, pack_vector *vec, size_t index) {
    if (index >= vec->count) {
        printf("vector index too high (%zu >= %zu)\n", index, vec->count);
        exit(1);
    }
    return vec->values[index];
}

void pack_vector_set(pack_state *state, pack_vector *vec, size_t index, pack_value val) {
    if (index >= vec->count) {
        printf("vector index too high (%zu >= %zu)\n", index, vec->count);
    }
    vec->values[index] = val;
}

pack_value pack_vector_last(pack_state *state, pack_vector *vec) {
    if (vec->count == 0) {
        printf("vector empty when getting last element\n");
    }
    return vec->values[vec->count-1];
}

void pack_vector_push(pack_state *state, pack_vector *vec, pack_value val) {
    pack_vector_realloc(state, vec);
    vec->values[vec->count] = val;
    vec->count ++;
}

void pack_vector_pop(pack_state *state, pack_vector *vec) {
    vec->count --;
}

pack_value pack_vector_popret(pack_state *state, pack_vector *vec) {
    vec->count --;
    return vec->values[vec->count];
}

void pack_vector_merge(pack_state *state, pack_vector *dest, pack_vector *src) {
    pack_vector_reserve_more(state, dest, src->count);
    for (size_t i = 0; i < src->count; i++) {
        dest->values[dest->count] = src->values[src->count];
        dest->count ++;
    }
}