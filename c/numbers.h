#pragma once
#include "interp.h"

// pack_number pack_number_new_fraction(mpz_t, mpz_t);
pack_number pack_number_new_bigint(char *);
pack_number pack_number_new_bigfloat(char *);
pack_number pack_number_new_double(double);
pack_number pack_number_new_int64(int64_t);
pack_number pack_number_new_uint64(uint64_t);

int64_t pack_number_to_int64(pack_number);
uint64_t pack_number_to_uint64(pack_number);
double pack_number_to_double(pack_number);

pack_number pack_number_add(pack_number, pack_number);
pack_number pack_number_mul(pack_number, pack_number);
pack_number pack_number_sub(pack_number, pack_number);
pack_number pack_number_div(pack_number, pack_number);

int pack_number_cmp(pack_number, pack_number);