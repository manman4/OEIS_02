/*
 * A263987 -- exact transposed forbidden-board rook DP.
 *
 * A descent x,y is forbidden precisely when x > y and y does not divide x.
 * Inclusion-exclusion over nonattacking forbidden adjacencies gives
 *
 *     a(n) = sum_k (-1)^k r_k (n-k)!,
 *
 * where r_k is the k-th rook number of the forbidden board.
 *
 * This verification program is deliberately oriented differently from
 * 263987_01.c.  It processes COLUMNS from right to left and its DP mask means
 * "used rows".  For n=33..38 it keeps the 30 high rows and 30 high columns in
 * the main DP, then folds the one to six LOW columns into the final sum.  The
 * first program instead masks columns, processes rows, and folds high rows.
 * The two programs also use disjoint CRT primes.
 *
 * Build on the configured Apple Silicon Mac:
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -Xpreprocessor -fopenmp \
 *     -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     263987_02.c -lomp -lgmp -o 263987_02
 *
 * Normal output is b263987_1.txt beside the executable.  The temporary
 * b263987_1_part.txt is flushed after every completed term and atomically
 * renamed after success.  A263987 has OFFSET 0.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <gmp.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef _OPENMP
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#endif
#include <omp.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif

#if ULONG_MAX < UINT64_MAX
#error "263987_02 requires a platform with 64-bit unsigned long"
#endif

#define SEQUENCE_OFFSET 0
#define DEFAULT_MAX_N 20
#define DEFAULT_CHECK_N 18
#define MAX_SUPPORTED_N 38
#define KNOWN_MAX_N 25
#define BUILTIN_MAX_N 33
#define DEFAULT_MEMORY_MIB UINT64_C(2048)
#define PARALLEL_RESERVE_MIB UINT64_C(1024)
#define MIN_MEMORY_MIB UINT64_C(16)
#define MAX_MEMORY_MIB UINT64_C(65536)
#define MAX_MODULUS_COUNT 6
#ifndef DIRECT_MAX_N
#define DIRECT_MAX_N 32
#endif
#define BASE_MASK_BITS (DIRECT_MAX_N - 2)
#define MAX_FOLDED_COLUMNS 6

static const char *const builtin_terms[BUILTIN_MAX_N + 1] = {
    "1", "1", "2", "4", "14", "28", "164", "328", "2240",
    "9970", "63410", "126820", "1810514", "3621028",
    "31417838", "294911038", "3344414606", "6688829212",
    "121919523980", "243839047960", "5307482547686",
    "56885719183654", "468469574780468", "936939149560936",
    "33136077712470338", "249693200433310492",
    "2206501242897918500", "26341855214541714358",
    "490506563674032534392", "981013127348065068784",
    "35754454366971755354786", "71508908733943510709572",
    "2245859537853874964579684",
    "30878686555260197866372216"
};

typedef struct {
    uint64_t transitions;
    double seconds;
} DpStats;

static void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static double monotonic_seconds(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        die("clock_gettime failed");
    }
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static int parse_n(const char *text, const char *label)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < SEQUENCE_OFFSET || value > MAX_SUPPORTED_N) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                label, SEQUENCE_OFFSET, MAX_SUPPORTED_N, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static uint64_t memory_budget_bytes(void)
{
    const char *text = getenv("A263987_MEMORY_MIB");
    uint64_t mib = DEFAULT_MEMORY_MIB;
    if (text != NULL && *text != '\0') {
        char *end = NULL;
        errno = 0;
        unsigned long long parsed = strtoull(text, &end, 10);
        if (errno != 0 || end == text || *end != '\0' ||
            parsed < MIN_MEMORY_MIB || parsed > MAX_MEMORY_MIB) {
            fprintf(stderr,
                    "error: A263987_MEMORY_MIB must be in %" PRIu64
                    "..%" PRIu64 ": %s\n",
                    MIN_MEMORY_MIB, MAX_MEMORY_MIB, text);
            exit(EXIT_FAILURE);
        }
        mib = (uint64_t)parsed;
    }
    return mib << 20;
}

static char *path_beside_executable(const char *argv0,
                                    const char *filename)
{
    char executable[PATH_MAX];
    char resolved[PATH_MAX];
    bool found = false;
#ifdef __APPLE__
    uint32_t size = (uint32_t)sizeof(executable);
    if (_NSGetExecutablePath(executable, &size) == 0) {
        found = true;
    }
#elif defined(__linux__)
    ssize_t length = readlink("/proc/self/exe", executable,
                              sizeof(executable) - 1);
    if (length >= 0) {
        executable[length] = '\0';
        found = true;
    }
#endif
    if (!found) {
        size_t length = strlen(argv0);
        if (length >= sizeof(executable)) {
            die("executable path is too long");
        }
        memcpy(executable, argv0, length + 1);
    }
    const char *base = realpath(executable, resolved);
    if (base == NULL) {
        base = executable;
    }
    const char *slash = strrchr(base, '/');
    size_t directory_length = slash == NULL ? 1 : (size_t)(slash - base);
    if (slash != NULL && directory_length == 0) {
        directory_length = 1;
    }
    size_t filename_length = strlen(filename);
    char *path = malloc(directory_length + 1 + filename_length + 1);
    if (path == NULL) {
        die("could not allocate an output path");
    }
    if (slash == NULL) {
        path[0] = '.';
    } else if (slash == base) {
        path[0] = '/';
    } else {
        memcpy(path, base, directory_length);
    }
    path[directory_length] = '/';
    memcpy(path + directory_length + 1, filename, filename_length + 1);
    return path;
}

static bool is_prime_int(int n)
{
    if (n < 2) {
        return false;
    }
    for (int divisor = 2; divisor <= n / divisor; ++divisor) {
        if (n % divisor == 0) {
            return false;
        }
    }
    return true;
}

static uint64_t multiply_mod(uint64_t left, uint64_t right,
                             uint64_t modulus)
{
    if (modulus == 0) {
        die("zero modular arithmetic modulus");
    }
    return (uint64_t)(((__uint128_t)left * right) % modulus);
}

static uint64_t power_mod(uint64_t base, uint64_t exponent,
                          uint64_t modulus)
{
    uint64_t result = 1;
    while (exponent != 0) {
        if ((exponent & 1U) != 0) {
            result = multiply_mod(result, base, modulus);
        }
        base = multiply_mod(base, base, modulus);
        exponent >>= 1;
    }
    return result;
}

static uint64_t subtract_mod(uint64_t left, uint64_t right,
                             uint64_t modulus)
{
    return left >= right ? left - right : modulus - (right - left);
}

/* Bit zero denotes the largest represented row, so new bits appear in
 * increasing bit order as columns are processed from right to left. */
