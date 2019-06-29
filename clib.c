#include "interp.h"

void pack_clib_println(uint16_t argc, pack_value **argv) {
    switch (argv[0]->type) {
        case VALUE_TYPE_NIL: {
            printf("(nil)\n");
            break;
        }
        case VALUE_TYPE_FUNCTION: {
            pack_func f = *argv[0]->value.f;
            if (f.type == FUNC_FROM_C) {
                printf("(function %p)\n", f.value.cfn);
            }
            else {
                printf("(function %d)\n", f.value.place);
            }
            break;
        }
        case VALUE_TYPE_NUMBER: {
            double v = argv[0]->value.n;
            if (fmod(v, 1) == 0) {
                printf("%ld\n", (int64_t) v);
            }
            else {
                printf("%lf\n", v);
            }
            break;
        }
        case VALUE_TYPE_STRING: {
            printf("%s\n", argv[0]->value.s);
            break;
        }
        case VALUE_TYPE_BOOLEAN: {
            printf("%s\n", argv[0]->value.b ? "true" : "false");
            break;
        }
    }
}

bool pack_clib_lt(uint16_t argc, pack_value **argv) {
    double d = argv[0]->value.n;
    for (size_t i = 1; i < argc; i++) {
        if (d < argv[i]->value.n) {
            d = argv[i]->value.n;
        }
        else {
            return false;
        }
    }
    return true;
}

bool pack_clib_gt(uint16_t argc, pack_value **argv) {
    double d = argv[0]->value.n;
    for (size_t i = 1; i < argc; i++) {
        if (d > argv[i]->value.n) {
            d = argv[i]->value.n;
        }
        else {
            return false;
        }
    }
    return true;
}

bool pack_clib_lte(uint16_t argc, pack_value **argv) {
    double d = argv[0]->value.n;
    for (size_t i = 1; i < argc; i++) {
        if (d <= argv[i]->value.n) {
            d = argv[i]->value.n;
        }
        else {
            return false;
        }
    }
    return true;
}

bool pack_clib_gte(uint16_t argc, pack_value **argv) {
    double d = argv[0]->value.n;
    for (size_t i = 1; i < argc; i++) {
        if (d >= argv[i]->value.n) {
            d = argv[i]->value.n;
        }
        else {
            return false;
        }
    }
    return true;
}

bool pack_clib_neq(uint16_t argc, pack_value **argv) {
    for (size_t i = 0; i < argc; i++) {
        pack_value *iv = argv[i];
        for (size_t j = i+1; j < argc; j++) {
            pack_value *jv = argv[j];
            if (iv->type == jv->type) {
                switch(iv->type) {
                    case VALUE_TYPE_NIL: {
                        return false;
                    }
                    case VALUE_TYPE_FUNCTION: {
                        return false;
                    }
                    case VALUE_TYPE_BOOLEAN: {
                        if (iv->value.b == jv->value.b) {
                            return false;
                        }
                        break;
                    }
                    case VALUE_TYPE_NUMBER: {
                        if (iv->value.n == jv->value.n) {
                            return false;
                        }
                        break;
                    }
                    case VALUE_TYPE_STRING: {
                        if (!strcmp(iv->value.s, jv->value.s)) {
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

bool pack_clib_eq(uint16_t argc, pack_value **argv) {
    pack_value *cmp = argv[0];
    for (size_t i = 1; i < argc; i++) {
        pack_value *cur = argv[i];
        if (cmp->type != cur->type) {
            return false;
        }
        switch (cmp->type) {
            case VALUE_TYPE_NIL: {
                break;
            }
            case VALUE_TYPE_FUNCTION: {
                break;
            }
            case VALUE_TYPE_BOOLEAN: {
                if (cur->value.b != cmp->value.b) {
                    return false;
                }
                break;
            }
            case VALUE_TYPE_NUMBER: {
                if (cur->value.n != cmp->value.n) {
                    return false;
                }
                break;
            }
            case VALUE_TYPE_STRING: {
                if (strcmp(cur->value.s, cmp->value.s)) {
                    return false;
                }
                break;
            }
        }
    }
    return true;
}