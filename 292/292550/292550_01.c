#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef __uint128_t u128;

enum {
    MAX_N = 7,
    DEFAULT_MAX_K = 40,
    MAX_SUMS2_BYTES = 512U * 1024U * 1024U
};

static const u128 U128_MAX_VALUE = (u128)(~(u128)0);

typedef struct {
    u128 sum;
    uint64_t mask;
} SumMask;

static bool add_u128_checked(u128 a, u128 b, u128 *out) {
    if (a > U128_MAX_VALUE - b) return false;
    *out = a + b;
    return true;
}

static bool mul_u128_checked(u128 a, u128 b, u128 *out) {
    if (a == 0 || b == 0) {
        *out = 0;
        return true;
    }
    if (a > U128_MAX_VALUE / b) return false;
    *out = a * b;
    return true;
}

static bool pow_u128_checked(uint64_t base, uint32_t exp, u128 *out) {
    u128 result = 1;
    u128 cur = base;
    uint32_t e = exp;

    while (e > 0) {
        if (e & 1U) {
            if (!mul_u128_checked(result, cur, &result)) return false;
        }
        e >>= 1U;
        if (e == 0) break;
        if (!mul_u128_checked(cur, cur, &cur)) return false;
    }
    *out = result;
    return true;
}

static int cmp_u128(const void *a, const void *b) {
    u128 x = *(const u128 *)a;
    u128 y = *(const u128 *)b;
    return (x > y) - (x < y);
}

static int cmp_summask(const void *a, const void *b) {
    const SumMask *x = (const SumMask *)a;
    const SumMask *y = (const SumMask *)b;
    if (x->sum < y->sum) return -1;
    if (x->sum > y->sum) return 1;
    return (x->mask > y->mask) - (x->mask < y->mask);
}

static bool contains_sorted_u128(const u128 *arr, size_t n, u128 key) {
    size_t lo = 0;
    size_t hi = n;
    while (lo < hi) {
        size_t mid = lo + (hi - lo) / 2U;
        if (arr[mid] < key) lo = mid + 1U;
        else hi = mid;
    }
    return (lo < n && arr[lo] == key);
}

// Returns: 1 = found, 0 = not found, negative = error
static int find_mask_small(const u128 *vals, uint32_t n, u128 target, uint64_t *mask_out) {
    uint32_t a = n / 2U;
    uint32_t b = n - a;
    if (a >= 63U || b >= 63U) return -4;

    size_t cb = (size_t)1ULL << b;
    SumMask *right = (SumMask *)malloc(cb * sizeof(SumMask));
    if (!right) return -2;

    right[0].sum = 0;
    right[0].mask = 0;
    u128 cur = 0;
    uint64_t prev_g = 0;
    for (size_t idx = 1; idx < cb; ++idx) {
        uint64_t g = (uint64_t)(idx ^ (idx >> 1U));
        uint64_t diff = g ^ prev_g;
        unsigned bit = (unsigned)__builtin_ctzll(diff);
        u128 v = vals[a + bit];
        if (g & (1ULL << bit)) {
            if (!add_u128_checked(cur, v, &cur)) {
                free(right);
                return -3;
            }
        } else {
            if (cur < v) {
                free(right);
                return -3;
            }
            cur -= v;
        }
        right[idx].sum = cur;
        right[idx].mask = g;
        prev_g = g;
    }

    qsort(right, cb, sizeof(SumMask), cmp_summask);

    size_t ca = (size_t)1ULL << a;
    u128 cur_left = 0;
    uint64_t prev_left_g = 0;
    for (size_t idx = 0; idx < ca; ++idx) {
        uint64_t g = (uint64_t)(idx ^ (idx >> 1U));
        if (idx > 0) {
            uint64_t diff = g ^ prev_left_g;
            unsigned bit = (unsigned)__builtin_ctzll(diff);
            u128 v = vals[bit];
            if (g & (1ULL << bit)) {
                if (!add_u128_checked(cur_left, v, &cur_left)) {
                    free(right);
                    return -3;
                }
            } else {
                if (cur_left < v) {
                    free(right);
                    return -3;
                }
                cur_left -= v;
            }
            prev_left_g = g;
        }

        if (cur_left > target) continue;
        u128 need = target - cur_left;

        size_t lo = 0, hi = cb;
        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2U;
            if (right[mid].sum < need) lo = mid + 1U;
            else hi = mid;
        }
        if (lo < cb && right[lo].sum == need) {
            *mask_out = g | (right[lo].mask << a);
            free(right);
            return 1;
        }
    }

    free(right);
    return 0;
}