static uint32_t forbidden_rows_for_column(int column, int n,
                                          int minimum_row)
{
    uint32_t mask = 0;
    int first = column + 1;
    if (first < minimum_row) {
        first = minimum_row;
    }
    for (int row = first; row <= n; ++row) {
        if (row % column != 0) {
            mask |= UINT32_C(1) << (unsigned)(n - row);
        }
    }
    return mask;
}

static bool direct_transposed_residue(uint64_t *output, int n,
                                      uint64_t modulus, DpStats *stats)
{
    memset(stats, 0, sizeof(*stats));
    if (modulus == 0 || modulus > INT32_MAX) {
        die("transposed DP modulus is outside the safe uint32_t range");
    }
    double start = monotonic_seconds();
    unsigned bits = n >= 2 ? (unsigned)(n - 2) : 0U;
    uint32_t state_count = UINT32_C(1) << bits;
    uint32_t p = (uint32_t)modulus;
    uint32_t *dp = calloc((size_t)state_count, sizeof(*dp));
    if (dp == NULL) {
        return false;
    }
    dp[0] = 1;

    uint32_t old_limit = 1;
    for (int column = n - 1; column >= 2; --column) {
        uint32_t forbidden =
            forbidden_rows_for_column(column, n, 3);
        for (uint32_t source = old_limit; source != 0;) {
            --source;
            uint32_t value = dp[source];
            if (value == 0) {
                continue;
            }
            uint32_t available = forbidden & ~source;
            while (available != 0) {
                uint32_t bit = available & (UINT32_C(0) - available);
                uint32_t target = source | bit;
                uint32_t sum = dp[target] + value;
                dp[target] = sum >= p ? sum - p : sum;
                ++stats->transitions;
                available ^= bit;
            }
        }
        old_limit <<= 1;
    }
    if (old_limit != state_count) {
        free(dp);
        die("direct transposed-DP active range invariant failed");
    }

    uint32_t factorial[MAX_SUPPORTED_N + 1];
    factorial[0] = 1;
    for (int value = 1; value <= n; ++value) {
        factorial[value] = (uint32_t)(
            ((uint64_t)factorial[value - 1] * (unsigned)value) % p);
    }
    uint32_t result = 0;
    for (uint32_t mask = 0; mask < state_count; ++mask) {
        unsigned rooks = (unsigned)__builtin_popcount(mask);
        uint32_t term = (uint32_t)(
            ((uint64_t)dp[mask] * factorial[n - (int)rooks]) % p);
        if ((rooks & 1U) == 0) {
            uint32_t sum = result + term;
            result = sum >= p ? sum - p : sum;
        } else {
            result = result >= term ? result - term
                                    : p - (term - result);
        }
    }
    *output = result;
    stats->seconds = monotonic_seconds() - start;
    free(dp);
    return true;
}

