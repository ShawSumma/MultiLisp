python3 main.py -i $1 -o out.pack &&
    # python3 main.py -i $1 -o out.txt &&
    clang c/interp.c c/lib.c c/clib.c c/state.c c/main.c -o c/pack -lm -lgc -O1 -g -std=c99 &&
    time ./c/pack out.pack
# clang c/read.c -lgc -lm -o c/pack -g && valgrind --tool=callgrind ./c/pack && rm callgrind*