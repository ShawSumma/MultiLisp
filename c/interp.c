#include "interp.h"
#include <unistd.h>

pack_value pack_value_num(pack_state *state, double n) {
    pack_value v;
    v.type = VALUE_TYPE_NUMBER;
    v.value.n = n;
    return v;
}

pack_value pack_value_bool(pack_state *state, bool b) {
    pack_value v;
    v.type = VALUE_TYPE_BOOLEAN;
    v.value.b = b;
    return v;
}

pack_value pack_value_str(pack_state *state, char *s) {
    pack_value v;
    v.type = VALUE_TYPE_STRING;
    v.value.s = gc_malloc(sizeof(char) * (strlen(s) + 1));
    size_t i = 0;
    while (s[i]) {
        v.value.s[i] = s[i];
        i++;
    }
    v.value.s[i] = '\0';
    return v;
}

pack_value pack_value_cfunc(pack_state *state, pack_value (*cfn)(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *)) {
    pack_value v;
    v.type = VALUE_TYPE_FUNCTION;
    v.value.f = gc_malloc(sizeof(pack_func));
    v.value.f->type = FUNC_FROM_C;
    v.value.f->capc = 0;
    v.value.f->cap = NULL;
    v.value.f->value.cfn = cfn;
    return v;
}

pack_value pack_value_packfunc(pack_state *state, uint16_t capc, pack_local_value *cap, uint32_t place) {
    pack_value v;
    v.type = VALUE_TYPE_FUNCTION;
    v.value.f = gc_malloc(sizeof(pack_func));
    v.value.f->type = FUNC_FROM_PACK;
    v.value.f->capc = capc;
    v.value.f->cap = cap;
    v.value.f->value.place = place;
    return v;
}

pack_value pack_value_func(pack_state *state, pack_func f) {
    pack_value v;
    v.type = VALUE_TYPE_FUNCTION;
    v.value.f = gc_malloc(sizeof(pack_func));
    *v.value.f = f;
    return v;
}

pack_value pack_value_nil(pack_state *state) {
    pack_value v;
    v.type = VALUE_TYPE_NIL;
    return v;
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
    pack_value *vals = gc_malloc(sizeof(pack_value ) * valcount);
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
            char *str = gc_malloc(sizeof(char) * (num+1));
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
    pack_opcode *opcodes = gc_malloc(sizeof(pack_opcode) * (opcount));
    for (size_t i = 0; i < opcount; i++) {
        uint8_t code = (getc(f)-'0')*10 + getc(f)-'0';
        uint32_t value = 0;
        if (got == EOF) {
            break;
        }
        got = getc(f);
        while (got != 'e') {
            value *= 10;
            value += got-'0';
            got = getc(f);
        }
        opcodes[i] = (pack_opcode){.type=code, .value=value};
    }
    pack_program ret;
    ret.pack_opcodes = opcodes;
    ret.opcount = opcount;
    ret.vals = vals;
    ret.valcount = valcount;
    state->prog = ret;
    runpack_program(state, 0, NULL, 0, NULL, 0);
}

