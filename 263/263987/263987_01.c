/*
 * A263987 -- exact forbidden-adjacency rook DP.
 *
 * A permutation is invalid exactly when it contains an adjacent pair x,y
 * with x > y and y not dividing x.  Put one forbidden cell (x,y) for every
 * such pair.  Inclusion-exclusion over selected forbidden adjacencies gives
 *
 *     a(n) = sum_k (-1)^k r_k (n-k)!,
 *
 * where r_k is the k-th rook number of the forbidden board.  Indeed, a
 * nonattacking selection has indegree and outdegree at most one, and every
 * selected edge strictly decreases.  It is therefore a disjoint union of
 * directed paths, never a cycle; contracting k selected adjacencies leaves
 * n-k freely orderable blocks.
 *
 * Rows 1,2 and columns 1,n are empty, so only n-2 column bits are needed.
 * Rows are processed in increasing order by an in-place descending-mask DP.
 * For n=33..38, the base is fixed at rows 3..32 and columns 2..31; the last
 * one to six rows are folded into the final sum by a small exact matching DP.
 * Thus the main array never exceeds 30 bits, or 4 GiB per CRT pass.
 * Every modular state is uint32_t.  CRT uses primes above 2^30 and reconstructs
 * only after their product exceeds the rigorous bound n!.  The reconstructed
 * value is checked against the bound and replayed modulo every prime.
 *
 * For a prime p, a(p)=2*a(p-1): p can only be inserted at the end or directly
 * before 1.  The implementation uses this exact shortcut.
 *
 * Build on the configured Apple Silicon Mac:
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -Xpreprocessor -fopenmp \
 *     -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     263987_01.c -lomp -lgmp -o 263987_01
 *
 * Normal output is b263987.txt beside the executable.  b263987_part.txt is
 * flushed after every completed term and atomically renamed after success.
 * A263987 has OFFSET 0.
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
#error "263987_01 requires a platform with 64-bit unsigned long"
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
#define MAX_MODULUS_COUNT 5
#ifndef DIRECT_MAX_N
#define DIRECT_MAX_N 32
#endif
#define BASE_MASK_BITS (DIRECT_MAX_N - 2)
#define MAX_TERMINAL_ROWS 6

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
        if ((exponent & 1) != 0) {
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

static uint32_t forbidden_mask_for_row(int row)
{
    uint32_t mask = 0;
    for (int column = 2; column < row; ++column) {
        if (row % column != 0) {
            mask |= UINT32_C(1) << (unsigned)(column - 2);
        }
    }
    return mask;
}

static bool direct_rook_residue(uint64_t *output, int n,
                                uint64_t modulus, DpStats *stats)
{
    memset(stats, 0, sizeof(*stats));
    if (modulus == 0 || modulus > UINT32_MAX) {
        die("rook-DP modulus is outside the uint32_t range");
    }
    double start = monotonic_seconds();
    unsigned bits = n <= DIRECT_MAX_N
                        ? (n >= 2 ? (unsigned)(n - 2) : 0U)
                        : BASE_MASK_BITS;
    uint32_t state_count = UINT32_C(1) << bits;
    uint32_t small_modulus = (uint32_t)modulus;
    uint32_t *dp = calloc((size_t)state_count, sizeof(*dp));
    if (dp == NULL) {
        return false;
    }
    dp[0] = 1;

    uint32_t old_limit = 1;
    for (int row = 3; row <= n; ++row) {
        uint32_t forbidden = forbidden_mask_for_row(row);
        uint32_t source = old_limit;
        while (source != 0) {
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
                dp[target] =
                    sum >= small_modulus ? sum - small_modulus : sum;
                ++stats->transitions;
                available ^= bit;
            }
        }
        old_limit <<= 1;
    }
    if (old_limit != state_count) {
        free(dp);
        die("rook-DP active range invariant failed");
    }

    uint32_t factorial[MAX_SUPPORTED_N + 1];
    factorial[0] = 1;
    for (int value = 1; value <= n; ++value) {
        factorial[value] = (uint32_t)(
            ((uint64_t)factorial[value - 1] * (unsigned)value) %
            small_modulus);
    }
    uint32_t result = 0;
    for (uint32_t mask = 0; mask < state_count; ++mask) {
        unsigned rooks = (unsigned)__builtin_popcount(mask);
        uint32_t term = (uint32_t)(
            ((uint64_t)dp[mask] * factorial[n - (int)rooks]) %
            small_modulus);
        if ((rooks & 1U) == 0) {
            uint32_t sum = result + term;
            result = sum >= small_modulus ? sum - small_modulus : sum;
        } else {
            result = result >= term
                         ? result - term
                         : small_modulus - (term - result);
        }
    }
    *output = result;
    stats->seconds = monotonic_seconds() - start;
    free(dp);
    return true;
}

/*
 * For n > 32, keep the same 30-bit base board (rows 3..32 and columns
 * 2..31), then fold rows 33..n into the final inclusion-exclusion sum.
 * For a fixed base mask U, the terminal rook numbers are matchings of at
 * most six labeled rows into the still available columns.
 *
 * If d(S) is the number of columns forbidden in every row of nonempty S,
 * the number inj(R) of injective assignments for exactly the rows R is
 *
 *   inj(R) = sum_{B subset R, pivot in B}
 *              (-1)^(|B|-1) (|B|-1)! d(B) inj(R\B).
 *
 * This is the Möbius formula on set partitions, evaluated with exact int64
 * arithmetic.  It avoids allocating any state bit for a terminal column.
 */
