#include "interp.h"

pack_number pack_number_new_bigint(char *str) {
    pack_number ret;
    ret.type = PACK_NUMBER_TYPE_INTEGER;
    mpz_init(ret.value.i);
    mpz_set_str(ret.value.i, str, 10);
    return ret;
}

pack_number pack_number_new_bigfloat(char *str) {
    pack_number ret;
    ret.type = PACK_NUMBER_TYPE_FLOATING;
    mpf_init(ret.value.f);
    mpf_set_str(ret.value.f, str, 10);
    return ret;
}

pack_number pack_number_new_double(double d) {
    pack_number ret;
    ret.type = PACK_NUMBER_TYPE_FLOATING;
    mpf_init(ret.value.f);
    mpf_set_d(ret.value.f, d);
    return ret;
}

pack_number pack_number_new_int64(int64_t i) {
    pack_number ret;
    ret.type = PACK_NUMBER_TYPE_INTEGER;
    mpz_init(ret.value.i);
    mpz_set_si(ret.value.i, i);
    return ret;
}

pack_number pack_number_new_uint64(uint64_t i) {
    pack_number ret;
    ret.type = PACK_NUMBER_TYPE_INTEGER;
    mpz_init(ret.value.i);
    mpz_set_ui(ret.value.i, i);
    return ret;
}

int64_t pack_number_to_int64(pack_number n) {
    switch (n.type) {
        case PACK_NUMBER_TYPE_FLOATING: {
            if (!mpf_fits_slong_p(n.value.f)) {
                gmp_fprintf(stderr, "%Ff cannot fit in any int64\n", n.value.f);
                exit(1);
            }
            return mpf_get_si(n.value.f);
        }
        case PACK_NUMBER_TYPE_INTEGER: {
            if (!mpz_fits_slong_p(n.value.i)) {
                gmp_fprintf(stderr, "%Zd cannot fit in any int64\n", n.value.i);
                exit(1);
            }
            return mpz_get_si(n.value.i);
        }
    }
}

uint64_t pack_number_to_uint64(pack_number n) {
    switch (n.type) {
        case PACK_NUMBER_TYPE_FLOATING: {
            if (!mpf_fits_ulong_p(n.value.f)) {
                gmp_fprintf(stderr, "%Ff cannot fit in any uint64\n", n.value.f);
                exit(1);
            }
            return mpf_get_ui(n.value.f);
        }
        case PACK_NUMBER_TYPE_INTEGER: {
            if (!mpz_fits_ulong_p(n.value.i)) {
                gmp_fprintf(stderr, "%Zd cannot fit in any uint64\n", n.value.i);
                exit(1);
            }
            return mpz_get_ui(n.value.i);
        }
    }
}

double pack_number_to_double(pack_number n) {
    switch (n.type) {
        case PACK_NUMBER_TYPE_FLOATING: {
            return mpf_get_d(n.value.f);
        }
        case PACK_NUMBER_TYPE_INTEGER: {
            return mpz_get_d(n.value.i);
        }
    }
}

pack_number pack_number_add(pack_number lhs, pack_number rhs) {
    pack_number ret;
    switch (lhs.type) {
        case PACK_NUMBER_TYPE_FLOATING: {
            switch (rhs.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    mpf_add(ret.value.f, lhs.value.f, rhs.value.f);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    mpf_t rhsf;
                    mpf_init(rhsf);
                    mpf_set_z(rhsf, rhs.value.i);
                    mpf_init(ret.value.f);
                    mpf_add(ret.value.f, lhs.value.f, rhsf);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
            }
            break;
        }
        case PACK_NUMBER_TYPE_INTEGER: {
            switch (rhs.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    mpf_t lhsf;
                    mpf_init(lhsf);
                    mpf_set_z(lhsf, lhs.value.i);
                    mpf_init(ret.value.f);
                    mpf_add(ret.value.f, lhsf, rhs.value.f);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    mpz_init(ret.value.i);
                    mpz_add(ret.value.i, lhs.value.i, rhs.value.i);
                    ret.type = PACK_NUMBER_TYPE_INTEGER;
                    break;
                }
            }
            break;
        }
    }
    return ret;
}

