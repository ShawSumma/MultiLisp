#include "interp.h"
#include <unistd.h>

pack_value pack_value_num(pack_state *state, pack_number n) {
    pack_value v;
    v.type = PACK_VALUE_TYPE_NUMBER;
    v.value.number = n;
    return v;
}

pack_value pack_value_library(pack_state *state, void *l) {
    pack_value v;
    v.type = PACK_VALUE_TYPE_LIBRARY;
    v.value.library = l;
    return v;
}

pack_value pack_value_ffi_type(pack_state *state, ffi_type *t, pack_ffi_type_id id) {
    pack_value v;
    v.type = PACK_VALUE_TYPE_FFI_TYPE;
    v.value.type.type = t;
    v.value.type.type_id = id;
    return v;
}

pack_value pack_value_bool(pack_state *state, bool b) {
    pack_value v;
    v.type = PACK_VALUE_TYPE_BOOLEAN;
    v.value.boolean = b;
    return v;
}

pack_value pack_value_str(pack_state *state, char *s) {
    pack_value v;
    v.type = PACK_VALUE_TYPE_STRING;
    v.value.string = gc_malloc(sizeof(char) * (strlen(s) + 1));
    size_t i = 0;
    while (s[i]) {
        v.value.string[i] = s[i];
        i++;
    }
    v.value.string[i] = '\0';
    return v;
}

pack_value pack_value_cfunc(pack_state *state, pack_value (*cfn)(pack_state *, size_t, pack_local_value *, size_t, pack_value *)) {
    pack_value v;
    v.type = PACK_VALUE_TYPE_FUNCTION;
    v.value.func = gc_malloc(sizeof(pack_func));
    v.value.func->type = FUNC_FROM_C;
    v.value.func->value.cfn.capc = 0;
    v.value.func->value.cfn.cap = NULL;
    v.value.func->value.cfn.cfn = cfn;
    return v;
}

pack_value pack_value_packfunc(pack_state *state, size_t capc, pack_local_value *cap, size_t place) {
    pack_value v;
    v.type = PACK_VALUE_TYPE_FUNCTION;
    v.value.func = gc_malloc(sizeof(pack_func));
    v.value.func->type = FUNC_FROM_PACK;
    v.value.func->value.place.capc = capc;
    v.value.func->value.place.cap = cap;
    v.value.func->value.place.place = place;
    return v;
}

pack_value pack_value_func(pack_state *state, pack_func f) {
    pack_value v;
    v.type = PACK_VALUE_TYPE_FUNCTION;
    v.value.func = gc_malloc(sizeof(pack_func));
    *v.value.func = f;
    return v;
}

