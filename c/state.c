#include "interp.h"

pack_state *pack_state_new() {
    pack_state *state = gc_malloc(sizeof(pack_state));
    state->localalloc = 16;
    state->locals = gc_malloc(sizeof(pack_value *) * state->localalloc);
    state->localindex = 0;
    state->stackalloc = 32;
    state->stack = gc_malloc(sizeof(pack_value) * state->stackalloc);
    state->stackindex = 0;
    state->caploc = 8;
    state->capture = gc_malloc(sizeof(pack_value *) * state->caploc);
    state->capc = 0;
    return state;
}