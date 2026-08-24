/*
 * A281994: number of set partitions of [n] into blocks whose element sums
 * are squares.
 *
 * This version is a parallel exact-cover decomposition.  For a set M, put
 * its largest element in a distinguished square-sum block B.  Then
 *
 *     F(0) = 1,
 *     F(M) = sum F(M \\ B).
 *
 * At the root, meet-in-the-middle subset-sum generation lists every possible
 * B and a dynamic pthread work queue distributes the complementary masks
 * among workers.  Recursive subproblems use one concurrent transposition
 * table: an atomic busy bit guarantees that every mask is evaluated once.
 * Strictly smaller popcounts make waits cycle-free.
 *
 * Masks below 2^24 use a dense atomic table.  Larger masks use a fixed-size
 * open-addressed atomic table allocated from a conservative n-dependent
 * bound.  During --upto, tables grow only between consecutive n and all
 * completed entries are rehashed, so no concurrent resize is possible and
 * smaller terms do not pay for the n=31 table.  Hash collisions are resolved
 * by the complete mask key.  Counts, table load, allocation sizes, and output
 * are checked.  Sparse tags reserve their top bit as the atomic busy flag and
 * use the lower 31 bits as the complete mask key.  The deliberate cap is n=31.
 * Sparse values below 2^31 use 32 bits; larger values are placed in compact
 * append-only chunks.  This lowers the n=31 base memo from about 6.1 GiB to
 * about 4.1 GiB without truncating any uint64_t result.
 *
 * Build:
 *   clang -O3 -march=native -std=c11 -Wall -Wextra -Wpedantic \
 *       281994_01.c -o 281994_01 -pthread
 *
 * Examples:
 *   ./281994_01 --target 31 --threads 8 --self-test --stats
 *   ./281994_01 --upto 24 --self-test
 *
 * Every result line is also written immediately to b281994_01.txt in the
 * current directory.  fflush plus fsync after each term preserves the
 * completed prefix if a long --upto run is interrupted.
 * For n >= 24, root-job progress is reported to stderr every 60 seconds.
 */

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <sched.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_N 31U
#define DEFAULT_N 24U
#define KNOWN_N 29U
#define DENSE_MEMO_MAX_N 24U
#define MITM_MIN_REST_SIZE 8U
#define PARALLEL_MIN_N 18U
#define TASK_CHUNK 64U
#define PROGRESS_MIN_N 24U
#define PROGRESS_INTERVAL_SECONDS 60.0
#define PROGRESS_POLL_NANOSECONDS 100000000L
#define BFILE_NAME "b281994_01.txt"
#define SPARSE_BUSY_BIT UINT32_C(0x80000000)
#define WIDE_VALUE_BIT UINT32_C(0x80000000)
#define WIDE_CHUNK_BITS 16U
#define WIDE_CHUNK_SIZE (UINT32_C(1) << WIDE_CHUNK_BITS)
#define WIDE_CHUNK_COUNT (UINT32_C(1) << (31U - WIDE_CHUNK_BITS))
#define LOW_BITS 16U
#define LOW_SIZE (UINT32_C(1) << LOW_BITS)
#define HIGH_BITS (MAX_N - LOW_BITS)
#define HIGH_SIZE (UINT32_C(1) << HIGH_BITS)
#define MAX_SUM (MAX_N * (MAX_N + 1U) / 2U)

_Static_assert(MAX_N <= 31U, "uint32_t element masks support at most n=31");
_Static_assert(MAX_SUM <= UINT16_MAX,
               "subset-sum lookup entries must fit in uint16_t");

typedef struct {
    _Atomic uint64_t *dense;
    size_t dense_state_count;
    _Atomic uint32_t *tags;
    uint32_t *values;
    _Atomic(uint64_t *) *wide_chunks;
    _Atomic uint32_t wide_size;
    pthread_mutex_t wide_mutex;
    size_t capacity;
    _Atomic size_t size;
} ConcurrentMemo;

typedef struct {
    ConcurrentMemo memo;
    _Atomic uint64_t computed_states;
    _Atomic uint64_t nonzero_states;
    _Atomic uint64_t wait_count;
    unsigned thread_count;
    unsigned n;
    uint16_t low_sum[LOW_SIZE];
    uint16_t high_sum[HIGH_SIZE];
    bool square[MAX_SUM + 1U];
    uint8_t ceil_root[MAX_SUM + 1U];
} Shared;

typedef struct {
    Shared *shared;
    uint32_t *mitm_grouped[MAX_N];
    size_t mitm_capacity[MAX_N];
} Worker;

typedef struct {
    bool dense;
    size_t index;
} MemoToken;

