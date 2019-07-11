
#include "interp.h"

void *pack_gmprealloc(void *o, size_t i0, size_t s) {
    return gc_realloc(o, s);
}

void pack_gmpfree(void *v, size_t i0) {
    gc_free(v);
}

int main(int argc, char **argv) {
    mp_set_memory_functions(gc_malloc, pack_gmprealloc, pack_gmpfree);
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