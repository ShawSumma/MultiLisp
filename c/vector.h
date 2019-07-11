#pragma once
#include "interp.h"

pack_vector *pack_vector_empty(pack_state *);
void pack_vector_reserve_more(pack_state *, pack_vector *, size_t);
size_t pack_vector_index_convert(pack_state *, pack_vector *, int64_t);
void pack_vector_realloc(pack_state *, pack_vector *);
pack_value pack_vector_get(pack_state *, pack_vector *, size_t);
void pack_vector_set(pack_state *, pack_vector *, size_t, pack_value);
pack_value pack_vector_last(pack_state *, pack_vector *);
void pack_vector_push(pack_state *, pack_vector *, pack_value);
void pack_vector_pop(pack_state *, pack_vector *);
pack_value pack_vector_popret(pack_state *, pack_vector *);
void pack_vector_merge(pack_state *, pack_vector *, pack_vector *);