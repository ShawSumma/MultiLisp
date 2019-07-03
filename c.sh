python3 main.py -i $1 -o out.pack &&
    python3 main.py -i $1 -o out.txt &&
    gcc-9 c/interp.c c/lib.c c/clib.c c/state.c c/main.c -o c/pack -lm -lgc -O3 -g -std=c99 &&
    time ./c/pack out.pack
# clang c/read.c -lgc -lm -o c/pack -g && valgrind --tool=callgrind ./c/pack && rm callgrind*