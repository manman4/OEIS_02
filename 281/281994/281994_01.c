/*
 * A281994: number of set partitions of [n] into blocks whose element sums
 * are squares.
 *
 * Let F(M) be the number of admissible partitions of the set represented by
 * the bit mask M.  To avoid counting the blocks of a partition in different
 * orders, put the largest element of M in a distinguished block B.  Then
 *
 *     F(0) = 1,
 *     F(M) = sum F(M \\ B),
 *
 * where the sum is over B contained in M, B contains max(M), and sum(B) is
 * a square.  The recursion only visits masks reached by deleting admissible
 * distinguished blocks; this is substantially sparser than filling all
 * 2^n states.  Up to n=24 a flat memo table gives constant-time lookups;
 * larger runs use an open-addressed hash table so that --upto 29 does not
 * require a 4-GiB array.  Subset sums come from two small lookup tables.
 *
 * Dense masks would also require testing every subset of the remaining
 * elements.  For sufficiently large states, admissible subsets are generated
 * directly by a meet-in-the-middle subset-sum construction.  Small states
 * use ordinary submask enumeration.
 * The exponential running time is why this version deliberately caps n at
 * 29.  Results and all additions are checked against uint64_t overflow.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic 281994_01.c -o 281994_01
 *
 * Examples:
 *   ./281994_01
 *   ./281994_01 --upto 18
 *   ./281994_01 --self-test
 */

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_N 29U
#define DEFAULT_N 24U
#define KNOWN_N 28U
#define DENSE_MEMO_MAX_N 24U
#define MITM_MIN_REST_SIZE 8U
#define INITIAL_HASH_CAPACITY ((size_t)1U << 16U)
#define LOW_BITS 16U
#define LOW_SIZE (UINT32_C(1) << LOW_BITS)
#define HIGH_BITS (MAX_N - LOW_BITS)
#define HIGH_SIZE (UINT32_C(1) << HIGH_BITS)
#define MAX_SUM (MAX_N * (MAX_N + 1U) / 2U)

typedef struct {
    uint64_t *dense;
    size_t dense_state_count;
    uint32_t *keys;
    uint64_t *values;
    size_t capacity;
    size_t size;
} Memo;

typedef struct {
    Memo memo;
    uint64_t computed_states;
    uint64_t nonzero_states;
    uint32_t *mitm_grouped[MAX_N];
    size_t mitm_capacity[MAX_N];
    uint16_t low_sum[LOW_SIZE];
    uint16_t high_sum[HIGH_SIZE];
    bool square[MAX_SUM + 1U];
    uint8_t ceil_root[MAX_SUM + 1U];
} Context;

static const uint64_t known[KNOWN_N + 1U] = {
    UINT64_C(1), UINT64_C(1), UINT64_C(0), UINT64_C(0),
    UINT64_C(1), UINT64_C(0), UINT64_C(1), UINT64_C(1),
    UINT64_C(7), UINT64_C(37), UINT64_C(22), UINT64_C(264),
    UINT64_C(310), UINT64_C(1181), UINT64_C(2350), UINT64_C(8102),
    UINT64_C(38471), UINT64_C(136528), UINT64_C(738072),
    UINT64_C(2811383), UINT64_C(12163208), UINT64_C(45121906),
    UINT64_C(160620225), UINT64_C(729516611), UINT64_C(3962353196),
    UINT64_C(20093745700), UINT64_C(128690290224),
    UINT64_C(474568411683), UINT64_C(2815183102398)
};

static _Noreturn void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static unsigned parse_n(const char *text)
{
    errno = 0;
    char *end = NULL;
    const uintmax_t value = strtoumax(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value > MAX_N) {
        fprintf(stderr, "error: n must be in 0..%u: %s\n", MAX_N, text);
        exit(EXIT_FAILURE);
    }
    return (unsigned)value;
}

static unsigned highest_bit_index(uint32_t mask)
{
#if defined(__clang__) || defined(__GNUC__)
    return 31U - (unsigned)__builtin_clz(mask);
#else
    unsigned index = 0;
    while (mask >>= 1U) ++index;
    return index;
#endif
}