pack_value pack_value_nil(pack_state *state) {
    pack_value v;
    v.type = PACK_VALUE_TYPE_NIL;
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
            size_t alloc = 16;
            char *s = gc_malloc(sizeof(char) * alloc);
            size_t count = 0;
            got = getc(f);
            while (got != 'e') {
                if (count + 4 > alloc) {
                    alloc *= 2;
                    s = gc_realloc(s, sizeof(char) * alloc);
                }
                s[count] = got;
                got = getc(f);
                count ++;
            }
            s[count] = '\0';
            vals[valindex] = pack_value_num(state, pack_number_new_bigint(s));
            valindex ++;
            got = getc(f);
        }
        else if (got == 'd') {
            size_t alloc = 16;
            char *s = gc_malloc(sizeof(char) * alloc);
            size_t count = 0;
            got = getc(f);
            while (got != 'e') {
                if (count + 4 > alloc) {
                    alloc *= 2;
                    s = gc_realloc(s, sizeof(char) * alloc);
                }
                s[count] = got;
                got = getc(f);
                count ++;
            }
            s[count] = '\0';
            vals[valindex] = pack_value_num(state, pack_number_new_bigfloat(s));
            valindex ++;
            got = getc(f);
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
                got = getc(f);
                if (got == '\\') {
                    got = getc(f);
                    switch (got) {
                        case 'n': {
                            str[i] = '\n';
                            break;
                        }
                        case 't': {
                            str[i] = '\t';
                            break;
                        }
                        case 'r': {
                            str[i] = '\r';
                            break;
                        }
                        case 's': {
                            str[i] = ' ';
                            break;
                        }
                        case 'e': {
                            str[i] = '\e';
                            break;
                        }
                        default: {
                            str[i] = got;
                            break;
                        }
                    }
                    num --;
                }
                else {
                    str[i] = got;
                }
            } 
            got = getc(f);
            str[num] = '\0';
            if (got == 'l') {
                if (!strcmp("println", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_println);
                }
                else if (!strcmp("print", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_print);
                }
                else if (!strcmp("newline", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_newline);
                }
                else if (!strcmp("vector", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_vector);
                }
                else if (!strcmp("vector-index", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_vector_index);
                }
                else if (!strcmp("vector-length", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_vector_length);
                }
                else if (!strcmp("vector-range", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_vector_range);
                }
                else if (!strcmp("vector-map", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_vector_map);
                }
                else if (!strcmp("vector-fold", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_vector_fold);
                }
                else if (!strcmp("<", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_lt);
                }
                else if (!strcmp("<=", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_lte);
                }
                else if (!strcmp(">", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_gt);
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
                else if (!strcmp("ffi-int8", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_sint8, PACK_FFI_TYPE_INT8);
                }
                else if (!strcmp("ffi-int16", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_sint16, PACK_FFI_TYPE_INT16);
                }
                else if (!strcmp("ffi-int32", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_sint32, PACK_FFI_TYPE_INT32);
                }
                else if (!strcmp("ffi-int64", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_sint64, PACK_FFI_TYPE_INT64);
                }
                else if (!strcmp("ffi-uint8", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_uint8, PACK_FFI_TYPE_UINT8);
                }
                else if (!strcmp("ffi-uint16", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_uint16, PACK_FFI_TYPE_UINT16);
                }
                else if (!strcmp("ffi-uint32", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_uint32, PACK_FFI_TYPE_UINT32);
                }
                else if (!strcmp("ffi-uint64", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_uint64, PACK_FFI_TYPE_UINT64);
                }
                else if (!strcmp("ffi-float", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_float, PACK_FFI_TYPE_FLOAT);
                }
                else if (!strcmp("ffi-double", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_double, PACK_FFI_TYPE_DOUBLE);
                }
                else if (!strcmp("ffi-void", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_void, PACK_FFI_TYPE_VOID);
                }
                else if (!strcmp("ffi-pointer", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_pointer, PACK_FFI_TYPE_POINTER);
                }
                else if (!strcmp("ffi-string", str)) {
                    vals[valindex] = pack_value_ffi_type(state, &ffi_type_pointer, PACK_FFI_TYPE_CHAR_POINTER);
                }
                else if (!strcmp("ffi-library", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_ffi_library);
                }
                else if (!strcmp("ffi-index", str)) {
                    vals[valindex] = pack_value_cfunc(state, pack_lib_ffi_index);
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
        size_t value = 0;
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

pack_value pack_call(pack_state *state, pack_func *f, size_t argc, pack_value *args) {
    if (f->type == FUNC_FROM_PACK) {
        return runpack_program(state, f->value.place.capc, f->value.place.cap, argc, args, f->value.place.place+1);
    }
    else if (f->type == FUNC_FROM_C) {
        return f->value.cfn.cfn(state, f->value.cfn.capc, f->value.cfn.cap, argc, args);
    }
    else {
        void **vpargs = gc_malloc(sizeof(void*) * argc);
        for (size_t i = 0; i < argc; i++) {
            pack_value arg = args[i];
            switch (arg.type) {
                case PACK_VALUE_TYPE_NIL: {
                    vpargs[i] = NULL;
                    break;
                }
                case PACK_VALUE_TYPE_BOOLEAN: {
                    vpargs[i] = &arg.value.boolean;
                    break;
                }
                case PACK_VALUE_TYPE_NUMBER: {
                    pack_number num = arg.value.number;
                    switch (f->value.ffi->arg_types[i].type_id) {
                        case PACK_FFI_TYPE_INT8:
                            vpargs[i] = gc_malloc(sizeof(int8_t));
                            *(int8_t *)vpargs[i] = pack_number_to_int64(num);
                            break;
                        case PACK_FFI_TYPE_INT16: {
                            vpargs[i] = gc_malloc(sizeof(int16_t));
                            *(int16_t *)vpargs[i] = pack_number_to_int64(num);
                            break;
                        }
                        case PACK_FFI_TYPE_INT32: {
                            vpargs[i] = gc_malloc(sizeof(int32_t));
                            *(int32_t *)vpargs[i] = pack_number_to_int64(num);
                            break;
                        }
                        case PACK_FFI_TYPE_INT64: {
                            vpargs[i] = gc_malloc(sizeof(int64_t));
                            *(int64_t *)vpargs[i] = pack_number_to_int64(num);
                            break;
                        }
                        case PACK_FFI_TYPE_UINT8: {
                            vpargs[i] = gc_malloc(sizeof(uint8_t));
                            *(uint8_t *)vpargs[i] = pack_number_to_uint64(num);
                            break;
                        }
                        case PACK_FFI_TYPE_UINT16: {
                            vpargs[i] = gc_malloc(sizeof(uint16_t));
                            *(uint16_t *)vpargs[i] = pack_number_to_uint64(num);
                            break;
                        }
                        case PACK_FFI_TYPE_UINT32: {
                            vpargs[i] = gc_malloc(sizeof(uint32_t));
                            *(uint32_t *)vpargs[i] = pack_number_to_uint64(num);
                            break;
                        }
                        case PACK_FFI_TYPE_UINT64: {
                            vpargs[i] = gc_malloc(sizeof(uint64_t));
                            *(uint64_t *)vpargs[i] = pack_number_to_uint64(num);
                            break;
                        }
                        case PACK_FFI_TYPE_FLOAT: {
                            vpargs[i] = gc_malloc(sizeof(float));
                            *(float *)vpargs[i] = pack_number_to_double(num);
                            break;
                        }
                        case PACK_FFI_TYPE_DOUBLE: {
                            vpargs[i] = gc_malloc(sizeof(double));
                            *(double *)vpargs[i] = pack_number_to_double(num);
                            break;
                        }
                        default: {
                            break;
                        }
                    }
                    break;
                }
                case PACK_VALUE_TYPE_STRING: {
                    vpargs[i] = &arg.value.string;
                    break;
                }
                default: {
                    printf("non ffi type in ffi call");
                    exit(1);
                }
            }
        }
        void *out;
        ffi_call(&f->value.ffi->cif, FFI_FN(f->value.ffi->func), &out, vpargs);
        pack_value back;
        switch (f->value.ffi->ret_type.type_id) {
            case PACK_FFI_TYPE_INT8:
                back.type = PACK_VALUE_TYPE_NUMBER;
                back.value.number = pack_number_new_int64((int8_t) out);
                break;
            case PACK_FFI_TYPE_INT16: {
                back.type = PACK_VALUE_TYPE_NUMBER;
                back.value.number = pack_number_new_int64((int16_t) out);
                break;
            }
            case PACK_FFI_TYPE_INT32: {
                back.type = PACK_VALUE_TYPE_NUMBER;
                back.value.number = pack_number_new_int64((int32_t) out);
                break;
            }
            case PACK_FFI_TYPE_INT64: {
                back.type = PACK_VALUE_TYPE_NUMBER;
                back.value.number = pack_number_new_int64((int64_t) out);
                break;
            }
            case PACK_FFI_TYPE_UINT8: {
                back.type = PACK_VALUE_TYPE_NUMBER;
                back.value.number = pack_number_new_uint64((uint8_t) out);
                break;
            }
            case PACK_FFI_TYPE_UINT16: {
                back.type = PACK_VALUE_TYPE_NUMBER;
                back.value.number = pack_number_new_uint64((uint16_t) out);
                break;
            }
            case PACK_FFI_TYPE_UINT32: {
                back.type = PACK_VALUE_TYPE_NUMBER;
                back.value.number = pack_number_new_uint64((uint32_t) out);
                break;
            }
            case PACK_FFI_TYPE_UINT64: {
                back.type = PACK_VALUE_TYPE_NUMBER;
                back.value.number = pack_number_new_uint64((uint64_t) out);
                break;
            }
            case PACK_FFI_TYPE_FLOAT: {
                back.type = PACK_VALUE_TYPE_NUMBER;
                back.value.number = pack_number_new_double(*(float*)&out);
                break;
            }
            case PACK_FFI_TYPE_DOUBLE: {
                back.type = PACK_VALUE_TYPE_NUMBER;
                back.value.number = pack_number_new_double(*(double*)&out);
                break;
            }
            case PACK_FFI_TYPE_VOID: {
                back.type = PACK_VALUE_TYPE_NIL;
                break;
            }
            case PACK_FFI_TYPE_CHAR_POINTER: {
                back.type = PACK_VALUE_TYPE_STRING;
                back.value.string = out;
                break;
            }
            case PACK_FFI_TYPE_POINTER: {
                back.type = PACK_VALUE_TYPE_POINTER;
                back.value.pointer = out;
                break;
            }
        }
        return back;
    }
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
        // printf("op(%zu)\n", i);
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
                if (v.type == PACK_VALUE_TYPE_BOOLEAN) {
                    if (v.value.boolean == false) {
                        i = op.value;
                    }
                }
                else if (v.type == PACK_VALUE_TYPE_NIL) {
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
                if (vf.type != PACK_VALUE_TYPE_FUNCTION) {
                    printf("cannot call that\n");
                    exit(1);
                }
                pack_func *f = vf.value.func;
                pack_value got = pack_call(state, f, op.value, args);
                state->stack[state->stackindex-1] = got;
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
