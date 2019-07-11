python3 main.py -i $1 -o out.pack &&
    python3 main.py -i $1 -o out.txt &&
    clang-8 c/interp.c c/lib.c c/clib.c c/state.c c/main.c c/vector.c -o c/pack -lm -lgc -lffi -ldl -Ofast -std=c99 -g &&
    valgrind --tool=callgrind ./c/pack out.pack && rm callgrind*
    # time ./c/pack out.pack