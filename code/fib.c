#include <stdio.h>

double fib(double n) {
    if (n < 2) {
        return n;
    }
    else {
        return fib(n-2) + fib(n-1);
    }
}

int main() {
    printf("%lf\n", fib(35));
}