pack_value runpack_program(pack_state *state, size_t capc, pack_local_value *cap, size_t argc, pack_value *argv, size_t i) {
    pack_opcode *pack_opcodes = state->prog.pack_opcodes;
    size_t opcount = state->prog.opcount;
    pack_value *vals = state->prog.vals;
    size_t blocal = state->localindex;
    state->localindex += argc + capc;
    if (state->localindex + 2 > state->localalloc) {
        state->localalloc *= 2;
        state->locals = gc_realloc(state->locals, sizeof(pack_local_value)*state->localalloc);
    }
    for (size_t j = 0; j < argc; j++) {
        state->locals[blocal + j].ismut = false;
        state->locals[blocal + j].value.imut = argv[j];
    }
    for (size_t j = 0; j < capc; j++) {
        state->locals[blocal + j+argc] = cap[j];
    }
    pack_value ret;
    while (i < opcount) {
        pack_opcode op = pack_opcodes[i];
        switch (op.type) {
            case OP_PUSH: {
                if (state->stackindex + 4 > state->stackalloc) {
                    state->stackalloc *= 2;
                    state->stack = gc_realloc(state->stack, sizeof(pack_value) * state->stackalloc);
                }
                state->stack[state->stackindex] = vals[op.value];
                state->stackindex ++;
                break;
            }
            case OP_LOAD: {
                if (state->stackindex + 4 > state->stackalloc) {
                    state->stackalloc *= 2;
                    state->stack = gc_realloc(state->stack, sizeof(pack_value) * state->stackalloc);
                }
                pack_local_value lv = state->locals[blocal + op.value];
                if (lv.ismut) {
                    state->stack[state->stackindex] = *lv.value.mut;
                }
                else {
                    state->stack[state->stackindex] = lv.value.imut;
                }
                state->stackindex ++;
                break;
            }
            case OP_POP: {
                state->stackindex --;
                break;
            }
            case OP_STORE: {
                *state->locals[blocal + op.value].value.mut = state->stack[state->stackindex-1];
                break;
            }
            case OP_NAME: {
                if (state->localindex + 2 > state->localalloc) {
                    state->localalloc *= 2;
                    state->locals = gc_realloc(state->locals, sizeof(pack_local_value)*state->localalloc);
                } 
                // state->locals[state->localindex].value.mut = gc_malloc(sizeof(pack_value));
                state->locals[state->localindex].ismut = false;
                state->locals[state->localindex].value.imut = vals[op.value];
                state->localindex ++;
                break;
            }
            case OP_SPACE: {
                if (state->localindex + 2 > state->localalloc) {
                    state->localalloc *= 2;
                    state->locals = gc_realloc(state->locals, sizeof(pack_local_value) * state->localalloc);
                } 
                state->locals[state->localindex].ismut = true;
                state->locals[state->localindex].value.mut = gc_malloc(sizeof(pack_value));
                *state->locals[state->localindex].value.mut = pack_value_nil(state);
                state->localindex ++;
                break;
            }
            case OP_CAPTURE: {
                if (state->capc + 2 > state->caploc) {
                    state->caploc *= 2;
                    state->capture = gc_realloc(state->capture, sizeof(pack_local_value) * state->caploc);
                }
                state->capture[state->capc] = state->locals[blocal + op.value];
                state->capc ++;
                break;
            }
            case OP_FUNC: {
                if (state->stackindex + 4 > state->stackalloc) {
                    state->stackalloc *= 2;
                    state->stack = gc_realloc(state->stack, sizeof(pack_value) * state->stackalloc);
                }
                state->stack[state->stackindex] = pack_value_packfunc(state, state->capc, state->capture, i);
                state->stackindex ++;
                state->caploc = state->capc + 2;
                state->capc = 0;
                state->capture = gc_malloc(sizeof(pack_local_value) * state->caploc);
                i = op.value;
                break;
            }
            case OP_JUMPFALSE: {
                state->stackindex --;
                pack_value v = state->stack[state->stackindex];
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
            case OP_RET: {
                state->stackindex --;
                ret = state->stack[state->stackindex];
                goto done;
                break;
            }
            case OP_JUMP: {
                i = op.value;
                break;
            }
            case OP_CALL: {
                pack_value *args = state->stack+state->stackindex - op.value;
                state->stackindex -= op.value;
                pack_value vf = state->stack[state->stackindex-1];
                if (vf.type != VALUE_TYPE_FUNCTION) {
                    printf("cannot call that\n");
                    exit(1);
                }
                pack_func f = *vf.value.f;
                if (f.type == FUNC_FROM_PACK) {
                    pack_value v = runpack_program(state, f.capc, f.cap, op.value, args, f.value.place+1);
                    state->stack[state->stackindex-1] = v;
                }
                else {
                    state->stack[state->stackindex-1] = f.value.cfn(state, f.capc, f.cap, op.value, args);
                }
                break;
            }
            default: {
                printf("no op: %d\n", op.type);
                exit(1);
            }
        }
        i ++;
    }
    ret = pack_value_nil(state);
    done:
    state->localindex = blocal;
    return ret;
}
