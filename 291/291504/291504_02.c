/*
 * A291504 -- low-memory modular layer DP with exact CRT recovery.
 *
 * Count permutations s_1,...,s_n of [n] for which every prefix sum is
 * nonprime.  For S subseteq [n], let F(S) count valid orders of S.  Then
 *
 *   F(empty) = 1,
 *   F(S) = 0                              if sum(S) is prime,
 *   F(S) = Sum_{x in S} F(S \ {x})       otherwise.
 *
 * As in 291504_01.c, fixed-cardinality subsets are enumerated in colex
 * order and only adjacent layers are retained.  The important difference is
 * that this program evaluates the recurrence modulo several small primes,
 * one pass at a time, and reconstructs the unique exact answer by CRT.
 *
 * For n<=33 each state is one uint16_t residue.  For n=34 each state is one
 * uint8_t residue.  The product of the selected pairwise-distinct primes is
 * required to exceed n!, while every DP value is between 0 and n!.  This
 * proves that the CRT reconstruction is unique.  A checked 256-bit integer
 * holds the answer, the running CRT product, and the factorial bound.
 *
 * Peak DP storage is approximately:
 *
 *   n=31: 1.12 GiB,  n=32: 2.17 GiB,
 *   n=33: 4.35 GiB,  n=34: 4.23 GiB.
 *
 * The time remains exponential.  Terms 31 and 32 are the principal intended
 * extension beyond program 01; 33 and especially 34 are long computations.
 * Exact n=45 would still require trillions of central-layer states and is not
 * claimed by this implementation.  The enforced maximum is therefore 34.
 *
 * Known terms a(0)..a(23) are built in.  Every newly completed term is saved
 * atomically in b291504_02.txt, so a later run can reuse it.
 *
 * Layers are divided into contiguous colex-rank ranges and evaluated in
 * parallel.  A291504_THREADS controls the worker count; the default is the
 * number of online processors, capped at 8.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       291504_02.c -o 291504_02
 *
 * Examples:
 *   ./291504_02 --term 31
 *   ./291504_02 --upto 32
 *   ./291504_02 --check
 *   A291504_MEMORY_MIB=5000 ./291504_02 --term 34
 */

#define _POSIX_C_SOURCE 200809L
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if !defined(__SIZEOF_INT128__)
#error "291504_02.c requires unsigned __int128"
#endif

#if SIZE_MAX < UINT64_MAX
#error "291504_02.c requires a 64-bit size_t"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 34U
#define DEFAULT_MAX_N 31U
#define KNOWN_MAX_N 23U
#define DIRECT_CHECK_MAX_N 10U
#define MAX_PREFIX_SUM (MAX_N * (MAX_N + 1U) / 2U)
#define MAX_MODULI 24U
#define DEFAULT_MEMORY_MIB UINT64_C(5120)
#define MIN_MEMORY_MIB UINT64_C(64)
#define MAX_MEMORY_MIB UINT64_C(65536)
#define REPORT_PARTS 8U
#define REPORT_MIN_STATES UINT64_C(134217728)
#define MAX_THREADS 64U
#define BIG_LIMBS 4U
#define BIG_TEXT_SIZE 80U
#define BFILE_PATH "b291504_02.txt"
#define LOCK_PATH "b291504_02.txt.lock"

typedef struct {
    uint64_t limb[BIG_LIMBS];
} U256;

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} Mode;

static uint64_t binomial[MAX_N + 1U][MAX_N + 1U];
static bool prime_sum[MAX_PREFIX_SUM + 1U];
static bool tables_ready;
static uint64_t memory_limit;
static unsigned configured_threads;
static bool quiet;

static const char *const known_terms[KNOWN_MAX_N + 1U] = {
    "1",
    "1",
    "0",
    "1",
    "3",
    "8",
    "48",
    "206",
    "1838",
    "13336",
    "133764",
    "1081556",
    "11046816",
    "108196128",
    "1555323224",
    "16279258144",
    "289771660328",
    "3495882548784",
    "66923393467216",
    "942785369844048",
    "15625264115770992",
    "315553823251866304",
    "5974132307015712032",
    "104979988889030774848"
};

static _Noreturn void die(const char *message)
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

