#include "interp.h"
#include <unistd.h>

pack_value *pack_value_num(pack_state *state, double n) {
    pack_value *v = malloc(sizeof(pack_value));
    v->type = VALUE_TYPE_NUMBER;
    v->value.n = n;
    return v;
}

pack_value *pack_value_bool(pack_state *state, bool b) {
    pack_value *v = malloc(sizeof(pack_value));
    v->type = VALUE_TYPE_BOOLEAN;
    v->value.b = b;
    return v;
}

pack_value *pack_value_str(pack_state *state, char *s) {
    pack_value *v = malloc(sizeof(pack_value));
    v->type = VALUE_TYPE_STRING;
    v->value.s = malloc(sizeof(char) * (strlen(s) + 1));
    size_t i = 0;
    while (s[i]) {
        v->value.s[i] = s[i];
        i++;
    }
    v->value.s[i] = '\0';
    return v;
}

pack_value *pack_value_cfunc(pack_state *state, pack_value *(*cfn)(pack_state *, uint16_t, pack_value **, uint16_t, pack_value **)) {
    pack_value *v = malloc(sizeof(pack_value));
    v->type = VALUE_TYPE_FUNCTION;
    v->value.f = malloc(sizeof(pack_func));
    v->value.f->type = FUNC_FROM_C;
    v->value.f->capc = 0;
    v->value.f->cap = NULL;
    v->value.f->value.cfn = cfn;
    return v;
}

pack_value *pack_value_packfunc(pack_state *state, uint16_t capc, pack_value **cap, uint32_t place) {
    pack_value *v = malloc(sizeof(pack_value));
    v->type = VALUE_TYPE_FUNCTION;
    v->value.f = malloc(sizeof(pack_func));
    v->value.f->type = FUNC_FROM_PACK;
    v->value.f->capc = capc;
    v->value.f->cap = cap;
    v->value.f->value.place = place;
    return v;
}

pack_value *pack_value_func(pack_state *state, pack_func f) {
    pack_value *v = malloc(sizeof(pack_value));
    v->type = VALUE_TYPE_FUNCTION;
    v->value.f = malloc(sizeof(pack_func));
    *v->value.f = f;
    return v;
}

pack_value *pack_value_nil(pack_state *state) {
    pack_value *ret = malloc(sizeof(pack_value));
    ret->type = VALUE_TYPE_NIL;
    return ret;
}

void runfile(pack_state *state, FILE *f) {
    size_t valcount = 0;
    char got = getc(f);
    while (got != 'e') {
        valcount *= 10;
        valcount += got-'0';
        got = getc(f);
    }
    size_t valindex = 0;
    pack_value **vals = malloc(sizeof(pack_value *) * valcount);
    got = getc(f);
    size_t newvalcount = 0;
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
            vals[valindex] = pack_value_num(state, num);
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
            vals[valindex] = pack_value_num(state, num);
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
                    vals[valindex] = pack_value_cfunc(state, pack_lib_println);
                }
                else if (!strcmp("<", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_lt);
                }
                else if (!strcmp("<=", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_lte);
                }
                else if (!strcmp(">", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_lte);
                }
                else if (!strcmp(">=", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_gte);
                }
                else if (!strcmp("!=", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_neq);
                }
                else if (!strcmp("true", str)) {
                    vals[valindex] = pack_value_bool(state, true);
                }
                else if (!strcmp("false", str)) {
                    vals[valindex] = pack_value_bool(state, false);
                }
                else if (!strcmp("=", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_eq);
                }
                else if (!strcmp("-", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_sub);
                }
                else if (!strcmp("*", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_mul);
                }
                else if (!strcmp("/", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_div);
                }
                else if (!strcmp("+", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_add);
                }
                else {
                    printf("could not load name %s\n", str);
                    exit(1);
                }
                valindex ++;
                got = getc(f);
            }
            else {
                vals[valindex] = pack_value_str(state, str);
                valindex ++;
            }
        }
        newvalcount++;
    }
    valcount = newvalcount;
    size_t opcount = 0;
    got = getc(f);
    while (got != 'e') {
        opcount *= 10;
        opcount += got-'0';
        got = getc(f);
    }
    opcount /= 2;
    got = getc(f);
    pack_opcode *opcodes = malloc(sizeof(pack_opcode) * (opcount));
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
        opcodes[i] = (pack_opcode){.type=code, .value=value};
        got = getc(f);
    }
    // for (uint32_t i = 0; i < opcount; i++) {
    //     printf("%u\t%u\n",i, opcodes[i].type);
    // }
    pack_program ret;
    ret.pack_opcodes = opcodes;
    ret.opcount = opcount;
    ret.vals = vals;
    ret.valcount = valcount;
    state->prog = ret;
    runpack_program(state, 0);
}

