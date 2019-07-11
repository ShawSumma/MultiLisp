#include "interp.h"

void pack_clib_print(pack_value argv) {
    switch (argv.type) {
        case PACK_VALUE_TYPE_NIL: {
            printf("(nil)");
            break;
        }
        case PACK_VALUE_TYPE_FFI_TYPE: {
            printf("(ffi-type)");
            break;
        }
        case PACK_VALUE_TYPE_LIBRARY: {
            printf("(c-library)");
            break;
        }
        case PACK_VALUE_TYPE_POINTER: {
            printf("(pointer %p)", argv.value.pointer);
            break;
        }
        case PACK_VALUE_TYPE_FUNCTION: {
            pack_func f = *argv.value.func;
            if (f.type == FUNC_FROM_C) {
                printf("(function %p)", f.value.cfn.cfn);
            }
            else if (f.type == FUNC_FROM_FFI) {
                printf("(functio %p)", f.value.ffi->func);
            }
            else {
                printf("(function %lu)", f.value.place.place);
            }
            break;
        }
        case PACK_VALUE_TYPE_VECTOR: {
            printf("(vector %p)", argv.value.vector);
            break;
        }
        case PACK_VALUE_TYPE_NUMBER: {
            switch (argv.value.number.type) {
                case PACK_NUMBER_TYPE_FLOATING: {
                    gmp_printf("%Ff", argv.value.number.value.f);
                    break;
                }
                case PACK_NUMBER_TYPE_INTEGER: {
                    gmp_printf("%Zd", argv.value.number.value.i);
                    break;
                }
            }
            break;
        }
        case PACK_VALUE_TYPE_STRING: {
            printf("%s", argv.value.string);
            break;
        }
        case PACK_VALUE_TYPE_BOOLEAN: {
            printf("%s", argv.value.boolean ? "true" : "false");
            break;
        }
    }
}

bool pack_clib_lt(size_t argc, pack_value *argv) {
    pack_number a = argv[0].value.number;
    for (size_t i = 1; i < argc; i++) {
        pack_number b = argv[i].value.number;
        int cmpv = pack_number_cmp(a, b);
        if (cmpv < 0) {
            a = b;
        }
        else {
            return false;
        }
    }
    return true;
}

bool pack_clib_gt(size_t argc, pack_value *argv) {
    pack_number a = argv[0].value.number;
    for (size_t i = 1; i < argc; i++) {
        pack_number b = argv[i].value.number;
        int cmpv = pack_number_cmp(a, b);
        if (cmpv > 0) {
            a = b;
        }
        else {
            return false;
        }
    }
    return true;
}

bool pack_clib_lte(size_t argc, pack_value *argv) {
    pack_number a = argv[0].value.number;
    for (size_t i = 1; i < argc; i++) {
        pack_number b = argv[i].value.number;
        int cmpv = pack_number_cmp(a, b);
        if (cmpv <= 0) {
            a = b;
        }
        else {
            return false;
        }
    }
    return true;
}

bool pack_clib_gte(size_t argc, pack_value *argv) {
    pack_number a = argv[0].value.number;
    for (size_t i = 1; i < argc; i++) {
        pack_number b = argv[i].value.number;
        int cmpv = pack_number_cmp(a, b);
        if (cmpv >= 0) {
            a = b;
        }
        else {
            return false;
        }
    }
    return true;
}

bool pack_clib_neq(size_t argc, pack_value *argv) {
    for (size_t i = 0; i < argc; i++) {
        pack_value iv = argv[i];
        for (size_t j = i+1; j < argc; j++) {
            pack_value jv = argv[j];
            if (iv.type == jv.type) {
                switch(iv.type) {
                    case PACK_VALUE_TYPE_NIL: {
                        return false;
                    }
                    case PACK_VALUE_TYPE_FUNCTION: {
                        return false;
                    }
                    case PACK_VALUE_TYPE_FFI_TYPE: {
                        return false;
                    }
                    case PACK_VALUE_TYPE_LIBRARY: {
                        return false;
                    }
                    case PACK_VALUE_TYPE_VECTOR: {
                        pack_vector *ivec = iv.value.vector;
                        pack_vector *jvec = jv.value.vector;
                        if (ivec->count == jvec->count) {
                            return false;
                        }
                        for (size_t i = 0; i < ivec->count; i++) {
                            pack_value a[2] = {ivec->values[i], jvec->values[i]};
                            if (!pack_clib_neq(2, a)) {
                                return false;
                            }
                        }
                    }
                    case PACK_VALUE_TYPE_BOOLEAN: {
                        if (iv.value.boolean == jv.value.boolean) {
                            return false;
                        }
                        break;
                    }
                    case PACK_VALUE_TYPE_POINTER: {
                        if (iv.value.pointer == jv.value.pointer) {
                            return false;
                        }
                        break;
                    }
                    case PACK_VALUE_TYPE_NUMBER: {
                        if (pack_number_cmp(iv.value.number, jv.value.number) == 0) {
                            return false;
                        }
                        break;
                    }
                    case PACK_VALUE_TYPE_STRING: {
                        if (!strcmp(iv.value.string, jv.value.string)) {
                            return false;
                        }
                        break;
                    }
                }
            }
        }
    }
    return true;
}

bool pack_clib_eq(size_t argc, pack_value *argv) {
    pack_value cmp = argv[0];
    for (size_t i = 1; i < argc; i++) {
        pack_value cur = argv[i];
        if (cmp.type != cur.type) {
            return false;
        }
        switch (cmp.type) {
            case PACK_VALUE_TYPE_NIL: {
                return true;
            }
            case PACK_VALUE_TYPE_FUNCTION: {
                return true;
            }
            case PACK_VALUE_TYPE_FFI_TYPE: {
                return true;
            }
            case PACK_VALUE_TYPE_LIBRARY: {
                return true;
            }
            case PACK_VALUE_TYPE_VECTOR: {
                pack_vector *cmpvec = cmp.value.vector;
                pack_vector *curvec = cur.value.vector;
                if (cmpvec->count != curvec->count) {
                    return false;
                }
                for (size_t i = 0; i < cmpvec->count; i++) {
                    pack_value a[2] = {cmpvec->values[i], curvec->values[i]};
                    if (!pack_clib_eq(2, a)) {
                        return false;
                    }
                }
                break;
            }
            case PACK_VALUE_TYPE_BOOLEAN: {
                if (cur.value.boolean != cmp.value.boolean) {
                    return false;
                }
                break;
            }
            case PACK_VALUE_TYPE_POINTER: {
                if (cur.value.pointer != cmp.value.pointer) {
                    return false;
                }
                break;
            }
            case PACK_VALUE_TYPE_NUMBER: {
                if (pack_number_cmp(cur.value.number, cmp.value.number) != 0) {
                    return false;
                }
                break;
            }
            case PACK_VALUE_TYPE_STRING: {
                if (strcmp(cur.value.string, cmp.value.string)) {
                    return false;
                }
                break;
            }
        }
    }
    return true;
}