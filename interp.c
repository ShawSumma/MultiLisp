#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <gc.h>

// #define malloc GC_MALLOC
// #define realloc GC_REALLOC

struct func;
typedef struct func func;
struct value;
typedef struct value value;
struct opcode;
typedef struct opcode opcode;

typedef enum {
    OP_INIT = 0,
    OP_PUSH,
    OP_POP,
    OP_CALL,
    OP_STORE,
    OP_LOAD,
    OP_FUNC,
    OP_CAPTURE,
    OP_RET,
    OP_NAME,
    OP_SPACE,
    OP_JUMP,
    OP_JUMPFALSE,
} optype;

struct func {
    union {
        struct {
            value **cap;
            uint32_t place;
            uint16_t capc;
        } place;
        value(*cfn)(uint16_t, value*);
    } value;
    enum {
        FUNC_FROM_C,
        FUNC_FROM_PACK,
    } type;
};

struct value {
    enum {
        VALUE_TYPE_NIL,
        VALUE_TYPE_NUMBER,
        VALUE_TYPE_STRING,
        VALUE_TYPE_FUNCTION,
        VALUE_TYPE_BOOLEAN,
    } type;
    union {
        double n;
        char *s;
        func f;
        bool b;
    } value;
};

struct opcode {
    optype type;
    uint32_t value;
};

value value_num(double n) {
    value v;
    v.type = VALUE_TYPE_NUMBER;
    v.value.n = n;
    return v;
}

value value_bool(bool b) {
    value v;
    v.type = VALUE_TYPE_BOOLEAN;
    v.value.b = b;
    return v;
}

value value_str(char *s) {
    value v;
    v.type = VALUE_TYPE_STRING;
    v.value.s = GC_MALLOC(sizeof(char) * strlen(s));
    size_t i = 0;
    while (s[i]) {
        v.value.s[i] = s[i];
        i++;
    }
    return v;
}

value value_cfunc(value(*cfn)(uint16_t, value*)) {
    value v;
    v.type = VALUE_TYPE_FUNCTION;
    v.value.f.type = FUNC_FROM_C;
    v.value.f.value.cfn = cfn;
    return v;
}

value value_packfunc(uint16_t capc, value **cap, uint32_t place) {
    value v;
    v.type = VALUE_TYPE_FUNCTION;
    v.value.f.type = FUNC_FROM_PACK;
    v.value.f.value.place.capc = capc;
    v.value.f.value.place.cap = cap;
    v.value.f.value.place.place = place;
    return v;
}

value value_func(func f) {
    value v;
    v.type = VALUE_TYPE_FUNCTION;
    v.value.f = f;
    return v;
}

value value_nil() {
    return (value){.type=VALUE_TYPE_NIL};
}

