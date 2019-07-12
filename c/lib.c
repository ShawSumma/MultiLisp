#include "interp.h"
#include "lib.h"

pack_value pack_lib_vector_range(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    pack_number n1;
    pack_number n2;
    if (argc == 2) {
        if (argv[0].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "vector-range", 0);
            exit(1);
        }
        if (argv[1].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "vector-range", 1);
            exit(1);
        }
        n1 = argv[0].value.number;
        n2 = argv[1].value.number;
    }
    else if (argc == 1) {
        if (argv[0].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "vector-range", 0);
            exit(1);
        }
        n1 = pack_number_new_int64(0);
        n2 = argv[0].value.number;
    }
    else {
        fprintf(stderr, "vector-range takes 1 or 2 arguments\n");
        exit(1);
    }
    pack_vector *vec = pack_vector_empty(state);
    size_t diff = pack_number_to_uint64(pack_number_sub(n2, n1));
    pack_vector_reserve_more(state, vec, diff);
    for (size_t i = 0; i < diff; i++) {
        vec->values[i] = pack_value_num(state, pack_number_add(n1, pack_number_new_uint64(i)));
    }
    vec->count = diff;
    pack_value ret;
    ret.type = PACK_VALUE_TYPE_VECTOR;
    ret.value.vector = vec;
    return ret;
}

pack_value pack_lib_vector_length(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc != 1) {
        fprintf(stderr, "vector-length takes 1 argument\n");
    }
    if (argv[0].type != PACK_VALUE_TYPE_VECTOR) {
        pack_error_argindex(state, "vector-length", 0);
        exit(1);
    }
    return pack_value_num(state, pack_number_new_uint64(argv[0].value.vector->count));
}

pack_value pack_lib_vector_map(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc != 2) {
        fprintf(stderr, "vector-map takes 2 arguments\n");
    }
    if (argv[0].type != PACK_VALUE_TYPE_FUNCTION) {
        pack_error_argindex(state, "vector-map", 0);
        exit(1);
    }
    if (argv[1].type != PACK_VALUE_TYPE_VECTOR) {
        pack_error_argindex(state, "vector-map", 1);
        exit(1);
    }
    pack_func *fun = argv[0].value.func;
    pack_vector *invec = argv[1].value.vector;
    pack_vector *outvec = pack_vector_empty(state);
    size_t len = invec->count;
    pack_vector_reserve_more(state, outvec, len);
    for (size_t i = 0; i < len; i++) {
        outvec->values[i] = pack_call(state, fun, 1, invec->values + i);
    }
    outvec->count = len;
    pack_value ret;
    ret.type = PACK_VALUE_TYPE_VECTOR;
    ret.value.vector = outvec;
    return ret;
}

pack_value pack_lib_vector_fold(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "vector-fold takes 2 arguments\n");
    }
    if (argv[0].type != PACK_VALUE_TYPE_FUNCTION) {
        pack_error_argindex(state, "vector-fold", 0);
        exit(1);
    }
    if (argv[argc-1].type != PACK_VALUE_TYPE_VECTOR) {
        pack_error_argindex(state, "vector-fold", argc-1);
        exit(1);
    }
    pack_func *fun = argv[0].value.func;
    pack_vector *vec = argv[argc-1].value.vector;
    pack_value cur;
    size_t begin;
    if (argc == 2) {
        cur = vec->values[0];
        begin = 1;
    }
    else {
        cur = argv[1];
        begin = 0;
    }
    for (size_t i = begin; i < vec->count; i++) {
        pack_value args[2] = {cur, vec->values[i]}; 
        cur = pack_call(state, fun, 2, args);
    }
    return cur;
}

