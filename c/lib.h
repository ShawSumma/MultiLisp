#pragma once
#include "interp.h"

pack_value pack_lib_newline(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_println(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_print(pack_state *, size_t, pack_local_value *, size_t, pack_value *);

pack_value pack_lib_lt(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_gt(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_lte(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_gte(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_neq(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_eq(pack_state *, size_t, pack_local_value *, size_t, pack_value *);

pack_value pack_lib_add(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_sub(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_mul(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_div(pack_state *, size_t, pack_local_value *, size_t, pack_value *);

pack_value pack_lib_vector(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_index(pack_state *, size_t, pack_local_value *, size_t, pack_value *);

pack_value pack_lib_ffi_index(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
pack_value pack_lib_ffi_library(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
