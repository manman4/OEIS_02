#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <vector>

using u128 = __uint128_t;

namespace {

constexpr uint32_t MAX_N = 7;
constexpr uint32_t DEFAULT_MAX_K = 40;
constexpr std::size_t MAX_SUMS2_BYTES = 512ULL * 1024ULL * 1024ULL;
constexpr u128 U128_MAX_VALUE = static_cast<u128>(~static_cast<u128>(0));

struct SumMask {
    u128 sum;
    uint64_t mask;
};

bool add_u128_checked(u128 a, u128 b, u128 &out) {
    if (a > U128_MAX_VALUE - b) return false;
    out = a + b;
    return true;
}

bool mul_u128_checked(u128 a, u128 b, u128 &out) {
    if (a == 0 || b == 0) {
        out = 0;
        return true;
    }
    if (a > U128_MAX_VALUE / b) return false;
    out = a * b;
    return true;
}

bool pow_u128_checked(uint64_t base, uint32_t exp, u128 &out) {
    u128 result = 1;
    u128 cur = base;
    uint32_t e = exp;
    while (e > 0) {
        if ((e & 1U) != 0U) {
            if (!mul_u128_checked(result, cur, result)) return false;
        }
        e >>= 1U;
        if (e == 0) break;
        if (!mul_u128_checked(cur, cur, cur)) return false;
    }
    out = result;
    return true;
}

bool contains_sorted_u128(const std::vector<u128> &arr, u128 key) {
    auto it = std::lower_bound(arr.begin(), arr.end(), key);
    return it != arr.end() && *it == key;
}

// Returns: 1 = found, 0 = not found, negative = error
int find_mask_small(const std::vector<u128> &vals, uint32_t n, u128 target, uint64_t &mask_out) {
    const uint32_t a = n / 2U;
    const uint32_t b = n - a;
    if (a >= 63U || b >= 63U) return -4;

    const std::size_t cb = static_cast<std::size_t>(1ULL) << b;
    std::vector<SumMask> right;
    try {
        right.resize(cb);
    } catch (...) {
        return -2;
    }

    right[0] = {0, 0};
    u128 cur = 0;
    uint64_t prev_g = 0;
    for (std::size_t idx = 1; idx < cb; ++idx) {
        const uint64_t g = static_cast<uint64_t>(idx ^ (idx >> 1U));
        const uint64_t diff = g ^ prev_g;
        const unsigned bit = static_cast<unsigned>(__builtin_ctzll(diff));
        const u128 v = vals[a + bit];
        if ((g & (1ULL << bit)) != 0ULL) {
            if (!add_u128_checked(cur, v, cur)) return -3;
        } else {
            if (cur < v) return -3;
            cur -= v;
        }
        right[idx] = {cur, g};
        prev_g = g;
    }

    std::sort(right.begin(), right.end(), [](const SumMask &x, const SumMask &y) {
        if (x.sum != y.sum) return x.sum < y.sum;
        return x.mask < y.mask;
    });

    const std::size_t ca = static_cast<std::size_t>(1ULL) << a;
    u128 cur_left = 0;
    uint64_t prev_left_g = 0;
    for (std::size_t idx = 0; idx < ca; ++idx) {
        const uint64_t g = static_cast<uint64_t>(idx ^ (idx >> 1U));
        if (idx > 0) {
            const uint64_t diff = g ^ prev_left_g;
            const unsigned bit = static_cast<unsigned>(__builtin_ctzll(diff));
            const u128 v = vals[bit];
            if ((g & (1ULL << bit)) != 0ULL) {
                if (!add_u128_checked(cur_left, v, cur_left)) return -3;
            } else {
                if (cur_left < v) return -3;
                cur_left -= v;
            }
            prev_left_g = g;
        }

        if (cur_left > target) continue;
        const u128 need = target - cur_left;
        auto it = std::lower_bound(
            right.begin(), right.end(), need,
            [](const SumMask &x, u128 v) { return x.sum < v; });
        if (it != right.end() && it->sum == need) {
            mask_out = g | (it->mask << a);
            return 1;
        }
    }
    return 0;
}

// Returns: 1 = found, 0 = not found, negative = error
int find_subset_mask_mitm(const std::vector<u128> &terms, uint32_t m, u128 target, uint64_t &mask_out) {
    const uint32_t n1 = m / 2U;
    const uint32_t n2 = m - n1;
    if (m >= 64U || n1 >= 63U || n2 >= 63U) return -4;

    const std::size_t c2 = static_cast<std::size_t>(1ULL) << n2;
    if (c2 > (std::numeric_limits<std::size_t>::max)() / sizeof(u128)) return -4;
    if (c2 * sizeof(u128) > MAX_SUMS2_BYTES) return -5;

    std::vector<u128> sums2;
    try {
        sums2.resize(c2);
    } catch (...) {
        return -2;
    }

    sums2[0] = 0;
    u128 cur2 = 0;
    uint64_t prev_g2 = 0;
    for (std::size_t idx = 1; idx < c2; ++idx) {
        const uint64_t g = static_cast<uint64_t>(idx ^ (idx >> 1U));
        const uint64_t diff = g ^ prev_g2;
        const unsigned bit = static_cast<unsigned>(__builtin_ctzll(diff));
        const u128 v = terms[n1 + bit];
        if ((g & (1ULL << bit)) != 0ULL) {
            if (!add_u128_checked(cur2, v, cur2)) return -3;
        } else {
            if (cur2 < v) return -3;
            cur2 -= v;
        }
        sums2[idx] = cur2;
        prev_g2 = g;
    }

    std::sort(sums2.begin(), sums2.end());

    const std::size_t c1 = static_cast<std::size_t>(1ULL) << n1;
    u128 cur1 = 0;
    uint64_t prev_g1 = 0;
    for (std::size_t idx = 0; idx < c1; ++idx) {
        const uint64_t g1 = static_cast<uint64_t>(idx ^ (idx >> 1U));
        if (idx > 0) {
            const uint64_t diff = g1 ^ prev_g1;
            const unsigned bit = static_cast<unsigned>(__builtin_ctzll(diff));
            const u128 v = terms[bit];
            if ((g1 & (1ULL << bit)) != 0ULL) {
                if (!add_u128_checked(cur1, v, cur1)) return -3;
            } else {
                if (cur1 < v) return -3;
                cur1 -= v;
            }
            prev_g1 = g1;
        }

        if (cur1 > target) continue;
        const u128 need = target - cur1;
        if (!contains_sorted_u128(sums2, need)) continue;

        std::vector<u128> right_terms(terms.begin() + n1, terms.begin() + m);
        uint64_t mask2 = 0;
        const int recon_rc = find_mask_small(right_terms, n2, need, mask2);
        if (recon_rc < 0) return recon_rc;
        if (recon_rc == 0) continue;
        mask_out = g1 | (mask2 << n1);
        return 1;
    }
    return 0;
}

// Returns: 0 = ok, negative = error
int find_a_n_with_example(uint32_t n, uint32_t max_k, uint32_t &k_out, uint64_t &mask_out) {
    if (n == 0U) {
        k_out = 1U;
        mask_out = 1ULL;
        return 0;
    }

    if (max_k > UINT32_MAX / 2U) return -4;
    const uint32_t max_m = 2U * max_k;
    if (static_cast<std::size_t>(max_m) > (std::numeric_limits<std::size_t>::max)() / sizeof(u128)) return -4;

    std::vector<u128> terms;
    try {
        terms.reserve(max_m);
    } catch (...) {
        return -2;
    }

    u128 total = 0;
    for (uint32_t k = 1; k <= max_k; ++k) {
        const uint64_t x1 = static_cast<uint64_t>(4U * k - 3U);
        const uint64_t x2 = static_cast<uint64_t>(4U * k - 1U);
        u128 v1 = 0, v2 = 0;

        if (!pow_u128_checked(x1, n, v1) || !pow_u128_checked(x2, n, v2)) return -3;
        if (!add_u128_checked(total, v1, total) || !add_u128_checked(total, v2, total)) return -3;

        terms.push_back(v1);
        terms.push_back(v2);
        const uint32_t m = static_cast<uint32_t>(terms.size());
        if (m >= 64U) return -4;

        if ((total & static_cast<u128>(1)) != 0) continue;
        uint64_t mask = 0;
        const int rc = find_subset_mask_mitm(terms, m, total / static_cast<u128>(2), mask);
        if (rc == 1) {
            k_out = k;
            mask_out = mask;
            return 0;
        }
        if (rc < 0) return rc;
    }

    return -1;
}

void print_side(uint32_t n, uint32_t m, uint64_t mask, bool selected) {
    bool first = true;
    for (uint32_t i = 0; i < m; ++i) {
        const bool in = ((mask >> i) & 1ULL) != 0ULL;
        if (in != selected) continue;
        if (!first) std::printf(" + ");
        std::printf("%" PRIu32 "^%" PRIu32, 2U * i + 1U, n);
        first = false;
    }
}

}  // namespace

int main() {
    uint32_t a[MAX_N + 1];
    uint64_t masks[MAX_N + 1];

    for (uint32_t n = 0; n <= MAX_N; ++n) {
        int rc = find_a_n_with_example(n, DEFAULT_MAX_K, a[n], masks[n]);
        if (rc != 0) {
            std::fprintf(stderr, "failed at n=%" PRIu32 " (rc=%d)\n", n, rc);
            return 1;
        }
    }

    for (uint32_t n = 0; n <= MAX_N; ++n) {
        if (n > 0) std::printf(", ");
        std::printf("%" PRIu32, a[n]);
    }
    std::printf(",\n");

    for (uint32_t n = 0; n <= MAX_N; ++n) {
        const uint32_t m = 2U * a[n];
        std::printf("n = %" PRIu32 "\n", n);
        print_side(n, m, masks[n], true);
        std::printf(" = ");
        print_side(n, m, masks[n], false);
        std::printf(".\n");
    }

    return 0;
}