pack_value pack_lib_ffi_index(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {    
    if (argc < 3) {
        fprintf(stderr, "ffi-index takes 3 or more arguments\n");
        exit(1);
    }
    if (argv[0].type != PACK_VALUE_TYPE_LIBRARY) {
        pack_error_argindex(state, "ffi-index", 0);
        exit(1);
    }
    if (argv[1].type != PACK_VALUE_TYPE_STRING) {
        pack_error_argindex(state, "ffi-index", 1);
        exit(1);
    }
    if (argv[2].type != PACK_VALUE_TYPE_FFI_TYPE) {
        pack_error_argindex(state, "ffi-index", 2);
        exit(1);
    }
    for (size_t i = 3; i < argc; i++) {
        if (argv[i].type != PACK_VALUE_TYPE_FFI_TYPE) {
            pack_error_argindex(state, "ffi-index", i);
            exit(1);
        }
    }
    void *sym = dlsym(argv[0].value.library, argv[1].value.string);
    if (!sym) {
        printf("no such symbol %s\n", argv[1].value.string);
        exit(1);
    }
    pack_func *fn = gc_malloc(sizeof(pack_func));
    fn->type = FUNC_FROM_FFI;
    ffi_cif cif;
    ffi_type **types = gc_malloc(sizeof(ffi_type *) * argc-3);
    pack_ffi_type *arg_types = gc_malloc(sizeof(pack_ffi_type) * argc-3);
    for (size_t i = 3; i < argc; i++) {
        types[i-3] = argv[i].value.type.type;
        arg_types[i-3] = argv[i].value.type;
    }
    ffi_prep_cif(&cif, FFI_DEFAULT_ABI, argc-3, argv[2].value.type.type, types);
    fn->value.ffi = malloc(sizeof(*fn->value.ffi));
    fn->value.ffi->cif = cif;
    fn->value.ffi->func = sym;
    fn->value.ffi->ret_type = argv[2].value.type;
    fn->value.ffi->arg_types = arg_types;
    fn->value.ffi->argc = argc-3;
    pack_value ret;
    ret.value.func = fn;
    ret.type = PACK_VALUE_TYPE_FUNCTION;
    return ret;
}


pack_value pack_lib_ffi_library(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc != 1) {
        fprintf(stderr, "ffi-library takes 1 argument\n");
        exit(1);
    }    
    if (argv[0].type != PACK_VALUE_TYPE_STRING) {
        pack_error_argindex(state, "ffi-library", 0);
        exit(1);
    }
    void *lib = dlopen(argv[0].value.string, RTLD_LAZY);
    if (!lib) {
        printf("ffi-library not found %s\n", argv[0].value.string);
        exit(1);
    }
    return pack_value_library(state, lib);
}


pack_value pack_lib_vector(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    pack_vector *vec = pack_vector_empty(state);
    pack_vector_reserve_more(state, vec, argc);
    vec->count = argc;
    for (size_t i = 0; i < argc; i++) {
        vec->values[i] = argv[i];
    }
    pack_value ret;
    ret.type = PACK_VALUE_TYPE_VECTOR;
    ret.value.vector = vec;
    return ret;
}

pack_value pack_lib_vector_index(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc < 3) {
        fprintf(stderr, "index takes 2 arguments\n");
        exit(1);
    }
    if (argv[0].type != PACK_VALUE_TYPE_VECTOR) {
        pack_error_argindex(state, "index", 0);
        exit(1);
    }
    if (argv[1].type != PACK_VALUE_TYPE_NUMBER) {
        pack_error_argindex(state, "index", 1);
        exit(1);
    }
    size_t ind = pack_vector_index_convert(state, argv[1].value.vector, pack_number_to_int64(argv[1].value.number));
    return pack_vector_get(state, argv[0].value.vector, ind);
}

pack_value pack_lib_println(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    for (size_t i = 0; i < argc; i++) {
        pack_clib_print(argv[i]);
    }
    printf("\n");
    return pack_value_nil(state);
}

pack_value pack_lib_newline(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, "newline takes no arguments\n");
        exit(1);
    }
    printf("\n");
    return pack_value_nil(state);
}

