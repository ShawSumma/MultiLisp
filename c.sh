pypy3 main.py -i $1 -o out.pack &&
    gcc interp.c -o pack -lm -lgc -Ofast &&
    time ./pack out.pack
# rm callgrind*