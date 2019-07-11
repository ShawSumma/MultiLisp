#include "interp.h"
#include "lib.h"


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

pack_value pack_lib_index(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
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
    size_t ind = pack_vector_index_convert(state, argv[1].value.vector, argv[1].value.number);
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
    for (size_t i = 0; i < argc; i++) {
        if (argv[i].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "+", i);
            exit(1);
        }
    }
    double d = 0;
    for (size_t i = 0; i < argc; i++) {
        d += argv[i].value.number;
    }
    return pack_value_num(state, d);
}

pack_value pack_lib_sub(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, "- takes 1 or more arguments\n");
        exit(1);
    }
    for (size_t i = 0; i < argc; i++) {
        if (argv[i].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "-", i);
            exit(1);
        }
    }
    if (argc == 1) {
        return pack_value_num(state, -argv[0].value.number);
    }
    double d = argv[0].value.number;
    for (size_t i = 1; i < argc; i++) {
        d -= argv[i].value.number;
    }
    return pack_value_num(state, d);
}

pack_value pack_lib_mul(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    for (size_t i = 0; i < argc; i++) {
        if (argv[i].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "*", i);
            exit(1);
        }
    }
    double d = 1;
    for (size_t i = 0; i < argc; i++) {
        d *= argv[i].value.number;
    }
    return pack_value_num(state, d);
}

pack_value pack_lib_div(pack_state *state,  size_t capc, pack_local_value *cap, size_t argc, pack_value *argv) {
    if (argc == 0) {
        fprintf(stderr, "/ takes 1 or more arguments\n");
        exit(1);
    }
    for (size_t i = 0; i < argc; i++) {
        if (argv[i].type != PACK_VALUE_TYPE_NUMBER) {
            pack_error_argindex(state, "/", i);
            exit(1);
        }
    }
    if (argc == 1) {
        return pack_value_num(state, 1/argv[0].value.number);
    }
    double d = argv[0].value.number;
    for (size_t i = 1; i < argc; i++) {
        d /= argv[i].value.number;
    }
    return pack_value_num(state, d);
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
