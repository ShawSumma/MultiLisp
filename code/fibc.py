def add(a):
    return lambda b: a + b
def sub(a):
    return lambda b: a - b
def fib(n):
    return n if n < 2 else add(fib(sub(n)(2)))(fib(sub(n)(1)))
print(fib(35))