static bool folded_rook_residue(uint64_t *output, int n,
                                uint64_t modulus, DpStats *stats)
{
    memset(stats, 0, sizeof(*stats));
    if (modulus == 0 || modulus > UINT32_MAX) {
        die("folded rook-DP modulus is outside the uint32_t range");
    }
    int terminal_rows = n - DIRECT_MAX_N;
    if (terminal_rows < 1 || terminal_rows > MAX_TERMINAL_ROWS) {
        die("folded rook-DP terminal-row count is out of range");
    }
    double start = monotonic_seconds();
    uint32_t state_count = UINT32_C(1) << BASE_MASK_BITS;
    uint32_t small_modulus = (uint32_t)modulus;
    uint32_t *dp = calloc((size_t)state_count, sizeof(*dp));
    if (dp == NULL) {
        return false;
    }
    dp[0] = 1;

    uint32_t old_limit = 1;
    for (int row = 3; row <= DIRECT_MAX_N; ++row) {
        uint32_t forbidden = forbidden_mask_for_row(row);
        uint32_t source = old_limit;
        while (source != 0) {
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
                dp[target] =
                    sum >= small_modulus ? sum - small_modulus : sum;
                ++stats->transitions;
                available ^= bit;
            }
        }
        old_limit <<= 1;
    }
    if (old_limit != state_count) {
        free(dp);
        die("folded rook-DP base range invariant failed");
    }

    uint32_t base_row_masks[MAX_TERMINAL_ROWS] = { 0 };
    uint32_t late_row_masks[MAX_TERMINAL_ROWS] = { 0 };
    for (int index = 0; index < terminal_rows; ++index) {
        int row = DIRECT_MAX_N + 1 + index;
        for (int column = 2; column < DIRECT_MAX_N; ++column) {
            if (row % column != 0) {
                base_row_masks[index] |=
                    UINT32_C(1) << (unsigned)(column - 2);
            }
        }
        for (int column = DIRECT_MAX_N; column < n; ++column) {
            if (column < row && row % column != 0) {
                late_row_masks[index] |=
                    UINT32_C(1) <<
                        (unsigned)(column - DIRECT_MAX_N);
            }
        }
    }

    unsigned terminal_state_count = 1U << (unsigned)terminal_rows;
    uint32_t base_intersection[1U << MAX_TERMINAL_ROWS] = { 0 };
    uint8_t late_intersection_count[1U << MAX_TERMINAL_ROWS] = { 0 };
    uint8_t subset_size[1U << MAX_TERMINAL_ROWS] = { 0 };
    int64_t block_factorial[1U << MAX_TERMINAL_ROWS] = { 0 };
    uint32_t all_base_columns =
        (UINT32_C(1) << BASE_MASK_BITS) - UINT32_C(1);
    uint32_t all_late_columns =
        (UINT32_C(1) << (unsigned)terminal_rows) - UINT32_C(1);
    for (unsigned subset = 1; subset < terminal_state_count; ++subset) {
        unsigned bit = subset & (0U - subset);
        unsigned index = (unsigned)__builtin_ctz(bit);
        unsigned rest = subset ^ bit;
        uint32_t base = rest == 0
                            ? all_base_columns
                            : base_intersection[rest];
        uint32_t late = rest == 0
                            ? all_late_columns
                            : (uint32_t)late_intersection_count[rest];
        base_intersection[subset] = base & base_row_masks[index];
        late &= late_row_masks[index];
        /*
         * terminal_rows <= 6, so storing the late intersection itself in
         * uint8_t is safe; convert it to a count only after all intersections.
         */
        late_intersection_count[subset] = (uint8_t)late;
        subset_size[subset] = (uint8_t)__builtin_popcount(subset);
        int64_t factorial = 1;
        for (unsigned factor = 2; factor < subset_size[subset]; ++factor) {
            factorial *= (int64_t)factor;
        }
        block_factorial[subset] = factorial;
    }
    for (unsigned subset = 1; subset < terminal_state_count; ++subset) {
        late_intersection_count[subset] =
            (uint8_t)__builtin_popcount(
                (unsigned)late_intersection_count[subset]);
    }

    uint32_t factorial[MAX_SUPPORTED_N + 1];
    factorial[0] = 1;
    for (int value = 1; value <= n; ++value) {
        factorial[value] = (uint32_t)(
            ((uint64_t)factorial[value - 1] * (unsigned)value) %
            small_modulus);
    }

    int evaluation_threads = 1;
