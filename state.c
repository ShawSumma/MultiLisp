#include "interp.h"

pack_state *pack_state_new() {
    pack_state *state = malloc(sizeof(pack_state));
    return state;
}