/*
 * For t=n-DIRECT_MAX_N, the main board consists of high rows t+3..n and
 * high columns t+2..n-1.  Both sides have BASE_MASK_BITS elements.  The low
 * columns 2..t+1 are folded after the transposed DP.  Their remaining row
 * candidates comprise unused high rows plus the t low rows 3..t+2.
 */
static bool folded_transposed_residue(uint64_t *output, int n,
                                      uint64_t modulus, DpStats *stats)
{
    memset(stats, 0, sizeof(*stats));
    if (modulus == 0 || modulus > INT32_MAX) {
        die("folded transposed-DP modulus is outside the safe uint32_t range");
    }
    int folded_columns = n - DIRECT_MAX_N;
    if (folded_columns < 1 || folded_columns > MAX_FOLDED_COLUMNS) {
        die("folded transposed-DP column count is out of range");
    }
    double start = monotonic_seconds();
    uint32_t state_count = UINT32_C(1) << BASE_MASK_BITS;
    uint32_t p = (uint32_t)modulus;
    uint32_t *dp = calloc((size_t)state_count, sizeof(*dp));
    if (dp == NULL) {
        return false;
    }
    dp[0] = 1;

    int minimum_base_row = folded_columns + 3;
    int minimum_base_column = folded_columns + 2;
    uint32_t old_limit = 1;
    for (int column = n - 1; column >= minimum_base_column; --column) {
        uint32_t forbidden = forbidden_rows_for_column(
            column, n, minimum_base_row);
        for (uint32_t source = old_limit; source != 0;) {
            --source;
            uint32_t value = dp[source];
            if (value == 0) {
                continue;
            }
            uint32_t available = forbidden & ~source;
            while (available != 0) {
                uint32_t bit = available & (UINT32_C(0) - available);
                uint32_t target = source | bit;
                uint32_t sum = dp[target] + value;
                dp[target] = sum >= p ? sum - p : sum;
                ++stats->transitions;
                available ^= bit;
            }
        }
        old_limit <<= 1;
    }
    if (old_limit != state_count) {
        free(dp);
        die("folded transposed-DP base range invariant failed");
    }

    uint32_t high_row_masks[MAX_FOLDED_COLUMNS] = { 0 };
    uint8_t low_row_masks[MAX_FOLDED_COLUMNS] = { 0 };
    for (int index = 0; index < folded_columns; ++index) {
        int column = index + 2;
        for (int row = minimum_base_row; row <= n; ++row) {
            if (row % column != 0) {
                high_row_masks[index] |=
                    UINT32_C(1) << (unsigned)(n - row);
            }
        }
        for (int row = 3; row < minimum_base_row; ++row) {
            if (row > column && row % column != 0) {
                low_row_masks[index] |=
                    (uint8_t)(1U << (unsigned)(row - 3));
            }
        }
    }

    unsigned folded_state_count = 1U << (unsigned)folded_columns;
    uint32_t high_intersection[1U << MAX_FOLDED_COLUMNS] = { 0 };
    uint8_t low_intersection[1U << MAX_FOLDED_COLUMNS] = { 0 };
    uint8_t subset_size[1U << MAX_FOLDED_COLUMNS] = { 0 };
    int64_t block_factorial[1U << MAX_FOLDED_COLUMNS] = { 0 };
    uint32_t all_high_rows =
        (UINT32_C(1) << BASE_MASK_BITS) - UINT32_C(1);
    uint8_t all_low_rows =
        (uint8_t)((1U << (unsigned)folded_columns) - 1U);
    for (unsigned subset = 1; subset < folded_state_count; ++subset) {
        unsigned bit = subset & (0U - subset);
        unsigned index = (unsigned)__builtin_ctz(bit);
        unsigned rest = subset ^ bit;
        uint32_t high = rest == 0 ? all_high_rows
                                  : high_intersection[rest];
        uint8_t low = rest == 0 ? all_low_rows
                                : low_intersection[rest];
        high_intersection[subset] = high & high_row_masks[index];
        low_intersection[subset] =
            (uint8_t)(low & low_row_masks[index]);
        subset_size[subset] = (uint8_t)__builtin_popcount(subset);
        int64_t factorial = 1;
        for (unsigned factor = 2; factor < subset_size[subset]; ++factor) {
            factorial *= (int64_t)factor;
        }
        block_factorial[subset] = factorial;
    }

    uint32_t factorial[MAX_SUPPORTED_N + 1];
    factorial[0] = 1;
    for (int value = 1; value <= n; ++value) {
        factorial[value] = (uint32_t)(
            ((uint64_t)factorial[value - 1] * (unsigned)value) % p);
    }

    int evaluation_threads = 1;
#ifdef _OPENMP
    evaluation_threads = omp_get_max_threads();
#endif
    uint64_t *partial = calloc((size_t)evaluation_threads,
                               sizeof(*partial));
    if (partial == NULL) {
        free(dp);
        return false;
    }

#ifdef _OPENMP
#pragma omp parallel num_threads(evaluation_threads)
#endif
    {
        int thread_index = 0;
        int thread_count = 1;
#ifdef _OPENMP
        thread_index = omp_get_thread_num();
        thread_count = omp_get_num_threads();
#endif
        uint64_t begin =
            ((uint64_t)state_count * (unsigned)thread_index) /
            (unsigned)thread_count;
        uint64_t end =
            ((uint64_t)state_count * (unsigned)(thread_index + 1)) /
            (unsigned)thread_count;
        uint64_t local = 0;
        for (uint64_t state_index = begin; state_index < end;
             ++state_index) {
            uint32_t used_high_rows = (uint32_t)state_index;
            uint32_t base_value = dp[used_high_rows];
            if (base_value == 0) {
                continue;
            }
            unsigned base_rooks =
                (unsigned)__builtin_popcount(used_high_rows);
            uint8_t degree[1U << MAX_FOLDED_COLUMNS] = { 0 };
            int64_t injective[1U << MAX_FOLDED_COLUMNS] = { 0 };
            uint64_t rook_number[MAX_FOLDED_COLUMNS + 1] = { 0 };
            injective[0] = 1;
            rook_number[0] = 1;

            for (unsigned subset = 1;
                 subset < folded_state_count; ++subset) {
                degree[subset] = (uint8_t)(
                    __builtin_popcount(high_intersection[subset] &
                                       ~used_high_rows) +
                    __builtin_popcount((unsigned)low_intersection[subset]));
            }
            /* Set-partition Möbius inversion counts injections of each
             * selected set of low columns into distinct available rows. */
            for (unsigned columns = 1;
                 columns < folded_state_count; ++columns) {
                unsigned pivot = columns & (0U - columns);
                int64_t count = 0;
                for (unsigned block = columns; block != 0;
                     block = (block - 1U) & columns) {
                    if ((block & pivot) == 0) {
                        continue;
                    }
                    int64_t term = block_factorial[block] *
                        (int64_t)degree[block] *
                        injective[columns ^ block];
                    if ((subset_size[block] & 1U) != 0) {
                        count += term;
                    } else {
                        count -= term;
                    }
                }
                if (count < 0) {
                    die("negative folded-column matching count");
                }
                injective[columns] = count;
                rook_number[subset_size[columns]] += (uint64_t)count;
            }

            for (int added = 0; added <= folded_columns; ++added) {
                int remaining = n - (int)base_rooks - added;
                uint64_t term = multiply_mod(
                    base_value, rook_number[added] % modulus, modulus);
                term = multiply_mod(term, factorial[remaining], modulus);
                if (((base_rooks + (unsigned)added) & 1U) == 0) {
                    local += term;
                    if (local >= modulus) {
                        local -= modulus;
                    }
                } else {
                    local = local >= term ? local - term
                                          : modulus - (term - local);
                }
            }
        }
        partial[thread_index] = local;
    }

    uint64_t result = 0;
    for (int thread = 0; thread < evaluation_threads; ++thread) {
        result += partial[thread];
        if (result >= modulus) {
            result -= modulus;
        }
    }
    *output = result;
    stats->seconds = monotonic_seconds() - start;
    free(partial);
    free(dp);
    return true;
}