value lib_println(uint16_t argc, value *argv) {
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

value lib_lt(uint16_t argc, value *argv) {
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

value lib_gt(uint16_t argc, value *argv) {
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

value lib_lte(uint16_t argc, value *argv) {
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

value lib_gte(uint16_t argc, value *argv) {
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

value lib_neq(uint16_t argc, value *argv) {
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

value lib_eq(uint16_t argc, value *argv) {
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

value lib_add(uint16_t argc, value *argv) {
    double d = 0;
    for (size_t i = 0; i < argc; i++) {
        d += argv[i].value.n;
    }
    return value_num(d);
}

value lib_sub(uint16_t argc, value *argv) {
    double d = argv[0].value.n;
    for (size_t i = 1; i < argc; i++) {
        d -= argv[i].value.n;
    }
    return value_num(d);
}

value lib_mul(uint16_t argc, value *argv) {
    double d = 1;
    for (size_t i = 0; i < argc; i++) {
        d *= argv[i].value.n;
    }
    return value_num(d);
}

value lib_div(uint16_t argc, value *argv) {
    double d = argv[0].value.n;
    for (size_t i = 1; i < argc; i++) {
        d /= argv[i].value.n;
    }
    return value_num(d);
}

void runfile(FILE *f) {
    size_t valcount = 0;
    char got = getc(f);
    while (got != 'e') {
        valcount *= 10;
        valcount += got-'0';
        got = getc(f);
    }
    size_t valindex = 0;
    value *vals = malloc(sizeof(value) * valcount);
    got = getc(f);
    while (got != ':') {
        if (got == 'i') {
            int64_t num = 0;
            got = getc(f);
            while (got != 'e') {
                num *= 10;
                num += got-'0';
                got = getc(f);
            }
            got = getc(f);
            vals[valindex] = value_num(num);
            valindex ++;
        }
        else if (got == 'd') {
            double num = 0;
            got = getc(f);
            while (got != 'e') {
                num *= 10;
                num += got-'0';
                got = getc(f);
            }
            got = getc(f);
            vals[valindex] = value_num(num);
            valindex ++;
        }
        else if (got == 's') {
            size_t num = 0;
            got = getc(f);
            while (got != 'e') {
                num *= 10;
                num += got-'0';
                got = getc(f);
            }
            char *str = malloc(sizeof(char) * (num+1));
            for (size_t i = 0; i < num; i++) {
                str[i] = getc(f);
            } 
            got = getc(f);
            str[num] = '\0';
            if (got == 'l') {
                if (!strcmp("println", str)) {
                    vals[valindex] = value_cfunc(lib_println);
                    valindex ++;
                }
                else if (!strcmp("<", str)) {
                    vals[valindex] = value_cfunc(lib_lt);
                    valindex ++;
                }
                else if (!strcmp("<=", str)) {
                    vals[valindex] = value_cfunc(lib_lte);
                    valindex ++;
                }
                else if (!strcmp(">", str)) {
                    vals[valindex] = value_cfunc(lib_lte);
                    valindex ++;
                }
                else if (!strcmp(">=", str)) {
                    vals[valindex] = value_cfunc(lib_gte);
                    valindex ++;
                }
                else if (!strcmp("!=", str)) {
                    vals[valindex] = value_cfunc(lib_neq);
                    valindex ++;
                }
                else if (!strcmp("true", str)) {
                    vals[valindex] = value_bool(true);
                    valindex ++;
                }
                else if (!strcmp("false", str)) {
                    vals[valindex] = value_bool(false);
                    valindex ++;
                }
                else if (!strcmp("=", str)) {
                    vals[valindex] = value_cfunc(lib_eq);
                    valindex ++;
                }
                else if (!strcmp("-", str)) {
                    vals[valindex] = value_cfunc(lib_sub);
                    valindex ++;
                }
                else if (!strcmp("*", str)) {
                    vals[valindex] = value_cfunc(lib_mul);
                    valindex ++;
                }
                else if (!strcmp("/", str)) {
                    vals[valindex] = value_cfunc(lib_div);
                    valindex ++;
                }
                else if (!strcmp("+", str)) {
                    vals[valindex] = value_cfunc(lib_add);
                    valindex ++;
                }
                else {
                    printf("could not load name %s\n", str);
                    exit(1);
                }
                got = getc(f);
            }
            else {
                vals[valindex] = value_str(str);
                valindex ++;
            }
            free(str);
        }
    }
    size_t opcount = 0;
    got = getc(f);
    while (got != 'e') {
        opcount *= 10;
        opcount += got-'0';
        got = getc(f);
    }
    opcount /= 2;
    got = getc(f);
    opcode *opcodes = malloc(sizeof(opcode) * (opcount));
    for (size_t i = 0; i < opcount; i++) {
        uint8_t code = (got-'0')*10 + getc(f)-'0';
        uint32_t value = 0;
        if (got == EOF) {
            break;
        }
        if (code > 10) {
            got = getc(f);
        }
        while (got != 'e') {
            value *= 10;
            value += got-'0';
            got = getc(f);
        }
        opcodes[i] = (opcode){.type=code, .value=value};
        got = getc(f);
    }
    size_t stacksize = 0;
    size_t stackalloc = 16;
    value *stack = malloc(sizeof(value) * stackalloc);

    size_t callalloc = 8;
    size_t callcount = 0;
    size_t *localbackalloc = malloc(sizeof(size_t) * callalloc);
    size_t *localbackcount = malloc(sizeof(size_t) * callalloc);
    localbackalloc[0] = 8;
    localbackcount[0] = 0;
    value **locals = malloc(sizeof(value *) * callalloc);
    locals[callcount] = malloc(sizeof(value) * localbackalloc[callcount]);
    size_t *rets = malloc(sizeof(size_t) * callalloc);
    size_t caploc = 8;
    size_t capc = 0;
    value **capture = malloc(sizeof(value*) * caploc);
    for (size_t i = 0; i < opcount; i++) {
        opcode op = opcodes[i];
        switch (op.type) {
            case OP_INIT: {
                break;
            }
            case OP_NAME: {
                if (localbackcount[callcount] + 2 > localbackalloc[callcount]) {
                    localbackalloc[callcount] *= 2;
                    locals[callcount] = realloc(locals[callcount], sizeof(value) * localbackalloc[callcount]);
                } 
                locals[callcount][localbackcount[callcount]] = vals[op.value];
                localbackcount[callcount] ++;
                break;
            }
            case OP_SPACE: {
                if (localbackcount[callcount] + 2 > localbackalloc[callcount]) {
                    localbackalloc[callcount] *= 2;
                    locals[callcount] = realloc(locals[callcount], sizeof(value) * localbackalloc[callcount]);
                } 
                locals[callcount][localbackcount[callcount]] = value_nil();
                localbackcount[callcount] ++;
                break;
            }
            case OP_CAPTURE: {
                if (capc + 2 > caploc) {
                    caploc *= 2;
                    capture = realloc(capture, sizeof(value *) * caploc);
                }
                capture[capc] = &locals[callcount][op.value]; 
                capc ++;
                break;
            }
            case OP_FUNC: {
                if (stacksize + 4 > stackalloc) {
                    stackalloc *= 2;
                    stack = realloc(stack, sizeof(value) * stackalloc);
                }
                stack[stacksize] = value_packfunc(capc, capture, i);
                stacksize ++;
                i = op.value;
                capc = 0;
                break;
            }
            case OP_JUMPFALSE: {
                stacksize --;
                value v = stack[stacksize];
                if (v.type == VALUE_TYPE_BOOLEAN) {
                    if (v.value.b == false) {
                        i = op.value;
                    }
                }
                else if (v.type == VALUE_TYPE_NIL) {
                    i = op.value;
                }
                break;
            }
            case OP_STORE: {
                locals[callcount][op.value] = stack[stacksize-1];
                break;
            }
            case OP_RET: {
                free(locals[callcount]);
                callcount --;
                i = rets[callcount];
                break;
            }
            case OP_LOAD: {
                if (stacksize + 4 > stackalloc) {
                    stackalloc *= 2;
                    stack = realloc(stack, sizeof(value) * stackalloc);
                }
                stack[stacksize] = locals[callcount][op.value];
                stacksize ++;
                break;
            }
            case OP_JUMP: {
                i = op.value;
                break;
            }
            case OP_PUSH: {
                if (stacksize + 4 > stackalloc) {
                    stackalloc *= 2;
                    stack = realloc(stack, sizeof(value) * stackalloc);
                }
                stack[stacksize] = vals[op.value];
                stacksize ++;
                break;
            }
            case OP_POP: {
                stacksize --;
                break;
            }
            case OP_CALL: {
                value *args = stack+stacksize-op.value;
                stacksize -= op.value;
                value f = stack[stacksize-1];
                if (f.type != VALUE_TYPE_FUNCTION) {
                    printf("cannot call that\n");
                    exit(1);
                }
                if (f.value.f.type == FUNC_FROM_PACK) {
                    stacksize --;
                    if (callcount + 4 > callalloc) {
                        callalloc *= 2;
                        locals = realloc(locals, sizeof(value *) * callalloc);
                        localbackalloc = realloc(localbackalloc, sizeof(size_t) * callalloc);
                        localbackcount = realloc(localbackcount, sizeof(size_t) * callalloc);
                        rets = realloc(rets, sizeof(size_t) * callalloc);
                    }
                    rets[callcount] = i;
                    callcount ++;
                    localbackalloc[callcount] = f.value.f.value.place.capc + op.value + 2;
                    localbackcount[callcount] = f.value.f.value.place.capc + op.value;
                    locals[callcount] = malloc(sizeof(value) * localbackalloc[callcount]);
                    i = f.value.f.value.place.place;
                    for (size_t j = 0; j < f.value.f.value.place.capc; j++) {
                        locals[callcount][j] = args[j];
                    }
                    for (size_t j = 0; j < f.value.f.value.place.capc; j++) {
                        locals[callcount][j+op.value] = *f.value.f.value.place.cap[j];
                    }
                }
                else {
                    stack[stacksize-1] = f.value.f.value.cfn(op.value, args);
                }
                break;
            }
            default: {
                printf("no op: %d\n", op.type);
                exit(1);
            }
        }
    }
    free(localbackalloc);
    free(localbackcount);
    free(locals);
    free(vals);
    free(stack);
    free(rets);
}

int main(int argc, char **argv) {
    GC_INIT();
    if (argc != 2) {
        printf("one argument must be provided\n");
        exit(1);
    }
    char *name = argv[1];
    FILE *f = fopen(name, "r");
    if (f == NULL) {
        printf("no such file %s\n", name);
        exit(1);
    }
    runfile(f);
    fclose(f);
}