pack_value pack_lib_print(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    for (size_t i = 0; i < argc; i++) {
        pack_clib_print(argv[i]);
    }
    return pack_value_nil(state);
}

pack_value pack_lib_add(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    bool hasfloat = false;
    for (size_t i = 0; i < argc; i++) {
        if (argv[i].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "+", i);
            exit(1);
        }
        if (argv[i].value.number.type == PACK_NUMBER_TYPE_FLOATING) {
            hasfloat = true;
        }
    }
    pack_number ret;
    if (hasfloat) {
        ret = pack_number_new_double(0);
    }
    else {
        ret = pack_number_new_int64(0);
    }
    for (size_t i = 0; i < argc; i++) {
        ret = pack_number_add(ret, argv[i].value.number);
    }
    return pack_value_num(state, ret);
}

pack_value pack_lib_sub(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, "- takes 1 or more arguments\n");
        exit(1);
    }
    bool hasfloat = false;
    for (size_t i = 0; i < argc; i++) {
        if (argv[i].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "-", i);
            exit(1);
        }
        if (argv[i].value.number.type == PACK_NUMBER_TYPE_FLOATING) {
            hasfloat = true;
        }
    }
    if (argc == 1) {
        return pack_value_num(state, pack_number_sub(pack_number_new_int64(0), argv[0].value.number));
    }
    pack_number ret = argv[0].value.number;
    for (size_t i = 1; i < argc; i++) {
        ret = pack_number_sub(ret, argv[i].value.number);
    }
    return pack_value_num(state, ret);
}

pack_value pack_lib_mul(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    bool hasfloat = false;
    for (size_t i = 0; i < argc; i++) {
        if (argv[i].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "*", i);
            exit(1);
        }
        if (argv[i].value.number.type == PACK_NUMBER_TYPE_FLOATING) {
            hasfloat = true;
        }
    }
    pack_number ret;
    if (hasfloat) {
        ret = pack_number_new_double(1);
    }
    else {
        ret = pack_number_new_int64(1);
    }
    for (size_t i = 0; i < argc; i++) {
        ret = pack_number_mul(ret, argv[i].value.number);
    }
    return pack_value_num(state, ret);
}

pack_value pack_lib_div(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, "/ takes 1 or more arguments\n");
        exit(1);
    }
    bool hasfloat = false;
    for (size_t i = 0; i < argc; i++) {
        if (argv[i].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "-", i);
            exit(1);
        }
        if (argv[i].value.number.type == PACK_NUMBER_TYPE_FLOATING) {
            hasfloat = true;
        }
    }
    if (argc == 1) {
        return pack_value_num(state, pack_number_div(pack_number_new_int64(1), argv[0].value.number));
    }
    pack_number ret = argv[0].value.number;
    for (size_t i = 1; i < argc; i++) {
        ret = pack_number_div(ret, argv[i].value.number);
    }
    return pack_value_num(state, ret);
}

pack_value pack_lib_lt(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, "< takes 1 or more arguments\n");
        exit(1);
    }
    return pack_value_bool(state, pack_clib_lt(argc, argv));
}

pack_value pack_lib_gt(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, "> takes 1 or more arguments\n");
        exit(1);
    }
    return pack_value_bool(state, pack_clib_gt(argc, argv));
}

pack_value pack_lib_lte(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, "<= takes 1 or more arguments\n");
        exit(1);
    }
    return pack_value_bool(state, pack_clib_lte(argc, argv));
}

pack_value pack_lib_gte(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, ">= takes 1 or more arguments\n");
        exit(1);
    }
    return pack_value_bool(state, pack_clib_gte(argc, argv));
}

pack_value pack_lib_eq(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, "= takes 1 or more arguments\n");
        exit(1);
    }
    return pack_value_bool(state, pack_clib_eq(argc, argv));
}

pack_value pack_lib_neq(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, "!= takes 1 or more arguments\n");
        exit(1);
    }
    return pack_value_bool(state, pack_clib_neq(argc, argv));
}
