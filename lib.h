#pragma once
#include "interp.h"

value lib_println(program, uint16_t, value *);
value lib_lt(program, uint16_t, value *);
value lib_gt(program, uint16_t, value *);
value lib_lte(program, uint16_t, value *);
value lib_gte(program, uint16_t, value *);
value lib_neq(program, uint16_t, value *);
value lib_eq(program, uint16_t, value *);
value lib_add(program, uint16_t, value *);
value lib_sub(program, uint16_t, value *);
value lib_mul(program, uint16_t, value *);
value lib_div(program, uint16_t, value *);