static unsigned parse_unsigned(const char *text, unsigned maximum,
                               const char *label)
{
    char *end = NULL;
    errno = 0;
    const unsigned long value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value > maximum) {
        fprintf(stderr, "error: %s must be in 0..%u: %s\n",
                label, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (unsigned)value;
}

static void configure_memory_limit(void)
{
    uint64_t mib = DEFAULT_MEMORY_MIB;
    const char *text = getenv("A291504_MEMORY_MIB");
    if (text != NULL && *text != '\0') {
        char *end = NULL;
        errno = 0;
        const unsigned long long parsed = strtoull(text, &end, 10);
        if (errno != 0 || end == text || *end != '\0' ||
            parsed < MIN_MEMORY_MIB || parsed > MAX_MEMORY_MIB) {
            fprintf(stderr,
                    "error: A291504_MEMORY_MIB must be in %" PRIu64
                    "..%" PRIu64 ": %s\n",
                    MIN_MEMORY_MIB, MAX_MEMORY_MIB, text);
            exit(EXIT_FAILURE);
        }
        mib = (uint64_t)parsed;
    }
    memory_limit = mib << 20U;
}

static void configure_threads(void)
{
    long online = sysconf(_SC_NPROCESSORS_ONLN);
    unsigned threads = online > 0 ? (unsigned)online : 1U;
    if (threads > 8U) {
        threads = 8U;
    }

    const char *text = getenv("A291504_THREADS");
    if (text != NULL && *text != '\0') {
        threads = parse_unsigned(text, MAX_THREADS, "A291504_THREADS");
        if (threads == 0U) {
            die("A291504_THREADS must be at least 1");
        }
    }
    configured_threads = threads;
}

static bool big_is_zero(const U256 *value)
{
    uint64_t combined = 0U;
    for (unsigned i = 0U; i < BIG_LIMBS; ++i) {
        combined |= value->limb[i];
    }
    return combined == 0U;
}

static int big_compare(const U256 *left, const U256 *right)
{
    for (unsigned i = BIG_LIMBS; i-- > 0U;) {
        if (left->limb[i] < right->limb[i]) {
            return -1;
        }
        if (left->limb[i] > right->limb[i]) {
            return 1;
        }
    }
    return 0;
}

static void big_multiply_small(U256 *value, uint32_t factor)
{
    U128 carry = 0U;
    for (unsigned i = 0U; i < BIG_LIMBS; ++i) {
        const U128 product = (U128)value->limb[i] * factor + carry;
        value->limb[i] = (uint64_t)product;
        carry = product >> 64U;
    }
    if (carry != 0U) {
        die("internal 256-bit multiplication overflow");
    }
}

static void big_add_product(U256 *target, const U256 *value,
                            uint32_t factor)
{
    U128 carry = 0U;
    for (unsigned i = 0U; i < BIG_LIMBS; ++i) {
        const U128 sum = (U128)value->limb[i] * factor +
                         target->limb[i] + carry;
        target->limb[i] = (uint64_t)sum;
        carry = sum >> 64U;
    }
    if (carry != 0U) {
        die("internal 256-bit addition overflow");
    }
}

static uint32_t big_modulo_small(const U256 *value, uint32_t modulus)
{
    uint64_t remainder = 0U;
    for (unsigned i = BIG_LIMBS; i-- > 0U;) {
        const U128 dividend = ((U128)remainder << 64U) | value->limb[i];
        remainder = (uint64_t)(dividend % modulus);
    }
    return (uint32_t)remainder;
}

static uint32_t big_divide_small(U256 *value, uint32_t divisor)
{
    uint64_t remainder = 0U;
    for (unsigned i = BIG_LIMBS; i-- > 0U;) {
        const U128 dividend = ((U128)remainder << 64U) | value->limb[i];
        value->limb[i] = (uint64_t)(dividend / divisor);
        remainder = (uint64_t)(dividend % divisor);
    }
    return (uint32_t)remainder;
}

static U256 big_factorial(unsigned n)
{
    U256 result = {{1U, 0U, 0U, 0U}};
    for (unsigned factor = 2U; factor <= n; ++factor) {
        big_multiply_small(&result, factor);
    }
    return result;
}

static bool parse_u256(const char *text, U256 *result)
{
    U256 value = {{0U, 0U, 0U, 0U}};
    if (*text == '\0') {
        return false;
    }
    while (*text != '\0') {
        if (*text < '0' || *text > '9') {
            return false;
        }
        const unsigned digit = (unsigned)(*text++ - '0');
        U256 before = value;
        big_multiply_small(&value, 10U);
        big_add_product(&value, &(U256){{1U, 0U, 0U, 0U}}, digit);
        if (big_compare(&value, &before) < 0) {
            return false;
        }
    }
    *result = value;
    return true;
}

static void u256_to_text(const U256 *value, char text[BIG_TEXT_SIZE])
{
    uint32_t chunks[10];
    size_t count = 0U;
    U256 copy = *value;
    do {
        if (count == sizeof(chunks) / sizeof(chunks[0])) {
            die("internal decimal conversion overflow");
        }
        chunks[count++] = big_divide_small(&copy, UINT32_C(1000000000));
    } while (!big_is_zero(&copy));

    int written = snprintf(text, BIG_TEXT_SIZE, "%" PRIu32,
                           chunks[count - 1U]);
    if (written < 0 || (size_t)written >= BIG_TEXT_SIZE) {
        die("cannot format 256-bit integer");
    }
    size_t used = (size_t)written;
    while (--count != 0U) {
        written = snprintf(text + used, BIG_TEXT_SIZE - used,
                           "%09" PRIu32, chunks[count - 1U]);
        if (written != 9 || used + (size_t)written >= BIG_TEXT_SIZE) {
            die("cannot format 256-bit integer");
        }
        used += (size_t)written;
    }
}

static void print_u256(FILE *stream, const U256 *value)
{
    char text[BIG_TEXT_SIZE];
    u256_to_text(value, text);
    if (fputs(text, stream) == EOF) {
        die("cannot write output");
    }
}

static void prepare_tables(void)
{
    if (tables_ready) {
        return;
    }
    binomial[0][0] = 1U;
    for (unsigned n = 1U; n <= MAX_N; ++n) {
        binomial[n][0] = 1U;
        binomial[n][n] = 1U;
        for (unsigned k = 1U; k < n; ++k) {
            binomial[n][k] =
                binomial[n - 1U][k - 1U] + binomial[n - 1U][k];
        }
    }
    for (unsigned value = 2U; value <= MAX_PREFIX_SUM; ++value) {
        prime_sum[value] = true;
    }
    for (unsigned divisor = 2U;
         divisor <= MAX_PREFIX_SUM / divisor; ++divisor) {
        if (!prime_sum[divisor]) {
            continue;
        }
        for (unsigned multiple = divisor * divisor;
             multiple <= MAX_PREFIX_SUM; multiple += divisor) {
            prime_sum[multiple] = false;
        }
    }
    tables_ready = true;
}

static bool is_prime_u32(uint32_t value)
{
    if (value < 2U) {
        return false;
    }
    if ((value & 1U) == 0U) {
        return value == 2U;
    }
    for (uint32_t divisor = 3U;
         divisor <= value / divisor; divisor += 2U) {
        if (value % divisor == 0U) {
            return false;
        }
    }
    return true;
}

static uint32_t multiply_mod(uint32_t left, uint32_t right,
                             uint32_t modulus)
{
    return (uint32_t)(((uint64_t)left * right) % modulus);
}

static uint32_t power_mod(uint32_t base, uint32_t exponent,
                          uint32_t modulus)
{
    uint32_t result = 1U;
    while (exponent != 0U) {
        if ((exponent & 1U) != 0U) {
            result = multiply_mod(result, base, modulus);
        }
        base = multiply_mod(base, base, modulus);
        exponent >>= 1U;
    }
    return result;
}

static unsigned select_moduli(unsigned n, unsigned cell_bytes,
                              uint32_t moduli[MAX_MODULI],
                              U256 *product_result)
{
    const uint32_t limit = cell_bytes == 1U ? UINT32_C(255) :
                                                   UINT32_C(65535);
    uint32_t candidate = limit;
    unsigned count = 0U;
    U256 product = {{1U, 0U, 0U, 0U}};
    const U256 bound = big_factorial(n);

    while (big_compare(&product, &bound) <= 0) {
        while (candidate >= 2U && !is_prime_u32(candidate)) {
            --candidate;
        }
        if (candidate < 2U || count == MAX_MODULI) {
            die("not enough pairwise-coprime CRT moduli");
        }
        moduli[count++] = candidate;
        big_multiply_small(&product, candidate);
        --candidate;
    }
    *product_result = product;
    return count;
}

static U256 reconstruct_crt(const uint32_t residues[MAX_MODULI],
                            const uint32_t moduli[MAX_MODULI],
                            unsigned count)
{
    U256 result = {{0U, 0U, 0U, 0U}};
    U256 product = {{1U, 0U, 0U, 0U}};

    for (unsigned i = 0U; i < count; ++i) {
        const uint32_t modulus = moduli[i];
        const uint32_t product_mod = big_modulo_small(&product, modulus);
        const uint32_t inverse =
            power_mod(product_mod, modulus - 2U, modulus);
        const uint32_t result_mod = big_modulo_small(&result, modulus);
        const uint32_t difference = residues[i] >= result_mod ?
            residues[i] - result_mod : modulus - (result_mod - residues[i]);
        const uint32_t multiplier =
            multiply_mod(difference, inverse, modulus);
        big_add_product(&result, &product, multiplier);
        big_multiply_small(&product, modulus);
    }
    return result;
}

static uint64_t layer_bytes(uint64_t count, unsigned cell_bytes)
{
    if (count > UINT64_MAX / cell_bytes ||
        count > SIZE_MAX / cell_bytes) {
        die("DP allocation size overflow");
    }
    return count * cell_bytes;
}

static void *allocate_layer(uint64_t count, unsigned cell_bytes,
                            unsigned n, unsigned cardinality)
{
    const uint64_t bytes = layer_bytes(count, cell_bytes);
    if (bytes > memory_limit) {
        die("one DP layer exceeds the configured memory limit");
    }
    void *layer = malloc((size_t)bytes);
    if (layer == NULL) {
        fprintf(stderr,
                "error: cannot allocate n=%u cardinality=%u layer "
                "(%.3f GiB)\n",
                n, cardinality,
                (double)bytes / (1024.0 * 1024.0 * 1024.0));
        exit(EXIT_FAILURE);
    }
    return layer;
}

static inline uint32_t load_cell(const void *layer, uint64_t index,
                                 unsigned cell_bytes)
{
    return cell_bytes == 1U ? ((const uint8_t *)layer)[index] :
                              ((const uint16_t *)layer)[index];
}

static inline void store_cell(void *layer, uint64_t index,
                              unsigned cell_bytes, uint32_t value)
{
    if (cell_bytes == 1U) {
        ((uint8_t *)layer)[index] = (uint8_t)value;
    } else {
        ((uint16_t *)layer)[index] = (uint16_t)value;
    }
}

static uint64_t next_combination(uint64_t mask)
{
    const uint64_t low_bit = mask & (UINT64_C(0) - mask);
    if (low_bit == 0U) {
        die("internal zero combination mask");
    }
    const uint64_t ripple = mask + low_bit;
    return ripple | (((mask ^ ripple) >> 2U) / low_bit);
}

static uint64_t combination_from_rank(unsigned n, unsigned cardinality,
                                      uint64_t rank)
{
    uint64_t mask = 0U;
    unsigned upper = n;
    for (unsigned i = cardinality; i > 0U; --i) {
        unsigned bit = upper - 1U;
        while (binomial[bit][i] > rank) {
            if (bit == 0U) {
                die("internal combinatorial unranking error");
            }
            --bit;
        }
        mask |= UINT64_C(1) << bit;
        rank -= binomial[bit][i];
        upper = bit;
    }
    if (rank != 0U) {
        die("internal combinatorial unranking remainder");
    }
    return mask;
}

typedef struct {
    const void *previous;
    void *current;
    uint64_t previous_count;
    uint64_t begin;
    uint64_t end;
    unsigned n;
    unsigned cardinality;
    unsigned cell_bytes;
    uint32_t modulus;
} LayerTask;

static void *compute_layer_range(void *argument)
{
    LayerTask *task = argument;
    if (task->begin == task->end) {
        return NULL;
    }

    uint64_t mask = combination_from_rank(
        task->n, task->cardinality, task->begin);
    for (uint64_t index = task->begin; index < task->end; ++index) {
        unsigned positions[MAX_N];
        uint64_t bits = mask;
        unsigned sum = 0U;
        for (unsigned i = 0U; i < task->cardinality; ++i) {
            const unsigned bit = (unsigned)__builtin_ctzll(bits);
            bits &= bits - 1U;
            positions[i] = bit;
            sum += bit + 1U;
        }

        uint32_t residue = 0U;
        if (!prime_sum[sum]) {
            uint64_t prefix_rank[MAX_N + 1U];
            uint64_t shifted_suffix_rank[MAX_N + 1U];
            prefix_rank[0] = 0U;
            for (unsigned i = 0U; i < task->cardinality; ++i) {
                prefix_rank[i + 1U] = prefix_rank[i] +
                    binomial[positions[i]][i + 1U];
            }
            shifted_suffix_rank[task->cardinality] = 0U;
            for (unsigned i = task->cardinality; i-- > 0U;) {
                shifted_suffix_rank[i] = shifted_suffix_rank[i + 1U] +
                    binomial[positions[i]][i];
            }

            uint64_t total = 0U;
            for (unsigned removed = 0U;
                 removed < task->cardinality; ++removed) {
                const uint64_t predecessor = prefix_rank[removed] +
                    shifted_suffix_rank[removed + 1U];
                if (predecessor >= task->previous_count) {
                    die("internal combinatorial rank error");
                }
                total += load_cell(
                    task->previous, predecessor, task->cell_bytes);
            }
            residue = (uint32_t)(total % task->modulus);
        }
        store_cell(task->current, index, task->cell_bytes, residue);
        if (index + 1U < task->end) {
            mask = next_combination(mask);
        }
    }
    return NULL;
}

static void compute_layer(const void *previous, void *current,
                          uint64_t previous_count, uint64_t current_count,
                          unsigned n, unsigned cardinality,
                          unsigned cell_bytes, uint32_t modulus)
{
    unsigned thread_count = configured_threads;
    if (current_count < UINT64_C(1048576)) {
        thread_count = 1U;
    }
    if ((uint64_t)thread_count > current_count) {
        thread_count = (unsigned)current_count;
    }

    LayerTask tasks[MAX_THREADS];
    pthread_t threads[MAX_THREADS];
    for (unsigned thread = 0U; thread < thread_count; ++thread) {
        tasks[thread] = (LayerTask){
            .previous = previous,
            .current = current,
            .previous_count = previous_count,
            .begin = current_count * thread / thread_count,
            .end = current_count * (thread + 1U) / thread_count,
            .n = n,
            .cardinality = cardinality,
            .cell_bytes = cell_bytes,
            .modulus = modulus
        };
    }

    if (thread_count == 1U) {
        (void)compute_layer_range(&tasks[0]);
        return;
    }
    unsigned created = 0U;
    for (; created < thread_count; ++created) {
        if (pthread_create(&threads[created], NULL,
                           compute_layer_range, &tasks[created]) != 0) {
            break;
        }
    }
    if (created != thread_count) {
        for (unsigned thread = 0U; thread < created; ++thread) {
            (void)pthread_join(threads[thread], NULL);
        }
        die("cannot create worker thread");
    }
    for (unsigned thread = 0U; thread < thread_count; ++thread) {
        if (pthread_join(threads[thread], NULL) != 0) {
            die("cannot join worker thread");
        }
    }
}

static uint32_t modular_pass(unsigned n, uint32_t modulus,
                             unsigned cell_bytes, unsigned pass,
                             unsigned pass_count)
{
    uint64_t previous_count = 1U;
    uint64_t previous_bytes = layer_bytes(previous_count, cell_bytes);
    void *previous = allocate_layer(previous_count, cell_bytes, n, 0U);
    store_cell(previous, 0U, cell_bytes, 1U);
    const double started = monotonic_seconds();

    if (!quiet) {
        fprintf(stderr,
                "291504_02: n=%u CRT pass %u/%u p=%" PRIu32
                " cell=%u byte%s\n",
                n, pass + 1U, pass_count, modulus, cell_bytes,
                cell_bytes == 1U ? "" : "s");
    }

    for (unsigned cardinality = 1U; cardinality <= n; ++cardinality) {
        const uint64_t current_count = binomial[n][cardinality];
        const uint64_t current_bytes =
            layer_bytes(current_count, cell_bytes);
        if (previous_bytes > memory_limit ||
            current_bytes > memory_limit - previous_bytes) {
            free(previous);
            die("adjacent DP layers exceed the configured memory limit");
        }
        void *current =
            allocate_layer(current_count, cell_bytes, n, cardinality);
        const bool report = !quiet && current_count >= REPORT_MIN_STATES;
        if (report) {
            fprintf(stderr,
                    "291504_02: n=%u pass=%u/%u layer=%u states=%" PRIu64
                    " started\n",
                    n, pass + 1U, pass_count, cardinality, current_count);
        }
        const double layer_started = monotonic_seconds();
        compute_layer(previous, current, previous_count, current_count,
                      n, cardinality, cell_bytes, modulus);
        if (report) {
            fprintf(stderr,
                    "291504_02: n=%u pass=%u/%u layer=%u done time=%.1fs\n",
                    n, pass + 1U, pass_count, cardinality,
                    monotonic_seconds() - layer_started);
        }

        free(previous);
        previous = current;
        previous_count = current_count;
        previous_bytes = current_bytes;
    }

    if (previous_count != 1U) {
        free(previous);
        die("internal final-layer size error");
    }
    const uint32_t result = load_cell(previous, 0U, cell_bytes);
    free(previous);
    if (!quiet) {
        fprintf(stderr,
                "291504_02: n=%u CRT pass %u/%u done time=%.3fs\n",
                n, pass + 1U, pass_count,
                monotonic_seconds() - started);
    }
    return result;
}

static uint64_t peak_dp_bytes(unsigned n, unsigned cell_bytes)
{
    uint64_t peak = 0U;
    for (unsigned cardinality = 1U; cardinality <= n; ++cardinality) {
        const uint64_t states = binomial[n][cardinality - 1U] +
                                binomial[n][cardinality];
        const uint64_t bytes = layer_bytes(states, cell_bytes);
        if (bytes > peak) {
            peak = bytes;
        }
    }
    return peak;
}

static U256 count_exact(unsigned n)
{
    if (n == 0U) {
        return (U256){{1U, 0U, 0U, 0U}};
    }
    prepare_tables();
    if (prime_sum[n * (n + 1U) / 2U]) {
        return (U256){{0U, 0U, 0U, 0U}};
    }

    const unsigned cell_bytes = n == MAX_N ? 1U : 2U;
    const uint64_t peak = peak_dp_bytes(n, cell_bytes);
    if (peak > memory_limit) {
        fprintf(stderr,
                "error: n=%u needs %.3f GiB for its largest adjacent "
                "layers, but the limit is %.3f GiB\n",
                n, (double)peak / (1024.0 * 1024.0 * 1024.0),
                (double)memory_limit / (1024.0 * 1024.0 * 1024.0));
        exit(EXIT_FAILURE);
    }

    uint32_t moduli[MAX_MODULI] = {0U};
    uint32_t residues[MAX_MODULI] = {0U};
    U256 crt_product;
    const unsigned modulus_count =
        select_moduli(n, cell_bytes, moduli, &crt_product);
    const U256 bound = big_factorial(n);

    if (!quiet) {
        fprintf(stderr,
                "291504_02: n=%u peak=%.3f GiB CRT_passes=%u threads=%u "
                "bound=n!\n",
                n, (double)peak / (1024.0 * 1024.0 * 1024.0),
                modulus_count, configured_threads);
    }
    const double started = monotonic_seconds();
    for (unsigned pass = 0U; pass < modulus_count; ++pass) {
        residues[pass] = modular_pass(
            n, moduli[pass], cell_bytes, pass, modulus_count);
    }
    U256 answer = reconstruct_crt(residues, moduli, modulus_count);

    if (big_compare(&crt_product, &bound) <= 0) {
        die("CRT product does not exceed the rigorous factorial bound");
    }
    if (big_compare(&answer, &bound) > 0) {
        die("CRT reconstruction exceeds the rigorous factorial bound");
    }
    for (unsigned i = 0U; i < modulus_count; ++i) {
        if (big_modulo_small(&answer, moduli[i]) != residues[i]) {
            die("CRT residue replay failed");
        }
    }
    if (!quiet) {
        fprintf(stderr, "291504_02: n=%u exact answer time=%.3fs\n",
                n, monotonic_seconds() - started);
    }
    return answer;
}

/* Independent definition-level DFS used only by --check. */
static uint64_t direct_search(unsigned n, unsigned position, uint64_t used,
                              unsigned prefix_sum)
{
    if (position == n) {
        return 1U;
    }
    uint64_t count = 0U;
    for (unsigned value = 1U; value <= n; ++value) {
        const uint64_t bit = UINT64_C(1) << (value - 1U);
        if ((used & bit) != 0U || prime_sum[prefix_sum + value]) {
            continue;
        }
        const uint64_t addition = direct_search(
            n, position + 1U, used | bit, prefix_sum + value);
        if (count > UINT64_MAX - addition) {
            die("direct-check counter overflow");
        }
        count += addition;
    }
    return count;
}

static int lock_bfile(void)
{
    const int descriptor = open(LOCK_PATH, O_RDWR | O_CREAT, 0666);
    if (descriptor < 0) {
        die("cannot open b-file lock");
    }
    struct flock lock = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };
    while (fcntl(descriptor, F_SETLKW, &lock) != 0) {
        if (errno != EINTR) {
            close(descriptor);
            die("cannot lock b-file");
        }
    }
    return descriptor;
}