static bool transposed_residue(uint64_t *output, int n,
                               uint64_t modulus, DpStats *stats)
{
    if (n <= DIRECT_MAX_N) {
        return direct_transposed_residue(output, n, modulus, stats);
    }
    return folded_transposed_residue(output, n, modulus, stats);
}

/* Start near 10^9, strictly below every prime used by 263987_01.c. */
static size_t choose_moduli(uint64_t *moduli, mpz_t product,
                            const mpz_t bound)
{
    mpz_t candidate;
    mpz_t prime;
    mpz_inits(candidate, prime, NULL);
    mpz_set_ui(candidate, 1000000000UL);
    mpz_set_ui(product, 1);
    size_t count = 0;
    while (mpz_cmp(product, bound) <= 0) {
        if (count >= MAX_MODULUS_COUNT) {
            mpz_clears(candidate, prime, NULL);
            die("too many CRT moduli for n! bound");
        }
        mpz_nextprime(prime, candidate);
        moduli[count++] = (uint64_t)mpz_get_ui(prime);
        if (moduli[count - 1] > INT32_MAX) {
            mpz_clears(candidate, prime, NULL);
            die("selected CRT modulus is too large");
        }
        mpz_mul_ui(product, product,
                   (unsigned long)moduli[count - 1]);
        mpz_set(candidate, prime);
    }
    mpz_clears(candidate, prime, NULL);
    return count;
}

