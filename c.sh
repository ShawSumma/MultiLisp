pypy3 main.py -i $1 -o out.pack &&
    clang interp.c lib.c -o pack -lm -lgc -O3 -std=c99 &&
    time ./pack out.pack