static void unlock_bfile(int descriptor)
{
    struct flock lock = {
        .l_type = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };
    if (fcntl(descriptor, F_SETLK, &lock) != 0 || close(descriptor) != 0) {
        die("cannot unlock b-file");
    }
}

static void read_bfile(bool present[MAX_N + 1U],
                       char values[MAX_N + 1U][BIG_TEXT_SIZE])
{
    memset(present, 0, (MAX_N + 1U) * sizeof(*present));
    FILE *input = fopen(BFILE_PATH, "r");
    if (input == NULL) {
        if (errno == ENOENT) {
            return;
        }
        die("cannot read b-file");
    }

    char line[256];
    while (fgets(line, sizeof(line), input) != NULL) {
        unsigned index;
        char number[BIG_TEXT_SIZE];
        char extra;
        if (sscanf(line, "%u %79s %c", &index, number, &extra) != 2 ||
            index > MAX_N || present[index]) {
            (void)fclose(input);
            die("b-file is malformed or contains a duplicate index");
        }
        U256 parsed;
        if (!parse_u256(number, &parsed)) {
            (void)fclose(input);
            die("b-file contains an invalid integer");
        }
        present[index] = true;
        strcpy(values[index], number);
    }
    if (ferror(input) || fclose(input) != 0) {
        die("cannot finish reading b-file");
    }
}

