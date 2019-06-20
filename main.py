import sys
import walker
import tree
import math
import argparse
import shutil
import sys

sys.setrecursionlimit(100)
argparser = argparse.ArgumentParser()
argparser.add_argument('-i', type=str)
argparser.add_argument('-o', default=None,type=str)
argsparsed = argparser.parse_args(sys.argv[1:])

state = tree.State()
with open(argsparsed.i) as f:
    ast = state.parse(f.read())
conv = state.change(ast)
conv.get()
conv.captures()
# conv.walk()
walk = walker.Walker()
walk.walk(conv)
name, ext = argsparsed.o.split('.')
if ext == 'pack':
    with open(argsparsed.o, 'w') as f:
        for pl, i in enumerate(walk.code):
            f.write(str(i) + '\n')
else:
    raise Exception('unknown extension ' + ext)