#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <gc.h>
#include <dlfcn.h>
#include <ffi.h>

#define gc_malloc GC_malloc
#define gc_realloc GC_realloc
#define gc_free GC_free
// #define gc_malloc malloc
// #define gc_realloc realloc
// #define gc_free free

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
struct pack_vector;
typedef struct pack_vector pack_vector;
struct pack_ffi_type;
typedef struct pack_ffi_type pack_ffi_type;
struct pack_state;
typedef struct pack_state pack_state;
struct pack_local_value;
typedef struct pack_local_value pack_local_value;

typedef enum {
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
    PACK_VALUE_TYPE_NIL,
    PACK_VALUE_TYPE_NUMBER,
    PACK_VALUE_TYPE_BOOLEAN,
    PACK_VALUE_TYPE_FUNCTION,
    PACK_VALUE_TYPE_STRING,
    PACK_VALUE_TYPE_LIBRARY,
    PACK_VALUE_TYPE_FFI_TYPE,
    PACK_VALUE_TYPE_POINTER,
    PACK_VALUE_TYPE_VECTOR,
    // PACK_VALUE_TYPE_PAIR,
} pack_type;

typedef enum {
    PACK_FFI_TYPE_VOID,
    PACK_FFI_TYPE_INT8,
    PACK_FFI_TYPE_INT16,
    PACK_FFI_TYPE_INT32,
    PACK_FFI_TYPE_INT64,
    PACK_FFI_TYPE_UINT8,
    PACK_FFI_TYPE_UINT16,
    PACK_FFI_TYPE_UINT32,
    PACK_FFI_TYPE_UINT64,
    PACK_FFI_TYPE_FLOAT,
    PACK_FFI_TYPE_DOUBLE,
    PACK_FFI_TYPE_POINTER,
    PACK_FFI_TYPE_CHAR_POINTER,
} pack_ffi_type_id;

typedef enum {
    PACK_UNMARKED = 0,
    PACK_MARKED = 1,
} pack_mark;

struct pack_ffi_type {
    ffi_type *type;
    pack_ffi_type_id type_id;
};

struct pack_func {
    union {
        struct {
            pack_local_value *cap;
            size_t place;
            size_t capc;
        } place;
        struct {
            pack_local_value *cap;
            pack_value (*cfn)(pack_state *, size_t, pack_local_value *, size_t, pack_value *);
            size_t capc;
        } cfn;
        struct {
            void *func;
            ffi_cif cif;
            pack_ffi_type ret_type;
            pack_ffi_type *arg_types;
            size_t argc;
        } *ffi;
    } value;
    enum {
        FUNC_FROM_C,
        FUNC_FROM_FFI,
        FUNC_FROM_PACK,
    } type;
};

struct pack_value {
    union {
        double number;
        char *string;
        bool boolean;
        pack_func *func;
        pack_pair *pair;
        void *library;
        pack_ffi_type type;
        pack_vector *vector;
        void *pointer;
    } value;
    pack_type type;
};

struct pack_pair {
    pack_value left;
    pack_value right;
};

struct pack_vector {
    size_t alloc;
    size_t count;
    pack_value *values;
};

struct pack_opcode {
    optype type;
    size_t value;
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
    union {
        pack_value imut;
        pack_value *mut;
    } value;
    bool ismut;
};

pack_value pack_value_num(pack_state *, double);
pack_value pack_value_library(pack_state *, void *);
pack_value pack_value_ffi_type(pack_state *, ffi_type *, pack_ffi_type_id);
pack_value pack_value_bool(pack_state *, bool);
pack_value pack_value_str(pack_state *, char *);
pack_value pack_value_cfunc(
    pack_state *,
    pack_value (*)(pack_state *, size_t, pack_local_value *, size_t, pack_value *)
);
pack_value pack_value_packfunc(pack_state *, size_t, pack_local_value *, size_t);
pack_value pack_value_pack_func(pack_state *, pack_func);
pack_value pack_value_nil(pack_state *);

void runfile(pack_state *, FILE *);
pack_value runpack_program(pack_state *, size_t, pack_local_value *, size_t, pack_value *, size_t);

#include "state.h"
#include "error.h"
#include "vector.h"
#include "clib.h"
#include "lib.h"