static unsigned bit_count(uint32_t mask)
{
#if defined(__clang__) || defined(__GNUC__)
    return (unsigned)__builtin_popcount(mask);
#else
    unsigned count = 0;
    while (mask != 0) {
        mask &= mask - 1U;
        ++count;
    }
    return count;
#endif
}

static uint16_t subset_sum(const Context *context, uint32_t mask)
{
    return (uint16_t)(context->low_sum[mask & (LOW_SIZE - 1U)] +
                      context->high_sum[mask >> LOW_BITS]);
}

static uint32_t hash_mask(uint32_t mask)
{
    mask ^= mask >> 16U;
    mask *= UINT32_C(0x7feb352d);
    mask ^= mask >> 15U;
    mask *= UINT32_C(0x846ca68b);
    mask ^= mask >> 16U;
    return mask;
}

static bool memo_get(const Memo *memo, uint32_t mask, uint64_t *value)
{
    if ((size_t)mask < memo->dense_state_count) {
        const uint64_t encoded = memo->dense[mask];
        if (encoded == 0) return false;
        *value = encoded - 1U;
        return true;
    }

    const uint32_t key = mask + 1U;
    size_t index = (size_t)hash_mask(mask) & (memo->capacity - 1U);
    while (memo->keys[index] != 0) {
        if (memo->keys[index] == key) {
            *value = memo->values[index];
            return true;
        }
        index = (index + 1U) & (memo->capacity - 1U);
    }
    return false;
}

static void memo_insert_raw(Memo *memo, uint32_t mask, uint64_t value)
{
    size_t index = (size_t)hash_mask(mask) & (memo->capacity - 1U);
    while (memo->keys[index] != 0)
        index = (index + 1U) & (memo->capacity - 1U);
    memo->keys[index] = mask + 1U;
    memo->values[index] = value;
    ++memo->size;
}

static void memo_grow(Memo *memo)
{
    if (memo->capacity > SIZE_MAX / 2U)
        die("memo hash table is too large");
    const size_t old_capacity = memo->capacity;
    uint32_t *old_keys = memo->keys;
    uint64_t *old_values = memo->values;
    memo->capacity *= 2U;
    memo->keys = calloc(memo->capacity, sizeof(*memo->keys));
    memo->values = malloc(memo->capacity * sizeof(*memo->values));
    if (memo->keys == NULL || memo->values == NULL)
        die("could not grow the memo hash table");
    memo->size = 0;
    for (size_t i = 0; i < old_capacity; ++i) {
        if (old_keys[i] != 0)
            memo_insert_raw(memo, old_keys[i] - 1U, old_values[i]);
    }
    free(old_keys);
    free(old_values);
}

static void memo_set(Memo *memo, uint32_t mask, uint64_t value)
{
    if ((size_t)mask < memo->dense_state_count) {
        if (value == UINT64_MAX)
            die("dense memo encoding overflow while counting partitions");
        memo->dense[mask] = value + 1U;
        return;
    }
    if ((memo->size + 1U) * 10U >= memo->capacity * 7U)
        memo_grow(memo);
    memo_insert_raw(memo, mask, value);
}

static uint32_t *mitm_workspace(Context *context, unsigned depth,
                                size_t needed)
{
    if (depth >= MAX_N) die("internal recursion depth overflow");
    if (context->mitm_capacity[depth] < needed) {
        if (needed > SIZE_MAX / sizeof(*context->mitm_grouped[depth]))
            die("meet-in-the-middle workspace is too large");
        uint32_t *replacement =
            realloc(context->mitm_grouped[depth],
                    needed * sizeof(*context->mitm_grouped[depth]));
        if (replacement == NULL)
            die("could not allocate meet-in-the-middle workspace");
        context->mitm_grouped[depth] = replacement;
        context->mitm_capacity[depth] = needed;
    }
    return context->mitm_grouped[depth];
}

static void checked_add(uint64_t *total, uint64_t addend)
{
    if (UINT64_MAX - *total < addend)
        die("uint64_t overflow while counting partitions");
    *total += addend;
}