#ifdef _OPENMP
    evaluation_threads = omp_get_max_threads();
#endif
    uint64_t *partial = calloc(
        (size_t)evaluation_threads, sizeof(*partial));
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
        for (uint64_t state_index = begin;
             state_index < end; ++state_index) {
            uint32_t mask = (uint32_t)state_index;
            uint32_t base_value = dp[mask];
            if (base_value == 0) {
                continue;
            }
            unsigned base_rooks =
                (unsigned)__builtin_popcount(mask);
            uint8_t degree[1U << MAX_TERMINAL_ROWS] = { 0 };
            int64_t injective[1U << MAX_TERMINAL_ROWS] = { 0 };
            uint64_t rook_number[MAX_TERMINAL_ROWS + 1] = { 0 };
            injective[0] = 1;
            rook_number[0] = 1;

            for (unsigned subset = 1;
                 subset < terminal_state_count; ++subset) {
                degree[subset] = (uint8_t)(
                    __builtin_popcount(
                        base_intersection[subset] & ~mask) +
                    late_intersection_count[subset]);
            }
            for (unsigned rows = 1;
                 rows < terminal_state_count; ++rows) {
                unsigned pivot = rows & (0U - rows);
                int64_t count = 0;
                for (unsigned block = rows; block != 0;
                     block = (block - 1U) & rows) {
                    if ((block & pivot) == 0) {
                        continue;
                    }
                    int64_t term =
                        block_factorial[block] *
                        (int64_t)degree[block] *
                        injective[rows ^ block];
                    if ((subset_size[block] & 1U) != 0) {
                        count += term;
                    } else {
                        count -= term;
                    }
                }
                if (count < 0) {
                    die("negative folded terminal matching count");
                }
                injective[rows] = count;
                rook_number[subset_size[rows]] += (uint64_t)count;
            }

            for (int added = 0; added <= terminal_rows; ++added) {
                int remaining =
                    n - (int)base_rooks - added;
                uint64_t term = multiply_mod(
                    base_value,
                    rook_number[added] % modulus, modulus);
                term = multiply_mod(
                    term, factorial[remaining], modulus);
                if (((base_rooks + (unsigned)added) & 1U) == 0) {
                    local += term;
                    if (local >= modulus) {
                        local -= modulus;
                    }
                } else {
                    local = local >= term
                                ? local - term
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

static bool rook_residue(uint64_t *output, int n, uint64_t modulus,
                         DpStats *stats)
{
    if (n <= DIRECT_MAX_N) {
        return direct_rook_residue(output, n, modulus, stats);
    }
    return folded_rook_residue(output, n, modulus, stats);
}

static size_t choose_moduli(uint64_t *moduli, mpz_t product,
                            const mpz_t bound)
{
    mpz_t candidate;
    mpz_t prime;
    mpz_inits(candidate, prime, NULL);
    mpz_set_ui(candidate, 1);
    mpz_mul_2exp(candidate, candidate, 30);
    mpz_set_ui(product, 1);
    size_t count = 0;
    while (mpz_cmp(product, bound) <= 0) {
        if (count >= MAX_MODULUS_COUNT) {
            mpz_clears(candidate, prime, NULL);
            die("too many CRT moduli for n! bound");
        }
        mpz_nextprime(prime, candidate);
        moduli[count++] = (uint64_t)mpz_get_ui(prime);
        mpz_mul_ui(product, product,
                   (unsigned long)moduli[count - 1]);
        mpz_set(candidate, prime);
    }
    mpz_clears(candidate, prime, NULL);
    return count;
}

static void reconstruct_crt(mpz_t result, const uint64_t *residues,
                            const uint64_t *moduli, size_t modulus_count)
{
    mpz_t product;
    mpz_init_set_ui(product, 1);
    mpz_set_ui(result, 0);
    for (size_t index = 0; index < modulus_count; ++index) {
        uint64_t modulus = moduli[index];
        uint64_t product_mod =
            (uint64_t)mpz_fdiv_ui(product, (unsigned long)modulus);
        uint64_t inverse = power_mod(product_mod, modulus - 2, modulus);
        uint64_t result_mod =
            (uint64_t)mpz_fdiv_ui(result, (unsigned long)modulus);
        uint64_t difference = subtract_mod(
            residues[index], result_mod, modulus);
        uint64_t multiplier = multiply_mod(difference, inverse, modulus);
        mpz_addmul_ui(result, product, (unsigned long)multiplier);
        mpz_mul_ui(product, product, (unsigned long)modulus);
    }
    for (size_t index = 0; index < modulus_count; ++index) {
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
                n,
                (double)workspace_bytes /
                    (1024.0 * 1024.0 * 1024.0),
                (double)memory_budget /
                    (1024.0 * 1024.0 * 1024.0));
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
    } else if (workspace_bytes >= (UINT64_C(256) << 20) &&
               workers > 2) {
        workers = 2;
    }
    if (workers < 1) {
        workers = 1;
    }
#endif

    if (report) {
        fprintf(stderr,
                "263987_01: n=%d, forbidden-mask=%u bits "
                "(%" PRIu64 " states), folded terminal rows=%d, "
                "bound=%zu bits, %zu CRT pass%s, "
                "%d worker%s, memory limit=%.2f GiB\n",
                n, bits, state_count,
                n > DIRECT_MAX_N ? n - DIRECT_MAX_N : 0,
                mpz_sizeinbase(bound, 2),
                modulus_count, modulus_count == 1 ? "" : "es",
                workers, workers == 1 ? "" : "s",
                (double)memory_budget /
                    (1024.0 * 1024.0 * 1024.0));
    }

#ifdef _OPENMP
    if (workers > 1) {
#pragma omp parallel for num_threads(workers) schedule(static)
        for (long pass = 0; pass < (long)modulus_count; ++pass) {
            status[pass] = rook_residue(
                &residues[pass], n, moduli[pass], &stats[pass]);
        }
    } else
#endif
    {
        for (long pass = 0; pass < (long)modulus_count; ++pass) {
            status[pass] = rook_residue(
                &residues[pass], n, moduli[pass], &stats[pass]);
        }
    }
    for (size_t pass = 0; pass < modulus_count; ++pass) {
        if (!status[pass]) {
            mpz_clears(bound, modulus_product, NULL);
            die("could not allocate rook-DP workspace");
        }
        if (report) {
            fprintf(stderr,
                    "263987_01: n=%d pass %zu/%zu ok, p=%" PRIu64
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
                "263987_01: n=%d done, workspace/worker=%.3f GiB, "
                "total %.3f s\n",
                n,
                (double)workspace_bytes /
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
                    "263987_01: n=%d is prime; used a(n)=2*a(n-1)\n",
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
                    "expected %Zd\n",
                    n, value, expected);
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
    printf("ok: forbidden-adjacency rook DP agrees with A263987 "
           "for n=%d..%d\n",
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
    char *path = path_beside_executable(argv0, "b263987.txt");
    char *part = path_beside_executable(argv0, "b263987_part.txt");
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
                "263987_01: using built-in verified prefix n=%d..%d\n",
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
                    "263987_01: n=%d is prime; used cached "
                    "a(n)=2*a(n-1)\n",
                    n);
        } else {
            compute_checked(value, n, memory_budget, true);
        }
        if (gmp_fprintf(stream, "%d %Zd\n", n, value) < 0 ||
            fflush(stream) != 0) {
            mpz_clears(value, previous, NULL);
            fclose(stream);
            free(part);
            free(path);
            die("could not write the A263987 b-file");
        }
        mpz_set(previous, value);
        have_previous = true;
    }
    mpz_clears(value, previous, NULL);
    if (fclose(stream) != 0) {
        free(part);
        free(path);
        die("could not close the A263987 b-file");
    }
    if (rename(part, path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part, path, strerror(errno));
        free(part);
        free(path);
        exit(EXIT_FAILURE);
    }
    printf("wrote %s (n=%d..%d)\n",
           path, SEQUENCE_OFFSET, max_n);
    free(part);
    free(path);
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N [START_N]]\n"
            "       %s --term N\n"
            "       %s --check [MAX_N]\n"
            "\n"
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
        int max_n = argc == 3
                        ? parse_n(argv[2], "MAX_N")
                        : DEFAULT_CHECK_N;
        return check_known_terms(max_n, memory_budget);
    }
    if (argc > 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    int max_n = argc >= 2
                    ? parse_n(argv[1], "MAX_N")
                    : DEFAULT_MAX_N;
    int start_n = argc == 3
                      ? parse_n(argv[2], "START_N")
                      : SEQUENCE_OFFSET;
    produce_b_file(argv[0], max_n, start_n, memory_budget);
    return EXIT_SUCCESS;
}
