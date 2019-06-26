#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <gc.h>
#include <time.h>

struct func;
typedef struct func func;
struct value;
typedef struct value value;
struct opcode;
typedef struct opcode opcode;
struct state;
typedef struct state state;
struct program;
typedef struct program program;
struct pair;
typedef struct pair pair;

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
        value(*cfn)(program, uint16_t, value*);
    } value;
    enum {
        FUNC_FROM_C,
        FUNC_FROM_PACK,
    } type;
};

struct value {
    enum {
        VALUE_TYPE_NIL,
        VALUE_TYPE_NONE,
        VALUE_TYPE_NUMBER,
        VALUE_TYPE_STRING,
        VALUE_TYPE_FUNCTION,
        VALUE_TYPE_BOOLEAN,
    } type;
    union {
        pair p;
        double n;
        char *s;
        func f;
        bool b;
    } value;
};

struct pair {
    value left;
    value right;
};

struct opcode {
    optype type;
    uint32_t value;
};

struct program {
    size_t opcount;
    opcode *opcodes;
    size_t valcount;
    value *vals;
};

value value_num(double);
value value_bool(bool);
value value_str(char *);
value value_cfunc(value(*)(program, uint16_t, value*));
value value_packfunc(uint16_t, value **, uint32_t);
value value_func(func);
value value_nil();

void runfile(FILE *);
void runprogram(program, size_t);

#include "lib.h"