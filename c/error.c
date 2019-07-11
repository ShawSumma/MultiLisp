#include "interp.h"

void pack_error_argindex(pack_state *state, char *name, size_t num) {
    fprintf(stderr, "bad argument %zu to %s\n", num, name);
}