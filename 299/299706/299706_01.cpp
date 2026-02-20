#include <iostream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <omp.h>

using namespace std;

// g++-omp 299706_01.cpp -o 299706_01

// 10^16に対応するための最大値。グローバルに一度だけ確保。
const int MAX_LIMIT = 30000000;
int8_t mu[MAX_LIMIT + 1];

void sieve_mobius(int n) {
    static int primes[MAX_LIMIT / 10];
    static bool is_not_prime[MAX_LIMIT + 1]; // bool配列(非特化)で高速化
    int pcnt = 0;
    mu[1] = 1;
    for (int i = 2; i <= n; ++i) {
        if (!is_not_prime[i]) {
            primes[pcnt++] = i;
            mu[i] = -1;
        }
        for (int j = 0; j < pcnt; ++j) {
            int target = i * primes[j];
            if (target > n) break;
            is_not_prime[target] = true;
            if (i % primes[j] == 0) {
                mu[target] = 0;
                break;
            }
            mu[target] = -mu[i];
        }
    }
}

inline long long range_sum_floor(long long V, long long low, long long high) {
    if (low > high || low > V) return 0;
    if (high > V) high = V;
    long long sum = 0;
    for (long long i = low, next_i; i <= high; i = next_i + 1) {
        long long val = V / i;
        if (val == 0) break;
        next_i = V / val;
        if (next_i > high) next_i = high;
        sum += (next_i - i + 1) * val;
    }
    return sum;
}

long long solve(long long N) {
    long long total_sum = 0;
    // long double を使用して 10^16 の精度誤差を回避
    long long limit = static_cast<long long>(sqrtl((long double)N / 12.0));

    #pragma omp parallel for reduction(+:total_sum) schedule(dynamic)
    for (long long d = 1; d <= limit; d += 2) {
        if (mu[d] == 0) continue;

        long long K = N / (2LL * d * d);
        long long f_K = 0;
        
        for (long long s = 3; s * s <= 2LL * K; s += 2) {
            long long V = K / s;
            long long m_start = s / 2 + 1;
            long long m_end = s - 1;
            f_K += range_sum_floor(V, m_start, m_end);
        }
        
        if (mu[d] == 1) total_sum += f_K;
        else total_sum -= f_K;
    }
    return total_sum;
}

int main() {
    // 最初に一度だけ最大範囲で篩をかける
    sieve_mobius(MAX_LIMIT);

    cout << fixed << setprecision(3);
    cout << " i | N = 10^i   | Result               | Time   " << endl;
    cout << "---+------------+----------------------+----------" << endl;

    for (int i = 1; i <= 16; ++i) {
        long long N = 1;
        for (int j = 0; j < i; j++) N *= 10;
        
        double start = omp_get_wtime();
        long long res = solve(N);
        double end = omp_get_wtime();
        
        cout << setw(2) << right << i << " | 10^" 
             << setw(7) << left << i 
             << " | " << setw(20) << right << res 
             << " | " << setw(8) << right << (end - start) << " s" << endl;
    }
    return 0;
}