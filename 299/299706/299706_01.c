#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// gcc-omp 299706_01.c -o 299706_01

typedef struct {
    int64_t a, b, c;
} Triple;

int64_t solve(int64_t n) {
    // 十分なサイズ（経験的に n=1e12 でも足りる）
    size_t capacity = 1000000;
    Triple *stack = malloc(sizeof(Triple) * capacity);
    size_t top = 0;

    stack[top++] = (Triple){3, 4, 5};

    int64_t sum = 0;

    while (top > 0) {
        Triple t = stack[--top];
        int64_t a = t.a;
        int64_t b = t.b;
        int64_t c = t.c;

        int64_t p = a + b + c;
        if (p > n) continue;

        sum += n / p;

        // 必要ならスタック拡張
        if (top + 3 >= capacity) {
            capacity *= 2;
            stack = realloc(stack, sizeof(Triple) * capacity);
        }

        // 3変換
        stack[top++] = (Triple){
            a - 2*b + 2*c,
            2*a - b + 2*c,
            2*a - 2*b + 3*c
        };

        stack[top++] = (Triple){
            a + 2*b + 2*c,
            2*a + b + 2*c,
            2*a + 2*b + 3*c
        };

        stack[top++] = (Triple){
           -a + 2*b + 2*c,
           -2*a + b + 2*c,
           -2*a + 2*b + 3*c
        };
    }

    free(stack);
    return sum;
}

int main() {
    for (int i = 1; i <= 11; i++) {
        int64_t n = 1;
        for (int j = 0; j < i; j++) n *= 10;
        printf("%lld\n", solve(n));
    }
    return 0;
}