static bool load_saved_term(unsigned n, U256 *value)
{
    bool present[MAX_N + 1U];
    char values[MAX_N + 1U][BIG_TEXT_SIZE];
    const int lock_descriptor = lock_bfile();
    read_bfile(present, values);
    const bool found = present[n];
    if (found && !parse_u256(values[n], value)) {
        unlock_bfile(lock_descriptor);
        die("saved term is invalid");
    }
    unlock_bfile(lock_descriptor);
    return found;
}

static void record_term(unsigned n, const U256 *value)
{
    bool present[MAX_N + 1U];
    char values[MAX_N + 1U][BIG_TEXT_SIZE];
    char text[BIG_TEXT_SIZE];
    u256_to_text(value, text);

    const int lock_descriptor = lock_bfile();
    read_bfile(present, values);
    if (present[n]) {
        if (strcmp(values[n], text) != 0) {
            unlock_bfile(lock_descriptor);
            die("computed term disagrees with the b-file");
        }
        unlock_bfile(lock_descriptor);
        return;
    }
    present[n] = true;
    strcpy(values[n], text);

    char temporary[] = "b291504_02.txt.tmp.XXXXXX";
    const int descriptor = mkstemp(temporary);
    if (descriptor < 0 || fchmod(descriptor, 0644) != 0) {
        if (descriptor >= 0) {
            (void)close(descriptor);
            (void)unlink(temporary);
        }
        unlock_bfile(lock_descriptor);
        die("cannot create temporary b-file");
    }
    FILE *output = fdopen(descriptor, "w");
    if (output == NULL) {
        (void)close(descriptor);
        (void)unlink(temporary);
        unlock_bfile(lock_descriptor);
        die("cannot open temporary b-file stream");
    }

    bool failed = false;
    for (unsigned index = 0U; index <= MAX_N; ++index) {
        if (present[index] &&
            fprintf(output, "%u %s\n", index, values[index]) < 0) {
            failed = true;
        }
    }
    if (!failed && fflush(output) != 0) {
        failed = true;
    }
    if (!failed && fsync(fileno(output)) != 0) {
        failed = true;
    }
    if (fclose(output) != 0) {
        failed = true;
    }
    if (failed || rename(temporary, BFILE_PATH) != 0) {
        (void)unlink(temporary);
        unlock_bfile(lock_descriptor);
        die("cannot atomically update b-file");
    }
    unlock_bfile(lock_descriptor);
    if (!quiet) {
        fprintf(stderr, "291504_02: saved a(%u) in %s\n", n, BFILE_PATH);
    }
}

