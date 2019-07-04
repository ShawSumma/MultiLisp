#pragma once
#include "interp.h"

pack_value pack_lib_println(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
pack_value pack_lib_lt(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
pack_value pack_lib_gt(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
pack_value pack_lib_lte(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
pack_value pack_lib_gte(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
pack_value pack_lib_neq(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
pack_value pack_lib_eq(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
pack_value pack_lib_add(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
pack_value pack_lib_sub(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
pack_value pack_lib_mul(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
pack_value pack_lib_div(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
