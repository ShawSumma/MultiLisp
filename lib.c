#include "interp.h"

value lib_println(program prog, uint16_t argc, value *argv) {
    switch (argv[0].type) {
        case VALUE_TYPE_NIL: {
            printf("(nil)\n");
            break;
        }
        case VALUE_TYPE_FUNCTION: {
            printf("function\n");
            break;
        }
        case VALUE_TYPE_NUMBER: {
            double v = argv[0].value.n;
            if (fmod(v, 1) == 0) {
                printf("%ld\n", (int64_t) v);
            }
            else {
                printf("%lf\n", v);
            }
            break;
        }
        case VALUE_TYPE_STRING: {
            printf("%s\n", argv[0].value.s);
            break;
        }
        case VALUE_TYPE_BOOLEAN: {
            printf("%s\n", argv[0].value.b ? "true" : "false");
            break;
        }
    }
    return value_nil();
}

value lib_lt(program prog, uint16_t argc, value *argv) {
    double d = argv[0].value.n;
    for (size_t i = 1; i < argc; i++) {
        if (d < argv[i].value.n) {
            d = argv[i].value.n;
        }
        else {
            return value_bool(false);
        }
    }
    return value_bool(true);
}

value lib_gt(program prog, uint16_t argc, value *argv) {
    double d = argv[0].value.n;
    for (size_t i = 1; i < argc; i++) {
        if (d > argv[i].value.n) {
            d = argv[i].value.n;
        }
        else {
            return value_bool(false);
        }
    }
    return value_bool(true);
}

value lib_lte(program prog, uint16_t argc, value *argv) {
    double d = argv[0].value.n;
    for (size_t i = 1; i < argc; i++) {
        if (d <= argv[i].value.n) {
            d = argv[i].value.n;
        }
        else {
            return value_bool(false);
        }
    }
    return value_bool(true);
}

value lib_gte(program prog, uint16_t argc, value *argv) {
    double d = argv[0].value.n;
    for (size_t i = 1; i < argc; i++) {
        if (d >= argv[i].value.n) {
            d = argv[i].value.n;
        }
        else {
            return value_bool(false);
        }
    }
    return value_bool(true);
}

value lib_neq(program prog, uint16_t argc, value *argv) {
    for (size_t i = 0; i < argc; i++) {
        value iv = argv[i];
        for (size_t j = i+1; j < argc; j++) {
            value jv = argv[i];
            if (iv.type == jv.type) {
                switch(iv.type) {
                    case VALUE_TYPE_NIL: {
                        return value_bool(false);
                    }
                    case VALUE_TYPE_FUNCTION: {
                        return value_bool(false);
                    }
                    case VALUE_TYPE_BOOLEAN: {
                        if (iv.value.b == jv.value.b) {
                            return value_bool(false);
                        }
                        break;
                    }
                    case VALUE_TYPE_NUMBER: {
                        if (iv.value.n == jv.value.n) {
                            return value_bool(false);
                        }
                        break;
                    }
                    case VALUE_TYPE_STRING: {
                        if (!strcmp(iv.value.s, jv.value.s)) {
                            return value_bool(false);
                        }
                        break;
                    }
                }
            }
        }
    }
    return value_bool(true);
}

value lib_eq(program prog, uint16_t argc, value *argv) {
    value cmp = argv[0];
    for (size_t i = 1; i < argc; i++) {
        value cur = argv[i];
        if (cmp.type != cur.type) {
            return value_bool(false);
        }
        switch (cmp.type) {
            case VALUE_TYPE_NIL: {
                break;
            }
            case VALUE_TYPE_FUNCTION: {
                break;
            }
            case VALUE_TYPE_BOOLEAN: {
                if (cur.value.b != cmp.value.b) {
                    return value_bool(false);
                }
                break;
            }
            case VALUE_TYPE_NUMBER: {
                if (cur.value.n != cmp.value.n) {
                    return value_bool(false);
                }
                break;
            }
            case VALUE_TYPE_STRING: {
                if (strcmp(cur.value.s, cmp.value.s)) {
                    return value_bool(false);
                }
                break;
            }
        }
    }
    return value_bool(true);
}

value lib_add(program prog, uint16_t argc, value *argv) {
    double d = 0;
    for (size_t i = 0; i < argc; i++) {
        d += argv[i].value.n;
    }
    return value_num(d);
}

value lib_sub(program prog, uint16_t argc, value *argv) {
    double d = argv[0].value.n;
    for (size_t i = 1; i < argc; i++) {
        d -= argv[i].value.n;
    }
    return value_num(d);
}

value lib_mul(program prog, uint16_t argc, value *argv) {
    double d = 1;
    for (size_t i = 0; i < argc; i++) {
        d *= argv[i].value.n;
    }
    return value_num(d);
}

value lib_div(program prog, uint16_t argc, value *argv) {
    double d = argv[0].value.n;
    for (size_t i = 1; i < argc; i++) {
        d /= argv[i].value.n;
    }
    return value_num(d);
}