static U256 known_term(unsigned n)
{
    U256 value;
    if (n > KNOWN_MAX_N || !parse_u256(known_terms[n], &value)) {
        die("internal known-term table error");
    }
    return value;
}

static U256 obtain_term(unsigned n)
{
    U256 value;
    if (load_saved_term(n, &value)) {
        if (!quiet) {
            fprintf(stderr, "291504_02: reusing saved a(%u)\n", n);
        }
        return value;
    }
    value = n <= KNOWN_MAX_N ? known_term(n) : count_exact(n);
    record_term(n, &value);
    return value;
}

static void print_bfile_line(unsigned n, const U256 *value)
{
    if (printf("%u ", n) < 0) {
        die("cannot write standard output");
    }
    print_u256(stdout, value);
    if (putchar('\n') == EOF || fflush(stdout) != 0) {
        die("cannot write standard output");
    }
}

static void check_implementation(unsigned maximum)
{
    prepare_tables();
    const bool saved_quiet = quiet;
    quiet = true;
    for (unsigned n = 0U; n <= maximum; ++n) {
        const U256 modular = count_exact(n);
        U256 direct = {{direct_search(n, 0U, 0U, 0U), 0U, 0U, 0U}};
        const U256 expected = known_term(n);
        if (big_compare(&modular, &direct) != 0 ||
            big_compare(&modular, &expected) != 0) {
            quiet = saved_quiet;
            fprintf(stderr, "error: check mismatch at n=%u\n", n);
            exit(EXIT_FAILURE);
        }
    }
    quiet = saved_quiet;
    printf("ok: modular/CRT DP, direct enumeration, and known terms "
           "agree for n=0..%u\n", maximum);
}