pack_number pack_number_mul(pack_number lhs, pack_number rhs) {
    pack_number ret;
    switch (lhs.type) {
        case PACK_NUMBER_TYPE_FLOATING: {
            switch (rhs.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    mpf_init(ret.value.f);
                    mpf_mul(ret.value.f, lhs.value.f, rhs.value.f);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    mpf_t rhsf;
                    mpf_init(rhsf);
                    mpf_set_z(rhsf, rhs.value.i);
                    mpf_init(ret.value.f);
                    mpf_mul(ret.value.f, lhs.value.f, rhsf);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
            }
            break;
        }
        case PACK_NUMBER_TYPE_INTEGER: {
            switch (rhs.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    mpf_t lhsf;
                    mpf_init(lhsf);
                    mpf_set_z(lhsf, lhs.value.i);
                    mpf_init(ret.value.f);
                    mpf_mul(ret.value.f, lhsf, rhs.value.f);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    mpz_init(ret.value.i);
                    mpz_mul(ret.value.i, lhs.value.i, rhs.value.i);
                    ret.type = PACK_NUMBER_TYPE_INTEGER;
                    break;
                }
            }
            break;
        }
    }
    return ret;
}

pack_number pack_number_sub(pack_number lhs, pack_number rhs) {
    pack_number ret;
    switch (lhs.type) {
        case PACK_NUMBER_TYPE_FLOATING: {
            switch (rhs.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    mpf_init(ret.value.f);
                    mpf_sub(ret.value.f, lhs.value.f, rhs.value.f);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    mpf_t rhsf;
                    mpf_init(rhsf);
                    mpf_set_z(rhsf, rhs.value.i);
                    mpf_init(ret.value.f);
                    mpf_sub(ret.value.f, lhs.value.f, rhsf);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
            }
            break;
        }
        case PACK_NUMBER_TYPE_INTEGER: {
            switch (rhs.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    mpf_t lhsf;
                    mpf_init(lhsf);
                    mpf_set_z(lhsf, lhs.value.i);
                    mpf_init(ret.value.f);
                    mpf_sub(ret.value.f, lhsf, rhs.value.f);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    mpz_init(ret.value.i);
                    mpz_sub(ret.value.i, lhs.value.i, rhs.value.i);
                    ret.type = PACK_NUMBER_TYPE_INTEGER;
                    break;
                }
            }
            break;
        }
    }
    return ret;
}

pack_number pack_number_div(pack_number lhs, pack_number rhs) {
    pack_number ret;
    switch (lhs.type) {
        case PACK_NUMBER_TYPE_FLOATING: {
            switch (rhs.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    mpf_init(ret.value.f);
                    mpf_div(ret.value.f, lhs.value.f, rhs.value.f);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    mpf_t rhsf;
                    mpf_init(rhsf);
                    mpf_set_z(rhsf, rhs.value.i);
                    mpf_init(ret.value.f);
                    mpf_div(ret.value.f, lhs.value.f, rhsf);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
            }
            break;
        }
        case PACK_NUMBER_TYPE_INTEGER: {
            switch (rhs.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    mpf_t lhsf;
                    mpf_init(lhsf);
                    mpf_set_z(lhsf, lhs.value.i);
                    mpf_init(ret.value.f);
                    mpf_div(ret.value.f, lhsf, rhs.value.f);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    mpf_t lhsf;
                    mpf_t rhsf;
                    mpf_init(lhsf);
                    mpf_init(rhsf);
                    mpf_set_z(rhsf, rhs.value.i);
                    mpf_set_z(lhsf, lhs.value.i);
                    mpf_init(ret.value.f);
                    mpf_div(ret.value.f, lhsf, rhsf);
                    ret.type = PACK_NUMBER_TYPE_FLOATING;
                    break;
                }
            }
            break;
        }
    }
    return ret;
}

int pack_number_cmp(pack_number lhs, pack_number rhs) {
    switch (lhs.type) {
        case PACK_NUMBER_TYPE_FLOATING: {
            switch (rhs.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    return mpf_cmp(lhs.value.f, rhs.value.f);
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    mpf_t rhsf;
                    mpf_init(rhsf);
                    mpf_set_z(rhsf, rhs.value.i);
                    return mpf_cmp(lhs.value.f, rhsf);
                }
            }
        }
        case PACK_NUMBER_TYPE_INTEGER: {
            switch (rhs.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    mpf_t lhsf;
                    mpf_init(lhsf);
                    mpf_set_z(lhsf, lhs.value.i);
                    return mpf_cmp(lhsf, rhs.value.f);
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    return mpz_cmp(lhs.value.i, rhs.value.i);
                }
            }
        }
    }
}
