pypy3 main.py -i $1 -o out.pack &&
    pypy3 main.py -i $1 -o out.txt &&
    clang interp.c lib.c clib.c state.c -o pack -lm -lgc -Ofast -std=gnu99 -g &&
    # ./pack out.pack
    time time ./pack out.pack
    # time valgrind --tool=callgrind ./pack out.pack