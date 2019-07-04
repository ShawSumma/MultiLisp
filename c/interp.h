#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <gc.h>

#define gc_malloc GC_malloc
#define gc_realloc GC_realloc
#define gc_free GC_free

struct pack_func;
typedef struct pack_func pack_func;
struct pack_value;
typedef struct pack_value pack_value;
struct pack_opcode;
typedef struct pack_opcode pack_opcode;
struct pack_program;
typedef struct pack_program pack_program;
struct pack_pair;
typedef struct pack_pair pack_pair;
struct pack_state;
typedef struct pack_state pack_state;
struct pack_local_value;
typedef struct pack_local_value pack_local_value;

typedef enum {
    // OP_INIT = 0,
    OP_PUSH = 0,
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

typedef enum {
    PACK_UNMARKED = 0,
    PACK_MARKED = 1,
} pack_mark;

struct pack_func {
    pack_local_value *cap;
    union {
        uint32_t place;
        pack_value (*cfn)(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *);
    } value;
    enum {
        FUNC_FROM_C,
        FUNC_FROM_PACK,
    } type;
    uint16_t capc;
};

struct pack_value {
    union {
        double n;
        char *s;
        pack_func *f;
        bool b;
    } value;
    enum {
        VALUE_TYPE_NIL,
        VALUE_TYPE_NUMBER,
        VALUE_TYPE_BOOLEAN,
        VALUE_TYPE_FUNCTION,
        VALUE_TYPE_STRING,
    } type;
};

struct pack_pair {
    pack_value *left;
    pack_value *right;
};

struct pack_opcode {
    optype type;
    uint32_t value;
};

struct pack_program {
    size_t opcount;
    pack_opcode *pack_opcodes;
    size_t valcount;
    pack_value *vals;
};

struct pack_state {
    pack_program prog;
    pack_local_value *locals;
    size_t localalloc;
    size_t localindex;
    pack_value *stack;
    size_t stackalloc;
    size_t stackindex;
    pack_local_value *capture;
    size_t caploc;
    size_t capc;
};

struct pack_local_value {
    bool ismut;
    union {
        pack_value imut;
        pack_value *mut;
    } value;
};

pack_value pack_value_num(pack_state *, double);
pack_value pack_value_bool(pack_state *, bool);
pack_value pack_value_str(pack_state *, char *);
pack_value pack_value_cfunc(
    pack_state *,
    pack_value (*)(pack_state *, uint16_t, pack_local_value *, uint16_t, pack_value *)
);
pack_value pack_value_packfunc(pack_state *, uint16_t, pack_local_value *, uint32_t);
pack_value pack_value_pack_func(pack_state *, pack_func);
pack_value pack_value_nil(pack_state *);

void runfile(pack_state *, FILE *);
pack_value runpack_program(pack_state *, size_t, pack_local_value *, size_t, pack_value *, size_t);

#include "state.h"
#include "clib.h"
#include "lib.h"