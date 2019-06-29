#include "interp.h"
#include "lib.h"

pack_value *pack_lib_println(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    pack_clib_println(argc, argv);
    return pack_value_nil(state);
}

pack_value *pack_lib_add(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    double d = 0;
    for (size_t i = 0; i < argc; i++) {
        d += argv[i]->value.n;
    }
    return pack_value_num(state, d);
}

pack_value *pack_lib_sub(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    double d = argv[0]->value.n;
    for (size_t i = 1; i < argc; i++) {
        d -= argv[i]->value.n;
    }
    return pack_value_num(state, d);
}

pack_value *pack_lib_mul(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    double d = 1;
    for (size_t i = 0; i < argc; i++) {
        d *= argv[i]->value.n;
    }
    return pack_value_num(state, d);
}

pack_value *pack_lib_div(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    double d = argv[0]->value.n;
    for (size_t i = 1; i < argc; i++) {
        d /= argv[i]->value.n;
    }
    return pack_value_num(state, d);
}

pack_value *pack_lib_lt(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    return pack_value_bool(state, pack_clib_lt(argc, argv));
}

pack_value *pack_lib_gt(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    return pack_value_bool(state, pack_clib_gt(argc, argv));
}

pack_value *pack_lib_lte(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    return pack_value_bool(state, pack_clib_lte(argc, argv));
}

pack_value *pack_lib_gte(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    return pack_value_bool(state, pack_clib_gte(argc, argv));
}

pack_value *pack_lib_eq(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    return pack_value_bool(state, pack_clib_eq(argc, argv));
}

pack_value *pack_lib_neq(pack_state *state,  uint16_t capc, pack_value **cap, uint16_t argc, pack_value **argv) {
    return pack_value_bool(state, pack_clib_neq(argc, argv));
}