static void usage(const char *program, FILE *stream)
{
    fprintf(stream,
            "Usage:\n"
            "  %s [MAX_N] [--quiet]\n"
            "  %s --upto MAX_N [--quiet]\n"
            "  %s --term N [--quiet]\n"
            "  %s --check [CHECK_N]\n"
            "\n"
            "MAX_N and N may be 0..%u; default MAX_N is %u.\n"
            "CHECK_N may be 0..%u and defaults to %u.\n"
            "Completed terms are saved atomically in %s.\n"
            "A291504_MEMORY_MIB sets the memory limit (default %" PRIu64
            " MiB).\n"
            "A291504_THREADS sets 1..%u worker threads (default: up to 8).\n",
            program, program, program, program,
            MAX_N, DEFAULT_MAX_N,
            DIRECT_CHECK_MAX_N, DIRECT_CHECK_MAX_N,
            BFILE_PATH, DEFAULT_MEMORY_MIB, MAX_THREADS);
}

int main(int argc, char **argv)
{
    configure_memory_limit();
    configure_threads();
    prepare_tables();

    Mode mode = MODE_UPTO;
    bool mode_seen = false;
    bool n_seen = false;
    unsigned n = DEFAULT_MAX_N;

    for (int i = 1; i < argc; ++i) {
        const char *argument = argv[i];
        if (strcmp(argument, "--help") == 0 || strcmp(argument, "-h") == 0) {
            usage(argv[0], stdout);
            return EXIT_SUCCESS;
        }
        if (strcmp(argument, "--quiet") == 0 || strcmp(argument, "-q") == 0) {
            quiet = true;
            continue;
        }
        if (strcmp(argument, "--term") == 0 ||
            strcmp(argument, "--upto") == 0) {
            if (mode_seen || n_seen || i + 1 >= argc) {
                usage(argv[0], stderr);
                return EXIT_FAILURE;
            }
            mode = strcmp(argument, "--term") == 0 ? MODE_TERM : MODE_UPTO;
            mode_seen = true;
            n = parse_unsigned(argv[++i], MAX_N,
                               mode == MODE_TERM ? "N" : "MAX_N");
            n_seen = true;
            continue;
        }
        if (strcmp(argument, "--check") == 0) {
            if (mode_seen || n_seen) {
                usage(argv[0], stderr);
                return EXIT_FAILURE;
            }
            mode = MODE_CHECK;
            mode_seen = true;
            n = DIRECT_CHECK_MAX_N;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                n = parse_unsigned(argv[++i], DIRECT_CHECK_MAX_N, "CHECK_N");
                n_seen = true;
            }
            continue;
        }
        if (argument[0] == '-' || mode_seen || n_seen) {
            usage(argv[0], stderr);
            return EXIT_FAILURE;
        }
        n = parse_unsigned(argument, MAX_N, "MAX_N");
        n_seen = true;
    }

    if (mode == MODE_CHECK) {
        check_implementation(n);
    } else if (mode == MODE_TERM) {
        const U256 value = obtain_term(n);
        print_u256(stdout, &value);
        if (putchar('\n') == EOF || fflush(stdout) != 0) {
            die("cannot write standard output");
        }
    } else {
        for (unsigned index = 0U; index <= n; ++index) {
            const U256 value = obtain_term(index);
            print_bfile_line(index, &value);
        }
    }
    return EXIT_SUCCESS;
}
