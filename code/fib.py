def sub(a):
    return lambda b: a-b
def fib(n):
    return n if n < 2 else fib(sub(n)(1)) + fib(sub(n)(2))
print(fib(32))