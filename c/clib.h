#pragma once
#include "interp.h"

void pack_clib_println(uint16_t, pack_value *);
bool pack_clib_lt(uint16_t, pack_value *);
bool pack_clib_gt(uint16_t, pack_value *);
bool pack_clib_lte(uint16_t, pack_value *);
bool pack_clib_gte(uint16_t, pack_value *);
bool pack_clib_neq(uint16_t, pack_value *);
bool pack_clib_eq(uint16_t, pack_value *);
