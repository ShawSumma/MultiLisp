
#include "interp.h"

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