static void reconstruct_crt(mpz_t result, const uint64_t *residues,
                            const uint64_t *moduli, size_t count)
{
    mpz_t product;
    mpz_init_set_ui(product, 1);
    mpz_set_ui(result, 0);
    for (size_t index = 0; index < count; ++index) {
        uint64_t p = moduli[index];
        uint64_t product_mod =
            (uint64_t)mpz_fdiv_ui(product, (unsigned long)p);
        uint64_t inverse = power_mod(product_mod, p - 2, p);
        uint64_t result_mod =
            (uint64_t)mpz_fdiv_ui(result, (unsigned long)p);
        uint64_t difference = subtract_mod(residues[index], result_mod, p);
        uint64_t multiplier = multiply_mod(difference, inverse, p);
        mpz_addmul_ui(result, product, (unsigned long)multiplier);
        mpz_mul_ui(product, product, (unsigned long)p);
    }
    for (size_t index = 0; index < count; ++index) {
        if ((uint64_t)mpz_fdiv_ui(result,
                                  (unsigned long)moduli[index]) !=
            residues[index]) {
            mpz_clear(product);
            die("CRT residue replay check failed");
        }
    }
    mpz_clear(product);
}

static void rook_exact(mpz_t result, int n, uint64_t memory_budget,
                       bool report)
{
    double start = monotonic_seconds();
    unsigned bits = n <= DIRECT_MAX_N
                        ? (n >= 2 ? (unsigned)(n - 2) : 0U)
                        : BASE_MASK_BITS;
    uint64_t state_count = UINT64_C(1) << bits;
    uint64_t workspace_bytes = state_count * sizeof(uint32_t);
    if (workspace_bytes > memory_budget) {
        fprintf(stderr,
                "error: A263987 n=%d needs %.3f GiB per CRT worker; "
                "configured limit is %.3f GiB\n",
                n, (double)workspace_bytes / (1024.0 * 1024.0 * 1024.0),
                (double)memory_budget / (1024.0 * 1024.0 * 1024.0));
        exit(EXIT_FAILURE);
    }

    mpz_t bound;
    mpz_t modulus_product;
    mpz_inits(bound, modulus_product, NULL);
    mpz_fac_ui(bound, (unsigned long)n);
    uint64_t moduli[MAX_MODULUS_COUNT] = { 0 };
    uint64_t residues[MAX_MODULUS_COUNT] = { 0 };
    DpStats stats[MAX_MODULUS_COUNT];
    bool status[MAX_MODULUS_COUNT] = { false };
    size_t modulus_count = choose_moduli(moduli, modulus_product, bound);

    int workers = 1;
#ifdef _OPENMP
    uint64_t reserve =
        PARALLEL_RESERVE_MIB * UINT64_C(1024) * UINT64_C(1024);
    uint64_t parallel_budget =
        memory_budget > reserve ? memory_budget - reserve : memory_budget;
    uint64_t memory_workers = parallel_budget / workspace_bytes;
    workers = omp_get_max_threads();
    if (workers > (int)modulus_count) {
        workers = (int)modulus_count;
    }
    if ((uint64_t)workers > memory_workers) {
        workers = (int)memory_workers;
    }
    if (workspace_bytes >= (UINT64_C(512) << 20)) {
        workers = 1;
    } else if (workspace_bytes >= (UINT64_C(256) << 20) && workers > 2) {
        workers = 2;
    }
    if (workers < 1) {
        workers = 1;
    }
#endif

    if (report) {
        fprintf(stderr,
                "263987_02: n=%d, used-row mask=%u bits "
                "(%" PRIu64 " states), folded low columns=%d, "
                "bound=%zu bits, %zu CRT pass%s, %d worker%s, "
                "memory limit=%.2f GiB\n",
                n, bits, state_count,
                n > DIRECT_MAX_N ? n - DIRECT_MAX_N : 0,
                mpz_sizeinbase(bound, 2), modulus_count,
                modulus_count == 1 ? "" : "es", workers,
                workers == 1 ? "" : "s",
                (double)memory_budget / (1024.0 * 1024.0 * 1024.0));
    }

#ifdef _OPENMP
    if (workers > 1) {
#pragma omp parallel for num_threads(workers) schedule(static)
        for (long pass = 0; pass < (long)modulus_count; ++pass) {
            status[pass] = transposed_residue(
                &residues[pass], n, moduli[pass], &stats[pass]);
        }
    } else
#endif
    {
        for (long pass = 0; pass < (long)modulus_count; ++pass) {
            status[pass] = transposed_residue(
                &residues[pass], n, moduli[pass], &stats[pass]);
        }
    }
    for (size_t pass = 0; pass < modulus_count; ++pass) {
        if (!status[pass]) {
            mpz_clears(bound, modulus_product, NULL);
            die("could not allocate transposed-DP workspace");
        }
        if (report) {
            fprintf(stderr,
                    "263987_02: n=%d pass %zu/%zu ok, p=%" PRIu64
                    ", transitions=%" PRIu64 ", %.3f s\n",
                    n, pass + 1, modulus_count, moduli[pass],
                    stats[pass].transitions, stats[pass].seconds);
        }
    }

    reconstruct_crt(result, residues, moduli, modulus_count);
    if (mpz_cmp(result, bound) > 0) {
        mpz_clears(bound, modulus_product, NULL);
        die("reconstructed A263987 value exceeds n! bound");
    }
    if (report) {
        fprintf(stderr,
                "263987_02: n=%d done, workspace/worker=%.3f GiB, "
                "total %.3f s\n",
                n, (double)workspace_bytes /
                    (1024.0 * 1024.0 * 1024.0),
                monotonic_seconds() - start);
    }
    mpz_clears(bound, modulus_product, NULL);
}