// Returns: 1 = found, 0 = not found, negative = error
static int find_subset_mask_mitm(const u128 *terms, uint32_t m, u128 target, uint64_t *mask_out) {
    uint32_t n1 = m / 2U;
    uint32_t n2 = m - n1;
    if (m >= 64U || n1 >= 63U || n2 >= 63U) return -4;

    size_t c2 = (size_t)1ULL << n2;
    if (c2 > SIZE_MAX / sizeof(u128)) return -4;
    if (c2 * sizeof(u128) > MAX_SUMS2_BYTES) return -5;

    u128 *sums2 = (u128 *)malloc(c2 * sizeof(u128));
    if (!sums2) return -2;

    sums2[0] = 0;
    u128 cur2 = 0;
    uint64_t prev_g2 = 0;
    for (size_t idx = 1; idx < c2; ++idx) {
        uint64_t g = (uint64_t)(idx ^ (idx >> 1U));
        uint64_t diff = g ^ prev_g2;
        unsigned bit = (unsigned)__builtin_ctzll(diff);
        u128 v = terms[n1 + bit];
        if (g & (1ULL << bit)) {
            if (!add_u128_checked(cur2, v, &cur2)) {
                free(sums2);
                return -3;
            }
        } else {
            if (cur2 < v) {
                free(sums2);
                return -3;
            }
            cur2 -= v;
        }
        sums2[idx] = cur2;
        prev_g2 = g;
    }

    qsort(sums2, c2, sizeof(u128), cmp_u128);

    size_t c1 = (size_t)1ULL << n1;
    u128 cur1 = 0;
    uint64_t prev_g1 = 0;
    for (size_t idx = 0; idx < c1; ++idx) {
        uint64_t g1 = (uint64_t)(idx ^ (idx >> 1U));
        if (idx > 0) {
            uint64_t diff = g1 ^ prev_g1;
            unsigned bit = (unsigned)__builtin_ctzll(diff);
            u128 v = terms[bit];
            if (g1 & (1ULL << bit)) {
                if (!add_u128_checked(cur1, v, &cur1)) {
                    free(sums2);
                    return -3;
                }
            } else {
                if (cur1 < v) {
                    free(sums2);
                    return -3;
                }
                cur1 -= v;
            }
            prev_g1 = g1;
        }

        if (cur1 > target) continue;
        u128 need = target - cur1;
        if (!contains_sorted_u128(sums2, c2, need)) continue;

        uint64_t mask2 = 0;
        int recon_rc = find_mask_small(terms + n1, n2, need, &mask2);
        if (recon_rc < 0) {
            free(sums2);
            return recon_rc;
        }
        if (recon_rc == 0) continue;
        *mask_out = g1 | (mask2 << n1);
        free(sums2);
        return 1;
    }

    free(sums2);
    return 0;
}

// Returns: 0=ok, negative=error
static int find_a_n_with_example(uint32_t n, uint32_t max_k, uint32_t *k_out, uint64_t *mask_out) {
    if (n == 0U) {
        *k_out = 1U;
        *mask_out = 1ULL; // {1^0} on left, {3^0} on right
        return 0;
    }

    if (max_k > UINT32_MAX / 2U) return -4;
    uint32_t max_m = 2U * max_k;
    if ((size_t)max_m > SIZE_MAX / sizeof(u128)) return -4;

    u128 *terms = (u128 *)malloc((size_t)max_m * sizeof(u128));
    if (!terms) return -2;

    uint32_t m = 0;
    u128 total = 0;

    for (uint32_t k = 1; k <= max_k; ++k) {
        uint64_t x1 = (uint64_t)(4U * k - 3U);
        uint64_t x2 = (uint64_t)(4U * k - 1U);
        u128 v1 = 0, v2 = 0;

        if (!pow_u128_checked(x1, n, &v1) || !pow_u128_checked(x2, n, &v2)) {
            free(terms);
            return -3;
        }
        if (!add_u128_checked(total, v1, &total) || !add_u128_checked(total, v2, &total)) {
            free(terms);
            return -3;
        }

        terms[m++] = v1;
        terms[m++] = v2;
        if (m >= 64U) {
            free(terms);
            return -4;
        }

        if ((total & (u128)1) != 0) continue;
        uint64_t mask = 0;
        int rc = find_subset_mask_mitm(terms, m, total / (u128)2, &mask);
        if (rc == 1) {
            *k_out = k;
            *mask_out = mask;
            free(terms);
            return 0;
        }
        if (rc < 0) {
            free(terms);
            return rc;
        }
    }

    free(terms);
    return -1;
}

static void print_side(uint32_t n, uint32_t m, uint64_t mask, bool selected) {
    bool first = true;
    for (uint32_t i = 0; i < m; ++i) {
        bool in = ((mask >> i) & 1ULL) != 0ULL;
        if (in != selected) continue;
        if (!first) printf(" + ");
        printf("%" PRIu32 "^%" PRIu32, 2U * i + 1U, n);
        first = false;
    }
}

int main(void) {
    uint32_t a[MAX_N + 1];
    uint64_t masks[MAX_N + 1];

    for (uint32_t n = 0; n <= MAX_N; ++n) {
        int rc = find_a_n_with_example(n, DEFAULT_MAX_K, &a[n], &masks[n]);
        if (rc != 0) {
            fprintf(stderr, "failed at n=%" PRIu32 " (rc=%d)\n", n, rc);
            return 1;
        }
    }

    for (uint32_t n = 0; n <= MAX_N; ++n) {
        if (n > 0) printf(", ");
        printf("%" PRIu32, a[n]);
    }
    printf(",\n");

    for (uint32_t n = 0; n <= MAX_N; ++n) {
        uint32_t m = 2U * a[n];
        printf("n = %" PRIu32 "\n", n);
        print_side(n, m, masks[n], true);
        printf(" = ");
        print_side(n, m, masks[n], false);
        printf(".\n");
    }

    return 0;
}