static uint64_t count_partitions(Context *context, uint32_t mask,
                                 unsigned depth)
{
    uint64_t memoized;
    if (memo_get(&context->memo, mask, &memoized)) return memoized;

    const unsigned pivot_index = highest_bit_index(mask);
    const uint32_t pivot = UINT32_C(1) << pivot_index;
    const uint32_t rest = mask ^ pivot;
    const unsigned pivot_value = pivot_index + 1U;
    uint64_t total = 0;

    const unsigned rest_size = bit_count(rest);
    if (rest_size >= MITM_MIN_REST_SIZE) {
        uint32_t low_elements = 0;
        uint32_t high_elements = 0;
        uint32_t copy = rest;
        for (unsigned i = 0; copy != 0; ++i) {
            const uint32_t bit = copy & (0U - copy);
            copy ^= bit;
            if (i < rest_size / 2U)
                low_elements |= bit;
            else
                high_elements |= bit;
        }

        const unsigned high_count = bit_count(high_elements);
        const size_t high_subset_count = (size_t)1U << high_count;
        const unsigned maximum_high_sum =
            (unsigned)subset_sum(context, high_elements);
        uint32_t counts[MAX_SUM + 1U];
        uint32_t offsets[MAX_SUM + 2U];
        memset(counts, 0,
               (maximum_high_sum + 1U) * sizeof(*counts));
        uint32_t high = high_elements;
        for (;;) {
            ++counts[subset_sum(context, high)];
            if (high == 0) break;
            high = (high - 1U) & high_elements;
        }
        offsets[0] = 0;
        for (unsigned sum = 0; sum <= maximum_high_sum; ++sum)
            offsets[sum + 1U] = offsets[sum] + counts[sum];

        uint32_t *grouped =
            mitm_workspace(context, depth, high_subset_count);
        uint32_t cursor[MAX_SUM + 1U];
        memcpy(cursor, offsets,
               (maximum_high_sum + 1U) * sizeof(*cursor));
        high = high_elements;
        for (;;) {
            const unsigned sum = subset_sum(context, high);
            grouped[cursor[sum]++] = high;
            if (high == 0) break;
            high = (high - 1U) & high_elements;
        }

        const unsigned maximum_block_sum =
            pivot_value + (unsigned)subset_sum(context, rest);
        uint32_t low = low_elements;
        for (;;) {
            const unsigned low_value = subset_sum(context, low);
            for (unsigned root = context->ceil_root[pivot_value + low_value];
                 root <= maximum_block_sum / root; ++root) {
                const unsigned square = root * root;
                if (square < pivot_value + low_value) continue;
                const unsigned high_value =
                    square - pivot_value - low_value;
                if (high_value > maximum_high_sum) continue;
                for (uint32_t i = offsets[high_value];
                     i < offsets[high_value + 1U]; ++i) {
                    const uint32_t block_rest = low | grouped[i];
                    checked_add(&total,
                                count_partitions(context,
                                                 rest ^ block_rest,
                                                 depth + 1U));
                }
            }
            if (low == 0) break;
            low = (low - 1U) & low_elements;
        }
    } else {
        uint32_t block_rest = rest;
        for (;;) {
            const unsigned block_sum =
                pivot_value + (unsigned)subset_sum(context, block_rest);
            if (context->square[block_sum])
                checked_add(&total,
                            count_partitions(context, rest ^ block_rest,
                                             depth + 1U));
            if (block_rest == 0) break;
            block_rest = (block_rest - 1U) & rest;
        }
    }

    memo_set(&context->memo, mask, total);
    ++context->computed_states;
    if (total != 0) ++context->nonzero_states;
    return total;
}

static void initialize_tables(Context *context)
{
    context->low_sum[0] = 0;
    for (uint32_t mask = 1; mask < LOW_SIZE; ++mask) {
        const uint32_t bit = mask & (0U - mask);
        context->low_sum[mask] =
            (uint16_t)(context->low_sum[mask ^ bit] +
                       highest_bit_index(bit) + 1U);
    }

    context->high_sum[0] = 0;
    for (uint32_t mask = 1; mask < HIGH_SIZE; ++mask) {
        const uint32_t bit = mask & (0U - mask);
        context->high_sum[mask] =
            (uint16_t)(context->high_sum[mask ^ bit] + LOW_BITS +
                       highest_bit_index(bit) + 1U);
    }

    memset(context->square, 0, sizeof(context->square));
    for (unsigned root = 1; root <= MAX_SUM / root; ++root)
        context->square[root * root] = true;
    unsigned root = 0;
    for (unsigned value = 0; value <= MAX_SUM; ++value) {
        while (root * root < value) ++root;
        context->ceil_root[value] = (uint8_t)root;
    }
}