static const uint64_t known[KNOWN_N + 1U] = {
    UINT64_C(1), UINT64_C(1), UINT64_C(0), UINT64_C(0),
    UINT64_C(1), UINT64_C(0), UINT64_C(1), UINT64_C(1),
    UINT64_C(7), UINT64_C(37), UINT64_C(22), UINT64_C(264),
    UINT64_C(310), UINT64_C(1181), UINT64_C(2350), UINT64_C(8102),
    UINT64_C(38471), UINT64_C(136528), UINT64_C(738072),
    UINT64_C(2811383), UINT64_C(12163208), UINT64_C(45121906),
    UINT64_C(160620225), UINT64_C(729516611), UINT64_C(3962353196),
    UINT64_C(20093745700), UINT64_C(128690290224),
    UINT64_C(474568411683), UINT64_C(2815183102398),
    UINT64_C(14478251405091)
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

static unsigned parse_threads(const char *text)
{
    errno = 0;
    char *end = NULL;
    const uintmax_t value = strtoumax(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value == 0 || value > 64U) {
        fprintf(stderr, "error: threads must be in 1..64: %s\n", text);
        exit(EXIT_FAILURE);
    }
    return (unsigned)value;
}

static unsigned default_thread_count(void)
{
    const long detected = sysconf(_SC_NPROCESSORS_ONLN);
    if (detected <= 0) return 1U;
    return detected > 8 ? 8U : (unsigned)detected;
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

static uint16_t subset_sum(const Shared *shared, uint32_t mask)
{
    return (uint16_t)(shared->low_sum[mask & (LOW_SIZE - 1U)] +
                      shared->high_sum[mask >> LOW_BITS]);
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

static uint64_t *wide_chunk(ConcurrentMemo *memo, uint32_t chunk_index,
                            bool create)
{
    if (chunk_index >= WIDE_CHUNK_COUNT)
        die("wide-value chunk index overflow");
    uint64_t *chunk =
        atomic_load_explicit(&memo->wide_chunks[chunk_index],
                             memory_order_acquire);
    if (chunk != NULL || !create) return chunk;

    const int lock_error = pthread_mutex_lock(&memo->wide_mutex);
    if (lock_error != 0) {
        fprintf(stderr, "error: pthread_mutex_lock: %s\n",
                strerror(lock_error));
        exit(EXIT_FAILURE);
    }
    chunk = atomic_load_explicit(&memo->wide_chunks[chunk_index],
                                 memory_order_relaxed);
    if (chunk == NULL) {
        if (WIDE_CHUNK_SIZE > SIZE_MAX / sizeof(*chunk))
            die("wide-value chunk allocation size overflow");
        chunk = malloc((size_t)WIDE_CHUNK_SIZE * sizeof(*chunk));
        if (chunk == NULL) die("could not allocate a wide-value chunk");
        atomic_store_explicit(&memo->wide_chunks[chunk_index], chunk,
                              memory_order_release);
    }
    const int unlock_error = pthread_mutex_unlock(&memo->wide_mutex);
    if (unlock_error != 0) {
        fprintf(stderr, "error: pthread_mutex_unlock: %s\n",
                strerror(unlock_error));
        exit(EXIT_FAILURE);
    }
    return chunk;
}

static uint32_t encode_sparse_value(ConcurrentMemo *memo, uint64_t value)
{
    if (value < WIDE_VALUE_BIT) return (uint32_t)value;
    const uint32_t index =
        atomic_fetch_add_explicit(&memo->wide_size, 1,
                                  memory_order_relaxed);
    if (index >= WIDE_VALUE_BIT)
        die("too many wide sparse memo values");
    uint64_t *chunk = wide_chunk(memo, index >> WIDE_CHUNK_BITS, true);
    chunk[index & (WIDE_CHUNK_SIZE - 1U)] = value;
    return WIDE_VALUE_BIT | index;
}

static uint64_t decode_sparse_value(ConcurrentMemo *memo, size_t index)
{
    const uint32_t encoded = memo->values[index];
    if ((encoded & WIDE_VALUE_BIT) == 0) return encoded;
    const uint32_t wide_index = encoded & ~WIDE_VALUE_BIT;
    uint64_t *chunk =
        wide_chunk(memo, wide_index >> WIDE_CHUNK_BITS, false);
    if (chunk == NULL) die("missing wide sparse memo value");
    return chunk[wide_index & (WIDE_CHUNK_SIZE - 1U)];
}

static void clear_wide_values(ConcurrentMemo *memo)
{
    const uint32_t count =
        atomic_load_explicit(&memo->wide_size, memory_order_relaxed);
    const uint32_t chunks = count == 0 ? 0U
        : ((count - 1U) >> WIDE_CHUNK_BITS) + 1U;
    for (uint32_t i = 0; i < chunks; ++i) {
        uint64_t *chunk =
            atomic_exchange_explicit(&memo->wide_chunks[i], NULL,
                                     memory_order_relaxed);
        free(chunk);
    }
    atomic_store_explicit(&memo->wide_size, 0, memory_order_relaxed);
}

static bool memo_claim(Shared *shared, uint32_t mask, uint64_t *value,
                       MemoToken *token)
{
    ConcurrentMemo *memo = &shared->memo;
    if ((size_t)mask < memo->dense_state_count) {
        token->dense = true;
        token->index = mask;
        for (;;) {
            uint64_t encoded =
                atomic_load_explicit(&memo->dense[mask],
                                     memory_order_acquire);
            if (encoded == UINT64_MAX) {
                atomic_fetch_add_explicit(&shared->wait_count, 1,
                                          memory_order_relaxed);
                unsigned spins = 0;
                do {
                    encoded =
                        atomic_load_explicit(&memo->dense[mask],
                                             memory_order_acquire);
                    if (++spins == 1024U && encoded == UINT64_MAX) {
                        sched_yield();
                        spins = 0;
                    }
                } while (encoded == UINT64_MAX);
                if (encoded != 0) {
                    *value = encoded - 1U;
                    return false;
                }
                continue;
            }
            if (encoded != 0) {
                *value = encoded - 1U;
                return false;
            }
            uint64_t expected = 0;
            if (atomic_compare_exchange_weak_explicit(
                    &memo->dense[mask], &expected, UINT64_MAX,
                    memory_order_acq_rel, memory_order_acquire))
                return true;
        }
    }

    if (memo->capacity == 0)
        die("internal sparse memo capacity is zero");
    if ((mask & SPARSE_BUSY_BIT) != 0)
        die("sparse memo mask exceeds 31-bit tag encoding");
    const uint32_t complete_tag = mask;
    const uint32_t busy_tag = complete_tag | SPARSE_BUSY_BIT;
    size_t index = (size_t)hash_mask(mask) & (memo->capacity - 1U);
    for (size_t probes = 0; probes < memo->capacity; ++probes) {
        uint32_t tag = atomic_load_explicit(&memo->tags[index],
                                            memory_order_acquire);
        if (tag == complete_tag) {
            *value = decode_sparse_value(memo, index);
            return false;
        }
        if (tag == busy_tag) {
            atomic_fetch_add_explicit(&shared->wait_count, 1,
                                      memory_order_relaxed);
            unsigned spins = 0;
            do {
                tag = atomic_load_explicit(&memo->tags[index],
                                           memory_order_acquire);
                if (++spins == 1024U && tag == busy_tag) {
                    sched_yield();
                    spins = 0;
                }
            } while (tag == busy_tag);
            if (tag == complete_tag) {
                *value = decode_sparse_value(memo, index);
                return false;
            }
            continue;
        }
        if (tag == 0) {
            uint32_t expected = 0;
            if (atomic_compare_exchange_weak_explicit(
                    &memo->tags[index], &expected, busy_tag,
                    memory_order_acq_rel, memory_order_acquire)) {
                const size_t size =
                    atomic_fetch_add_explicit(&memo->size, 1,
                                              memory_order_relaxed) + 1U;
                if (size * 10U >= memo->capacity * 7U)
                    die("concurrent memo table exceeded 70% load");
                token->dense = false;
                token->index = index;
                return true;
            }
            continue;
        }
        index = (index + 1U) & (memo->capacity - 1U);
    }
    die("concurrent memo table is full");
}

static void memo_publish(Shared *shared, uint32_t mask,
                         const MemoToken *token, uint64_t value)
{
    if (token->dense) {
        if (value >= UINT64_MAX - 1U)
            die("dense memo encoding overflow");
        atomic_store_explicit(&shared->memo.dense[token->index], value + 1U,
                              memory_order_release);
    } else {
        shared->memo.values[token->index] =
            encode_sparse_value(&shared->memo, value);
        const uint32_t complete_tag = mask;
        atomic_store_explicit(&shared->memo.tags[token->index], complete_tag,
                              memory_order_release);
    }
    atomic_fetch_add_explicit(&shared->computed_states, 1,
                              memory_order_relaxed);
    if (value != 0)
        atomic_fetch_add_explicit(&shared->nonzero_states, 1,
                                  memory_order_relaxed);
}

static uint32_t *mitm_workspace(Worker *worker, unsigned depth,
                                size_t needed)
{
    if (depth >= MAX_N) die("internal recursion depth overflow");
    if (worker->mitm_capacity[depth] < needed) {
        if (needed > SIZE_MAX / sizeof(*worker->mitm_grouped[depth]))
            die("meet-in-the-middle workspace is too large");
        uint32_t *replacement =
            realloc(worker->mitm_grouped[depth],
                    needed * sizeof(*worker->mitm_grouped[depth]));
        if (replacement == NULL)
            die("could not allocate meet-in-the-middle workspace");
        worker->mitm_grouped[depth] = replacement;
        worker->mitm_capacity[depth] = needed;
    }
    return worker->mitm_grouped[depth];
}

static void checked_add(uint64_t *total, uint64_t addend)
{
    if (UINT64_MAX - *total < addend)
        die("uint64_t overflow while counting partitions");
    *total += addend;
}

static uint64_t count_partitions(Worker *worker, uint32_t mask,
                                 unsigned depth)
{
    Shared *shared = worker->shared;
    uint64_t memoized;
    MemoToken token;
    if (!memo_claim(shared, mask, &memoized, &token)) return memoized;

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
            (unsigned)subset_sum(shared, high_elements);
        uint32_t counts[MAX_SUM + 1U];
        uint32_t offsets[MAX_SUM + 2U];
        memset(counts, 0,
               (maximum_high_sum + 1U) * sizeof(*counts));
        uint32_t high = high_elements;
        for (;;) {
            ++counts[subset_sum(shared, high)];
            if (high == 0) break;
            high = (high - 1U) & high_elements;
        }
        offsets[0] = 0;
        for (unsigned sum = 0; sum <= maximum_high_sum; ++sum)
            offsets[sum + 1U] = offsets[sum] + counts[sum];

        uint32_t *grouped =
            mitm_workspace(worker, depth, high_subset_count);
        uint32_t cursor[MAX_SUM + 1U];
        memcpy(cursor, offsets,
               (maximum_high_sum + 1U) * sizeof(*cursor));
        high = high_elements;
        for (;;) {
            const unsigned sum = subset_sum(shared, high);
            grouped[cursor[sum]++] = high;
            if (high == 0) break;
            high = (high - 1U) & high_elements;
        }

        const unsigned maximum_block_sum =
            pivot_value + (unsigned)subset_sum(shared, rest);
        uint32_t low = low_elements;
        for (;;) {
            const unsigned low_value = subset_sum(shared, low);
            for (unsigned root = shared->ceil_root[pivot_value + low_value];
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
                                count_partitions(worker,
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
                pivot_value + (unsigned)subset_sum(shared, block_rest);
            if (shared->square[block_sum])
                checked_add(&total,
                            count_partitions(worker, rest ^ block_rest,
                                             depth + 1U));
            if (block_rest == 0) break;
            block_rest = (block_rest - 1U) & rest;
        }
    }

    memo_publish(shared, mask, &token, total);
    return total;
}

static void initialize_tables(Shared *shared)
{
    shared->low_sum[0] = 0;
    for (uint32_t mask = 1; mask < LOW_SIZE; ++mask) {
        const uint32_t bit = mask & (0U - mask);
        shared->low_sum[mask] =
            (uint16_t)(shared->low_sum[mask ^ bit] +
                       highest_bit_index(bit) + 1U);
    }

    shared->high_sum[0] = 0;
    for (uint32_t mask = 1; mask < HIGH_SIZE; ++mask) {
        const uint32_t bit = mask & (0U - mask);
        shared->high_sum[mask] =
            (uint16_t)(shared->high_sum[mask ^ bit] + LOW_BITS +
                       highest_bit_index(bit) + 1U);
    }

    memset(shared->square, 0, sizeof(shared->square));
    for (unsigned root = 1; root <= MAX_SUM / root; ++root)
        shared->square[root * root] = true;
    unsigned root = 0;
    for (unsigned value = 0; value <= MAX_SUM; ++value) {
        while (root * root < value) ++root;
        shared->ceil_root[value] = (uint8_t)root;
    }
}

static double monotonic_seconds(void);

static size_t sparse_capacity_for_n(unsigned n)
{
    if (n < 25U) return 0;
    const unsigned sparse_bits = n == 25U ? 10U : n - 2U;
    return (size_t)1U << sparse_bits;
}

static void grow_dense_memo(Shared *shared, size_t new_count)
{
    ConcurrentMemo *memo = &shared->memo;
    if (new_count <= memo->dense_state_count) return;
    if (new_count > SIZE_MAX / sizeof(*memo->dense))
        die("dense concurrent memo allocation size overflow");
    _Atomic uint64_t *replacement =
        calloc(new_count, sizeof(*replacement));
    if (replacement == NULL)
        die("could not grow the dense concurrent memo table");
    for (size_t i = 0; i < memo->dense_state_count; ++i) {
        const uint64_t encoded =
            atomic_load_explicit(&memo->dense[i], memory_order_relaxed);
        if (encoded == UINT64_MAX)
            die("busy dense memo entry found between terms");
        atomic_store_explicit(&replacement[i], encoded,
                              memory_order_relaxed);
    }
    free(memo->dense);
    memo->dense = replacement;
    memo->dense_state_count = new_count;
}

static void grow_sparse_memo(Shared *shared, size_t new_capacity,
                             unsigned new_n)
{
    ConcurrentMemo *memo = &shared->memo;
    if (new_capacity <= memo->capacity) return;
    if (new_capacity > SIZE_MAX / sizeof(*memo->tags) ||
        new_capacity > SIZE_MAX / sizeof(*memo->values))
        die("sparse concurrent memo allocation size overflow");

    const double started = monotonic_seconds();
    fprintf(stderr,
            "preparing n=%u: sparse memo %zu -> %zu slots\n",
            new_n, memo->capacity, new_capacity);
    if (fflush(stderr) == EOF)
        die("could not flush memo growth output");

    const size_t old_size =
        atomic_load_explicit(&memo->size, memory_order_relaxed);
    if (old_size > SIZE_MAX / sizeof(uint32_t) ||
        old_size > SIZE_MAX / sizeof(uint64_t))
        die("sparse memo compaction allocation size overflow");
    uint32_t *saved_masks = old_size == 0 ? NULL
        : malloc(old_size * sizeof(*saved_masks));
    uint64_t *saved_values = old_size == 0 ? NULL
        : malloc(old_size * sizeof(*saved_values));
    if (old_size != 0 && (saved_masks == NULL || saved_values == NULL))
        die("could not compact the sparse memo before growing");

    size_t saved = 0;
    for (size_t i = 0; i < memo->capacity; ++i) {
        const uint32_t tag =
            atomic_load_explicit(&memo->tags[i], memory_order_relaxed);
        if (tag == 0) continue;
        if ((tag & SPARSE_BUSY_BIT) != 0)
            die("busy sparse memo entry found between terms");
        const uint32_t mask = tag;
        if (saved == old_size)
            die("sparse memo contains more entries than its size");
        saved_masks[saved] = mask;
        saved_values[saved] = decode_sparse_value(memo, i);
        ++saved;
    }
    if (saved != old_size)
        die("sparse memo size mismatch while growing between terms");

    _Atomic uint32_t *new_tags =
        calloc(new_capacity, sizeof(*new_tags));
    if (new_tags == NULL)
        die("could not allocate the grown sparse memo keys");
    free(memo->tags);
    free(memo->values);
    clear_wide_values(memo);
    uint32_t *new_values =
        malloc(new_capacity * sizeof(*new_values));
    if (new_values == NULL)
        die("could not allocate the grown sparse memo values");

    for (size_t i = 0; i < saved; ++i) {
        const uint32_t mask = saved_masks[i];
        const uint32_t tag = mask;
        size_t index =
            (size_t)hash_mask(mask) & (new_capacity - 1U);
        while (atomic_load_explicit(&new_tags[index],
                                    memory_order_relaxed) != 0)
            index = (index + 1U) & (new_capacity - 1U);
        new_values[index] = encode_sparse_value(memo, saved_values[i]);
        atomic_store_explicit(&new_tags[index], tag,
                              memory_order_relaxed);
    }
    free(saved_masks);
    free(saved_values);
    memo->tags = new_tags;
    memo->values = new_values;
    memo->capacity = new_capacity;
    fprintf(stderr,
            "preparing n=%u: sparse memo ready in %.1fs\n",
            new_n, monotonic_seconds() - started);
    if (fflush(stderr) == EOF)
        die("could not flush memo growth output");
}

static void shared_prepare_n(Shared *shared, unsigned n)
{
    if (n < shared->n) die("internal attempt to shrink the DP context");
    const unsigned dense_n =
        n < DENSE_MEMO_MAX_N ? n : DENSE_MEMO_MAX_N;
    const size_t dense_count = (size_t)UINT32_C(1) << dense_n;
    grow_dense_memo(shared, dense_count);
    grow_sparse_memo(shared, sparse_capacity_for_n(n), n);
    shared->n = n;
}

static Shared *shared_create(unsigned n, unsigned thread_count)
{
    Shared *shared = calloc(1, sizeof(*shared));
    if (shared == NULL) die("could not allocate the shared DP context");
    shared->thread_count = thread_count;
    shared->memo.wide_chunks =
        calloc(WIDE_CHUNK_COUNT, sizeof(*shared->memo.wide_chunks));
    if (shared->memo.wide_chunks == NULL)
        die("could not allocate wide-value chunk pointers");
    const int mutex_error = pthread_mutex_init(&shared->memo.wide_mutex, NULL);
    if (mutex_error != 0) {
        fprintf(stderr, "error: pthread_mutex_init: %s\n",
                strerror(mutex_error));
        exit(EXIT_FAILURE);
    }
    initialize_tables(shared);
    shared_prepare_n(shared, n);
    uint64_t ignored;
    MemoToken token;
    if (!memo_claim(shared, 0, &ignored, &token))
        die("could not claim the empty memo state");
    memo_publish(shared, 0, &token, 1U);
    return shared;
}

static void worker_destroy(Worker *worker)
{
    for (unsigned i = 0; i < MAX_N; ++i)
        free(worker->mitm_grouped[i]);
}

static void shared_destroy(Shared *shared)
{
    free(shared->memo.dense);
    free(shared->memo.tags);
    free(shared->memo.values);
    clear_wide_values(&shared->memo);
    free(shared->memo.wide_chunks);
    const int mutex_error = pthread_mutex_destroy(&shared->memo.wide_mutex);
    if (mutex_error != 0) {
        fprintf(stderr, "error: pthread_mutex_destroy: %s\n",
                strerror(mutex_error));
        exit(EXIT_FAILURE);
    }
    free(shared);
}

typedef struct {
    Shared *shared;
    uint32_t *block_rests;
    size_t block_count;
    uint32_t rest;
    _Atomic size_t next_task;
    _Atomic size_t completed_tasks;
    _Atomic bool stop_progress;
    Worker *workers;
    uint64_t *results;
    uint64_t initial_state_count;
    double start_time;
    unsigned term;
} RootQueue;

typedef struct {
    RootQueue *queue;
    unsigned worker_index;
} ThreadArgument;

static double monotonic_seconds(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        die("could not read the monotonic clock");
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static void report_progress(const RootQueue *queue, bool finished)
{
    const size_t completed =
        atomic_load_explicit(&queue->completed_tasks,
                             memory_order_relaxed);
    const uint64_t states =
        atomic_load_explicit(&queue->shared->computed_states,
                             memory_order_relaxed);
    const uint64_t new_states = states - queue->initial_state_count;
    const double percent = queue->block_count == 0
        ? 100.0
        : 100.0 * (double)completed / (double)queue->block_count;
    fprintf(stderr,
            "progress n=%u: root jobs %zu/%zu (%.1f%%), "
            "new states=%" PRIu64 ", elapsed=%.1fs%s\n",
            queue->term, completed, queue->block_count, percent,
            new_states, monotonic_seconds() - queue->start_time,
            finished ? ", done" : "");
    if (fflush(stderr) == EOF)
        die("could not flush progress output");
}

static void *progress_worker(void *opaque)
{
    RootQueue *queue = opaque;
    double next_report = queue->start_time + PROGRESS_INTERVAL_SECONDS;
    const struct timespec pause = {0, PROGRESS_POLL_NANOSECONDS};
    while (!atomic_load_explicit(&queue->stop_progress,
                                 memory_order_acquire)) {
        (void)nanosleep(&pause, NULL);
        const double now = monotonic_seconds();
        if (now >= next_report &&
            !atomic_load_explicit(&queue->stop_progress,
                                  memory_order_acquire)) {
            report_progress(queue, false);
            do {
                next_report += PROGRESS_INTERVAL_SECONDS;
            } while (next_report <= now);
        }
    }
    return NULL;
}

static uint32_t *build_root_blocks(const Shared *shared, uint32_t mask,
                                   size_t *result_count)
{
    const unsigned pivot_index = highest_bit_index(mask);
    const unsigned pivot_value = pivot_index + 1U;
    const uint32_t pivot = UINT32_C(1) << pivot_index;
    const uint32_t rest = mask ^ pivot;
    const unsigned rest_size = bit_count(rest);
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

    const unsigned maximum_high_sum = subset_sum(shared, high_elements);
    const size_t high_subset_count =
        (size_t)1U << bit_count(high_elements);
    uint32_t counts[MAX_SUM + 1U];
    uint32_t offsets[MAX_SUM + 2U];
    uint32_t cursor[MAX_SUM + 1U];
    memset(counts, 0,
           (maximum_high_sum + 1U) * sizeof(*counts));

    uint32_t high = high_elements;
    for (;;) {
        ++counts[subset_sum(shared, high)];
        if (high == 0) break;
        high = (high - 1U) & high_elements;
    }
    offsets[0] = 0;
    for (unsigned sum = 0; sum <= maximum_high_sum; ++sum)
        offsets[sum + 1U] = offsets[sum] + counts[sum];
    memcpy(cursor, offsets,
           (maximum_high_sum + 1U) * sizeof(*cursor));

    uint32_t *grouped = malloc(high_subset_count * sizeof(*grouped));
    if (grouped == NULL) die("could not allocate root subset-sum groups");
    high = high_elements;
    for (;;) {
        const unsigned sum = subset_sum(shared, high);
        grouped[cursor[sum]++] = high;
        if (high == 0) break;
        high = (high - 1U) & high_elements;
    }

    const unsigned maximum_block_sum =
        pivot_value + subset_sum(shared, rest);
    size_t total = 0;
    uint32_t low = low_elements;
    for (;;) {
        const unsigned low_value = subset_sum(shared, low);
        for (unsigned root = shared->ceil_root[pivot_value + low_value];
             root <= maximum_block_sum / root; ++root) {
            const unsigned high_value =
                root * root - pivot_value - low_value;
            if (high_value <= maximum_high_sum)
                total += counts[high_value];
        }
        if (low == 0) break;
        low = (low - 1U) & low_elements;
    }
    if (total > SIZE_MAX / sizeof(uint32_t))
        die("root block list allocation size overflow");
    uint32_t *blocks = malloc(total * sizeof(*blocks));
    if (blocks == NULL && total != 0)
        die("could not allocate the root block list");

    size_t used = 0;
    low = low_elements;
    for (;;) {
        const unsigned low_value = subset_sum(shared, low);
        for (unsigned root = shared->ceil_root[pivot_value + low_value];
             root <= maximum_block_sum / root; ++root) {
            const unsigned high_value =
                root * root - pivot_value - low_value;
            if (high_value > maximum_high_sum) continue;
            for (uint32_t i = offsets[high_value];
                 i < offsets[high_value + 1U]; ++i)
                blocks[used++] = low | grouped[i];
        }
        if (low == 0) break;
        low = (low - 1U) & low_elements;
    }
    free(grouped);
    if (used != total) die("internal root block list size mismatch");
    *result_count = total;
    return blocks;
}

static void *root_worker(void *opaque)
{
    ThreadArgument *argument = opaque;
    RootQueue *queue = argument->queue;
    Worker *worker = &queue->workers[argument->worker_index];
    uint64_t total = 0;
    for (;;) {
        const size_t begin =
            atomic_fetch_add_explicit(&queue->next_task, TASK_CHUNK,
                                      memory_order_relaxed);
        if (begin >= queue->block_count) break;
        size_t end = begin + TASK_CHUNK;
        if (end > queue->block_count) end = queue->block_count;
        for (size_t i = begin; i < end; ++i)
            checked_add(&total,
                        count_partitions(worker,
                                         queue->rest ^ queue->block_rests[i],
                                         0));
        atomic_fetch_add_explicit(&queue->completed_tasks, end - begin,
                                  memory_order_relaxed);
    }
    queue->results[argument->worker_index] = total;
    return NULL;
}

static uint64_t count_full(Shared *shared, uint32_t mask)
{
    Worker serial_worker;
    memset(&serial_worker, 0, sizeof(serial_worker));
    serial_worker.shared = shared;
    const unsigned term = bit_count(mask);
    if (term < PARALLEL_MIN_N ||
        (shared->thread_count == 1U && term < PROGRESS_MIN_N)) {
        const uint64_t result = count_partitions(&serial_worker, mask, 0);
        worker_destroy(&serial_worker);
        return result;
    }

    uint64_t memoized;
    MemoToken root_token;
    if (!memo_claim(shared, mask, &memoized, &root_token)) {
        worker_destroy(&serial_worker);
        return memoized;
    }

    size_t block_count = 0;
    uint32_t *blocks = build_root_blocks(shared, mask, &block_count);
    const unsigned thread_count = shared->thread_count;
    RootQueue queue;
    memset(&queue, 0, sizeof(queue));
    queue.shared = shared;
    queue.block_rests = blocks;
    queue.block_count = block_count;
    queue.term = term;
    queue.initial_state_count =
        atomic_load_explicit(&shared->computed_states,
                             memory_order_relaxed);
    queue.start_time = monotonic_seconds();
    const unsigned pivot_index = highest_bit_index(mask);
    queue.rest = mask ^ (UINT32_C(1) << pivot_index);
    queue.workers = calloc(thread_count, sizeof(*queue.workers));
    queue.results = calloc(thread_count, sizeof(*queue.results));
    ThreadArgument *arguments =
        calloc(thread_count, sizeof(*arguments));
    pthread_t *threads =
        thread_count > 1U ? calloc(thread_count - 1U, sizeof(*threads)) : NULL;
    pthread_t progress_thread;
    const bool show_progress = term >= PROGRESS_MIN_N;
    if (queue.workers == NULL || queue.results == NULL || arguments == NULL ||
        (thread_count > 1U && threads == NULL))
        die("could not allocate root worker data");
    for (unsigned i = 0; i < thread_count; ++i) {
        queue.workers[i].shared = shared;
        arguments[i].queue = &queue;
        arguments[i].worker_index = i;
    }

    if (show_progress) {
        report_progress(&queue, false);
        const int error =
            pthread_create(&progress_thread, NULL, progress_worker, &queue);
        if (error != 0) {
            fprintf(stderr, "error: pthread_create: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }

    for (unsigned i = 1; i < thread_count; ++i) {
        const int error =
            pthread_create(&threads[i - 1U], NULL, root_worker, &arguments[i]);
        if (error != 0) {
            fprintf(stderr, "error: pthread_create: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }
    root_worker(&arguments[0]);
    for (unsigned i = 1; i < thread_count; ++i) {
        const int error = pthread_join(threads[i - 1U], NULL);
        if (error != 0) {
            fprintf(stderr, "error: pthread_join: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }

    if (show_progress) {
        atomic_store_explicit(&queue.stop_progress, true,
                              memory_order_release);
        const int error = pthread_join(progress_thread, NULL);
        if (error != 0) {
            fprintf(stderr, "error: pthread_join: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }

    uint64_t result = 0;
    for (unsigned i = 0; i < thread_count; ++i) {
        checked_add(&result, queue.results[i]);
        worker_destroy(&queue.workers[i]);
    }
    memo_publish(shared, mask, &root_token, result);
    if (show_progress) report_progress(&queue, true);
    free(threads);
    free(arguments);
    free(queue.results);
    free(queue.workers);
    free(blocks);
    worker_destroy(&serial_worker);
    return result;
}

static void usage(FILE *stream, const char *program)
{
    fprintf(stream,
            "Usage: %s [--upto N | --target N] [--threads T] "
            "[--self-test] [--stats]\n"
            "  --upto N     print a(0)..a(N), 0 <= N <= %u (default %u)\n"
            "  --target N   print only a(N)\n"
            "               appends when the b-file contains n=0..N-1\n"
            "               n=31 uses about 4.1 GiB plus wide-value chunks;\n"
            "               --upto 31 can temporarily require about 6 GiB\n"
            "  --threads T  use 1..64 workers (default: up to 8 CPUs)\n"
            "  --self-test  compare computed terms with regression values\n"
            "  --stats      report states, waits, and sparse-table load\n"
            "  For n >= 24, progress is reported every 60 seconds.\n"
            "  Results are also synchronized to " BFILE_NAME
            " after every term.\n"
            "  --help       show this help\n",
            program, MAX_N, DEFAULT_N);
}

static FILE *open_bfile(bool target_only, unsigned target)
{
    if (target_only) {
        FILE *input = fopen(BFILE_NAME, "r");
        if (input != NULL) {
            char line[128];
            unsigned expected = 0;
            while (fgets(line, sizeof(line), input) != NULL) {
                unsigned index;
                uint64_t value;
                char extra;
                if (strchr(line, '\n') == NULL && !feof(input))
                    die("b-file line is too long");
                if (sscanf(line, "%u %" SCNu64 " %c",
                           &index, &value, &extra) != 2 ||
                    index != expected)
                    die("existing b-file is malformed or has a gap");
                if (index <= KNOWN_N && value != known[index])
                    die("existing b-file disagrees with a known value");
                ++expected;
            }
            if (ferror(input) || fclose(input) != 0)
                die("could not finish reading the existing b-file");
            if (expected != 0 && expected != target) {
                fprintf(stderr,
                        "error: --target %u requires an existing b-file "
                        "through n=%u, but it has %u terms\n",
                        target, target == 0 ? 0 : target - 1U, expected);
                exit(EXIT_FAILURE);
            }
            FILE *stream = fopen(BFILE_NAME, expected == 0 ? "w" : "a");
            if (stream == NULL) {
                fprintf(stderr, "error: could not open %s: %s\n",
                        BFILE_NAME, strerror(errno));
                exit(EXIT_FAILURE);
            }
            return stream;
        }
        if (errno != ENOENT) {
            fprintf(stderr, "error: could not read %s: %s\n",
                    BFILE_NAME, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    FILE *stream = fopen(BFILE_NAME, "w");
    if (stream == NULL) {
        fprintf(stderr, "error: could not create %s: %s\n",
                BFILE_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return stream;
}

static void write_bfile_term(FILE *stream, unsigned n, uint64_t value)
{
    if (fprintf(stream, "%u %" PRIu64 "\n", n, value) < 0 ||
        fflush(stream) != 0) {
        fprintf(stderr, "error: could not write %s: %s\n",
                BFILE_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    const int descriptor = fileno(stream);
    if (descriptor < 0 || fsync(descriptor) != 0) {
        fprintf(stderr, "error: could not synchronize %s: %s\n",
                BFILE_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void close_bfile(FILE *stream)
{
    if (fclose(stream) != 0) {
        fprintf(stderr, "error: could not close %s: %s\n",
                BFILE_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    unsigned n = DEFAULT_N;
    unsigned thread_count = default_thread_count();
    bool target_only = false;
    bool self_test = false;
    bool stats = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--upto") == 0) {
            if (++i == argc) die("--upto requires an argument");
            n = parse_n(argv[i]);
            target_only = false;
        } else if (strcmp(argv[i], "--target") == 0) {
            if (++i == argc) die("--target requires an argument");
            n = parse_n(argv[i]);
            target_only = true;
        } else if (strcmp(argv[i], "--threads") == 0) {
            if (++i == argc) die("--threads requires an argument");
            thread_count = parse_threads(argv[i]);
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

    const unsigned initial_n = target_only ? n : 0U;
    Shared *shared = shared_create(initial_n, thread_count);
    FILE *bfile = open_bfile(target_only, n);
    const unsigned first = target_only ? n : 0U;
    for (unsigned k = first; k <= n; ++k) {
        shared_prepare_n(shared, k);
        const uint32_t mask =
            k == 0 ? 0U : (UINT32_C(1) << k) - 1U;
        const uint64_t value = count_full(shared, mask);
        if (self_test && k <= KNOWN_N && value != known[k]) {
            fprintf(stderr,
                    "self-test failed at n=%u: got %" PRIu64
                    ", expected %" PRIu64 "\n",
                    k, value, known[k]);
            close_bfile(bfile);
            shared_destroy(shared);
            return EXIT_FAILURE;
        }
        write_bfile_term(bfile, k, value);
        printf("%u %" PRIu64 "\n", k, value);
        if (fflush(stdout) == EOF) die("could not flush sequence output");
    }
    if (self_test) {
        const unsigned tested = n < KNOWN_N ? n : KNOWN_N;
        fprintf(stderr, "self-test passed through known n=%u\n", tested);
    }
    if (stats)
        fprintf(stderr,
                "memoized states: %" PRIu64 ", nonzero: %" PRIu64
                ", waits: %" PRIu64 ", sparse: %zu/%zu, wide: %u\n",
                atomic_load_explicit(&shared->computed_states,
                                     memory_order_relaxed),
                atomic_load_explicit(&shared->nonzero_states,
                                     memory_order_relaxed),
                atomic_load_explicit(&shared->wait_count,
                                     memory_order_relaxed),
                atomic_load_explicit(&shared->memo.size,
                                     memory_order_relaxed),
                shared->memo.capacity,
                atomic_load_explicit(&shared->memo.wide_size,
                                     memory_order_relaxed));
    close_bfile(bfile);
    shared_destroy(shared);
    return EXIT_SUCCESS;
}