static void count_exact(mpz_t result, int n, uint64_t memory_budget,
                        bool report)
{
    if (is_prime_int(n)) {
        mpz_t previous;
        mpz_init(previous);
        rook_exact(previous, n - 1, memory_budget, report);
        mpz_mul_ui(result, previous, 2);
        mpz_clear(previous);
        if (report) {
            fprintf(stderr,
                    "263987_02: n=%d is prime; used a(n)=2*a(n-1)\n",
                    n);
        }
        return;
    }
    rook_exact(result, n, memory_budget, report);
}

static void verify_known(const mpz_t value, int n)
{
    if (n < SEQUENCE_OFFSET || n > KNOWN_MAX_N) {
        return;
    }
    mpz_t expected;
    mpz_init(expected);
    if (mpz_set_str(expected, builtin_terms[n], 10) != 0) {
        mpz_clear(expected);
        die("invalid built-in A263987 term");
    }
    if (mpz_cmp(value, expected) != 0) {
        gmp_fprintf(stderr,
                    "error: A263987 mismatch at n=%d: got %Zd, "
                    "expected %Zd\n", n, value, expected);
        mpz_clear(expected);
        exit(EXIT_FAILURE);
    }
    mpz_clear(expected);
}

static void compute_checked(mpz_t value, int n, uint64_t memory_budget,
                            bool report)
{
    count_exact(value, n, memory_budget, report);
    verify_known(value, n);
}

