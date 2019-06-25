pypy3 main.py -i $1 -o out/out.pack
clang interp.c -o pack -lm -lgc -Ofast
time ./pack out/out.pack
# rm callgrind*