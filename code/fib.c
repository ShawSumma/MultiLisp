#include <stdio.h>
#include <stdint.h>

uint64_t fib(uint32_t n) {
    if (n < 2) {
        return n;
    }
    else {
        return fib(n-2) + fib(n-1);
    }
}

int main() {
    printf("%lu\n", fib(37));
}