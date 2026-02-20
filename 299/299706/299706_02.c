#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <omp.h> // 並列化用

// gcc-omp 299706_02.c -o 299706_02

// N=10^16 の時、dの上限は約28,867,513
#define MAX_SQRT_N 30000000
int8_t mu[MAX_SQRT_N + 1];

void sieve_mobius(int n) {
    static int p[MAX_SQRT_N + 1];
    static int primes[MAX_SQRT_N / 10];
    int pcnt = 0;
    for (int i = 0; i <= n; i++) mu[i] = 0, p[i] = 0;
    mu[1] = 1;
    for (int i = 2; i <= n; i++) {
        if (!p[i]) { primes[pcnt++] = i; mu[i] = -1; }
        for (int j = 0; j < pcnt && i * primes[j] <= n; j++) {
            p[i * primes[j]] = 1;
            if (i % primes[j] == 0) { mu[i * primes[j]] = 0; break; }
            mu[i * primes[j]] = -mu[i];
        }
    }
}

int64_t range_sum_floor(int64_t V, int64_t low, int64_t high) {
    if (low > high || low > V) return 0;
    if (high > V) high = V;
    int64_t sum = 0;
    for (int64_t i = low, next_i; i <= high; i = next_i + 1) {
        int64_t val = V / i;
        if (val == 0) break;
        next_i = V / val;
        if (next_i > high) next_i = high;
        sum += (next_i - i + 1) * val;
    }
    return sum;
}

int64_t solve(int64_t N) {
    int64_t total_sum = 0;
    int64_t limit = (int64_t)sqrt((double)N / 12.0);
    sieve_mobius((int)limit);

    #pragma omp parallel for reduction(+:total_sum) schedule(dynamic)
    for (int64_t d = 1; d <= limit; d += 2) {
        if (mu[d] == 0) continue;

        // d*d がオーバーフローしないよう、確実に int64_t で計算
        int64_t d_sq_2 = 2LL * d * d; 
        int64_t K = N / d_sq_2;
        
        int64_t f_K = 0;
        // s*s/2 が K を超えない範囲でループ
        for (int64_t s = 3; s * s <= 2LL * K; s += 2) {
            int64_t V = K / s;
            int64_t m_start = s / 2 + 1;
            int64_t m_end = s - 1;
            f_K += range_sum_floor(V, m_start, m_end);
        }
        
        if (mu[d] == 1) total_sum += f_K;
        else total_sum -= f_K;
    }
    return total_sum;
}

int main() {
    printf("%-3s | %-14s | %-20s | %-8s\n", "i", "N = 10^i", "Result", "Time");
    printf("------------------------------------------------------------\n");
    for (int i = 1; i <= 16; i++) { // 16まで回す
        int64_t N = 1;
        for (int j = 0; j < i; j++) N *= 10;
        
        double start = omp_get_wtime();
        int64_t res = solve(N);
        double end = omp_get_wtime();
        
        printf("%-3d | 10^%-11d | %-20lld | %.3f s\n", i, i, res, end - start);
    }
    return 0;
}