static int check_known_terms(int max_n, uint64_t memory_budget)
{
    if (max_n > KNOWN_MAX_N) {
        fprintf(stderr, "error: --check has known terms only through n=%d\n",
                KNOWN_MAX_N);
        return EXIT_FAILURE;
    }
    mpz_t value;
    mpz_init(value);
    for (int n = SEQUENCE_OFFSET; n <= max_n; ++n) {
        compute_checked(value, n, memory_budget, false);
    }
    mpz_clear(value);
    printf("ok: transposed rook DP agrees with A263987 for n=%d..%d\n",
           SEQUENCE_OFFSET, max_n);
    return EXIT_SUCCESS;
}

static void write_known(FILE *stream, int n)
{
    if (n < SEQUENCE_OFFSET || n > BUILTIN_MAX_N) {
        die("requested built-in A263987 term is unavailable");
    }
    if (fprintf(stream, "%d %s\n", n, builtin_terms[n]) < 0) {
        die("could not write a built-in A263987 term");
    }
}

static void produce_b_file(const char *argv0, int max_n, int start_n,
                           uint64_t memory_budget)
{
    if (start_n < SEQUENCE_OFFSET || start_n > max_n + 1 ||
        start_n > BUILTIN_MAX_N + 1) {
        fprintf(stderr,
                "error: START_N must be in %d..min(MAX_N+1,%d)\n",
                SEQUENCE_OFFSET, BUILTIN_MAX_N + 1);
        exit(EXIT_FAILURE);
    }
    char *path = path_beside_executable(argv0, "b263987_1.txt");
    char *part = path_beside_executable(argv0, "b263987_1_part.txt");
    FILE *stream = fopen(part, "w");
    if (stream == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part, strerror(errno));
        free(part);
        free(path);
        exit(EXIT_FAILURE);
    }
    for (int n = SEQUENCE_OFFSET; n < start_n && n <= max_n; ++n) {
        write_known(stream, n);
    }
    if (fflush(stream) != 0) {
        fclose(stream);
        free(part);
        free(path);
        die("could not flush the built-in A263987 prefix");
    }
    if (start_n > SEQUENCE_OFFSET) {
        fprintf(stderr,
                "263987_02: using built-in verified prefix n=%d..%d\n",
                SEQUENCE_OFFSET, start_n - 1);
    }

    mpz_t value;
    mpz_t previous;
    mpz_inits(value, previous, NULL);
    bool have_previous = false;
    if (start_n > SEQUENCE_OFFSET) {
        if (mpz_set_str(previous, builtin_terms[start_n - 1], 10) != 0) {
            mpz_clears(value, previous, NULL);
            fclose(stream);
            free(part);
            free(path);
            die("invalid built-in A263987 prefix term");
        }
        have_previous = true;
    }
    for (int n = start_n; n <= max_n; ++n) {
        if (is_prime_int(n) && have_previous) {
            mpz_mul_ui(value, previous, 2);
            verify_known(value, n);
            fprintf(stderr,
                    "263987_02: n=%d is prime; used cached "
                    "a(n)=2*a(n-1)\n", n);
        } else {
            compute_checked(value, n, memory_budget, true);
        }
        if (gmp_fprintf(stream, "%d %Zd\n", n, value) < 0 ||
            fflush(stream) != 0) {
            mpz_clears(value, previous, NULL);
            fclose(stream);
            free(part);
            free(path);
            die("could not write the A263987 verification b-file");
        }
        mpz_set(previous, value);
        have_previous = true;
    }
    mpz_clears(value, previous, NULL);
    if (fclose(stream) != 0) {
        free(part);
        free(path);
        die("could not close the A263987 verification b-file");
    }
    if (rename(part, path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part, path, strerror(errno));
        free(part);
        free(path);
        exit(EXIT_FAILURE);
    }
    printf("wrote %s (n=%d..%d)\n", path, SEQUENCE_OFFSET, max_n);
    free(part);
    free(path);
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N [START_N]]\n"
            "       %s --term N\n"
            "       %s --check [MAX_N]\n\n"
            "MAX_N defaults to %d and may be at most %d.\n"
            "START_N defaults to %d; the built-in prefix ends at n=%d.\n"
            "The default memory limit is %" PRIu64 " MiB; override it with\n"
            "A263987_MEMORY_MIB.\n",
            program, program, program, DEFAULT_MAX_N, MAX_SUPPORTED_N,
            SEQUENCE_OFFSET, BUILTIN_MAX_N, DEFAULT_MEMORY_MIB);
}

int main(int argc, char **argv)
{
    uint64_t memory_budget = memory_budget_bytes();
    if (argc >= 2 && strcmp(argv[1], "--term") == 0) {
        if (argc != 3) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        int n = parse_n(argv[2], "N");
        mpz_t value;
        mpz_init(value);
        compute_checked(value, n, memory_budget, true);
        gmp_printf("%d %Zd\n", n, value);
        mpz_clear(value);
        return EXIT_SUCCESS;
    }
    if (argc >= 2 && strcmp(argv[1], "--check") == 0) {
        if (argc > 3) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        int max_n = argc == 3 ? parse_n(argv[2], "MAX_N")
                              : DEFAULT_CHECK_N;
        return check_known_terms(max_n, memory_budget);
    }
    if (argc > 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    int max_n = argc >= 2 ? parse_n(argv[1], "MAX_N")
                          : DEFAULT_MAX_N;
    int start_n = argc == 3 ? parse_n(argv[2], "START_N")
                            : SEQUENCE_OFFSET;
    produce_b_file(argv[0], max_n, start_n, memory_budget);
    return EXIT_SUCCESS;
}