static Context *context_create(unsigned n)
{
    Context *context = calloc(1, sizeof(*context));
    if (context == NULL) die("could not allocate the DP context");

    const unsigned dense_n =
        n < DENSE_MEMO_MAX_N ? n : DENSE_MEMO_MAX_N;
    context->memo.dense_state_count = (size_t)UINT32_C(1) << dense_n;
    context->memo.dense =
        calloc(context->memo.dense_state_count,
               sizeof(*context->memo.dense));
    if (context->memo.dense == NULL) {
        free(context);
        die("could not allocate the dense memo table");
    }
    if (n > DENSE_MEMO_MAX_N) {
        context->memo.capacity = INITIAL_HASH_CAPACITY;
        context->memo.keys =
            calloc(context->memo.capacity, sizeof(*context->memo.keys));
        context->memo.values =
            malloc(context->memo.capacity * sizeof(*context->memo.values));
        if (context->memo.keys == NULL || context->memo.values == NULL) {
            free(context->memo.dense);
            free(context->memo.keys);
            free(context->memo.values);
            free(context);
            die("could not allocate the memo hash table");
        }
    }
    initialize_tables(context);
    memo_set(&context->memo, 0, 1U);
    context->computed_states = 1;
    context->nonzero_states = 1;
    return context;
}

static void context_destroy(Context *context)
{
    free(context->memo.dense);
    free(context->memo.keys);
    free(context->memo.values);
    for (unsigned i = 0; i < MAX_N; ++i)
        free(context->mitm_grouped[i]);
    free(context);
}

static void usage(FILE *stream, const char *program)
{
    fprintf(stream,
            "Usage: %s [--upto N] [--self-test] [--stats]\n"
            "  --upto N     print a(0)..a(N), 0 <= N <= %u (default %u)\n"
            "               N=29 can take tens of minutes and several GiB\n"
            "  --self-test  compare computed terms with the OEIS terms\n"
            "  --stats      report memoized-state counts on stderr\n"
            "  --help       show this help\n",
            program, MAX_N, DEFAULT_N);
}

int main(int argc, char **argv)
{
    unsigned n = DEFAULT_N;
    bool self_test = false;
    bool stats = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--upto") == 0) {
            if (++i == argc) die("--upto requires an argument");
            n = parse_n(argv[i]);
        } else if (strcmp(argv[i], "--self-test") == 0) {
            self_test = true;
        } else if (strcmp(argv[i], "--stats") == 0) {
            stats = true;
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            usage(stdout, argv[0]);
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "error: unknown argument: %s\n", argv[i]);
            usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
    }

    Context *context = context_create(n);
    for (unsigned k = 0; k <= n; ++k) {
        const uint32_t mask =
            k == 0 ? 0U : (UINT32_C(1) << k) - 1U;
        const uint64_t value = count_partitions(context, mask, 0);
        if (self_test && k <= KNOWN_N && value != known[k]) {
            fprintf(stderr,
                    "self-test failed at n=%u: got %" PRIu64
                    ", expected %" PRIu64 "\n",
                    k, value, known[k]);
            context_destroy(context);
            return EXIT_FAILURE;
        }
        printf("%u %" PRIu64 "\n", k, value);
        if (fflush(stdout) == EOF) die("could not flush sequence output");
    }
    if (self_test) {
        const unsigned tested = n < KNOWN_N ? n : KNOWN_N;
        fprintf(stderr, "self-test passed through known n=%u\n", tested);
    }
    if (stats)
        fprintf(stderr,
                "memoized states: %" PRIu64 ", nonzero: %" PRIu64 "\n",
                context->computed_states, context->nonzero_states);
    context_destroy(context);
    return EXIT_SUCCESS;
}
