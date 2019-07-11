#pragma once
#include "interp.h"

void pack_clib_print(pack_value);
bool pack_clib_lt(size_t, pack_value *);
bool pack_clib_gt(size_t, pack_value *);
bool pack_clib_lte(size_t, pack_value *);
bool pack_clib_gte(size_t, pack_value *);
bool pack_clib_neq(size_t, pack_value *);
bool pack_clib_eq(size_t, pack_value *);