void runpack_program(pack_state *state, size_t i) {
    pack_opcode *pack_opcodes = state->prog.pack_opcodes;
    size_t opcount = state->prog.opcount;
    pack_value **vals = state->prog.vals;
    size_t valcount = state->prog.valcount;

    size_t stacksize = 0;
    size_t stackalloc = 16;
    pack_value **stack = malloc(sizeof(pack_value *) * stackalloc);
    size_t callalloc = 8;
    size_t callcount = 0;
    size_t *localbackalloc = malloc(sizeof(size_t) * callalloc);
    size_t *localbackcount = malloc(sizeof(size_t) * callalloc);
    localbackalloc[0] = 8;
    localbackcount[0] = 0;
    pack_value ***locals = malloc(sizeof(pack_value **) * callalloc);
    locals[0] = malloc(sizeof(pack_value *) * localbackalloc[0]);
    size_t *rets = malloc(sizeof(size_t) * callalloc);
    size_t caploc = 8;
    size_t capc = 0;
    pack_value **capture = malloc(sizeof(pack_value *) * caploc);
    size_t countops = 0;
    while (i < opcount) {
        // for (uint32_t i = 0; i < localbackcount[callcount]; i++) {
        //     printf("    locals[%d] = ", i);
        //     pack_clib_println(1, &locals[callcount][i]);
        // }
        // printf("\n");
        // for (uint32_t i = 0; i < stacksize; i++) {
        //     printf("    stack[%d] = ", i);
        //     pack_clib_println(1, &stack[i]);
        // }
        // printf("\n");
        // printf("%zu: %lu\n", i, callcount);
        pack_opcode op = pack_opcodes[i];
        switch (op.type) {
            case OP_INIT: {
                break;
            }
            case OP_NAME: {
                if (localbackcount[callcount] + 2 > localbackalloc[callcount]) {
                    localbackalloc[callcount] *= 2;
                    locals[callcount] = realloc(locals[callcount], sizeof(pack_value *) * localbackalloc[callcount]);
                } 
                locals[callcount][localbackcount[callcount]] = vals[op.value];
                localbackcount[callcount] ++;
                break;
            }
            case OP_SPACE: {
                if (localbackcount[callcount] + 2 > localbackalloc[callcount]) {
                    localbackalloc[callcount] *= 2;
                    locals[callcount] = realloc(locals[callcount], sizeof(pack_value *) * localbackalloc[callcount]);
                } 
                locals[callcount][localbackcount[callcount]] = pack_value_nil(state);
                localbackcount[callcount] ++;
                break;
            }
            case OP_CAPTURE: {
                if (capc + 2 > caploc) {
                    caploc *= 2;
                    capture = realloc(capture, sizeof(pack_value **) * caploc);
                }
                capture[capc] = locals[callcount][op.value]; 
                capc ++;
                break;
            }
            case OP_FUNC: {
                if (stacksize + 4 > stackalloc) {
                    stackalloc *= 2;
                    stack = realloc(stack, sizeof(pack_value *) * stackalloc);
                }
                stack[stacksize] = pack_value_packfunc(state, capc, capture, i);
                stacksize ++;
                caploc = capc + 2;
                capc = 0;
                capture = malloc(sizeof(pack_value *) * caploc);
                i = op.value;
                break;
            }
            case OP_JUMPFALSE: {
                stacksize --;
                pack_value *v = stack[stacksize];
                if (v->type == VALUE_TYPE_BOOLEAN) {
                    if (v->value.b == false) {
                        i = op.value;
                    }
                }
                else if (v->type == VALUE_TYPE_NIL) {
                    i = op.value;
                }
                break;
            }
            case OP_STORE: {
                *locals[callcount][op.value] = *stack[stacksize-1];
                break;
            }
            case OP_RET: {
                if (callcount == 0) {
                    goto done;
                }
                callcount --;
                i = rets[callcount];
                break;
            }
            case OP_LOAD: {
                if (stacksize + 4 > stackalloc) {
                    stackalloc *= 2;
                    stack = realloc(stack, sizeof(pack_value *) * stackalloc);
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
                    stack = realloc(stack, sizeof(pack_value *) * stackalloc);
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
                pack_value **args = stack+stacksize-op.value;
                stacksize -= op.value;
                pack_value *vf = stack[stacksize-1];
                if (vf->type != VALUE_TYPE_FUNCTION) {
                    printf("%p = ", vf);
                    pack_clib_println(1, &vf);
                    // printf("\n");
                    printf("cannot call that\n");
                    exit(1);
                }
                pack_func f = *vf->value.f;
                if (f.type == FUNC_FROM_PACK) {
                    stacksize --;
                    if (callcount + 4 > callalloc) {
                        callalloc *= 2;
                        locals = realloc(locals, sizeof(pack_value **) * callalloc);
                        localbackalloc = realloc(localbackalloc, sizeof(size_t) * callalloc);
                        localbackcount = realloc(localbackcount, sizeof(size_t) * callalloc);
                        rets = realloc(rets, sizeof(size_t) * callalloc);
                    }
                    rets[callcount] = i;
                    callcount ++;
                    localbackalloc[callcount] = f.capc + op.value + 2;
                    localbackcount[callcount] = f.capc + op.value;
                    locals[callcount] = malloc(sizeof(pack_value *) * localbackalloc[callcount]);
                    i = f.value.place;
                    for (size_t j = 0; j < op.value; j++) {
                        locals[callcount][j] = args[j];
                    }
                    for (size_t j = 0; j < f.capc; j++) {
                        locals[callcount][j+op.value] = f.cap[j];
                    }
                }
                else {
                    stack[stacksize-1] = f.value.cfn(state, f.capc, f.cap, op.value, args);
                    for (size_t i = 0; i < op.value; i++) {
                    }
                }
                break;
            }
            default: {
                printf("no op: %d\n", op.type);
                exit(1);
            }
        }
        i ++;
        countops ++;
    }
    done: return;
}

int main(int argc, char **argv) {
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
    pack_state *state = pack_state_new();
    runfile(state, f);
    fclose(f);
}