/*
 * A291355 -- compact reverse sparse subset DP for n <= 150.
 *
 * Count permutations s_1,...,s_n of [n] such that
 *
 *     j divides Sum_{i=1..j} s_i^3,       1 <= j <= n.
 *
 * For a set U occupying the first j positions, put
 * P(U) = Sum_{x in U} x^3.  If x is removed as s_j, the preceding
 * prefix is valid exactly when
 *
 *     x^3 == P(U) (mod j-1).
 *
 * Consequently the number C(U) of valid orders of U through position
 * j-1 satisfies
 *
 *     C(U) = Sum C(U-{x}),
 *
 * over the x satisfying the congruence, with C(U)=1 for |U|<=1.
 * The root additionally requires P([n]) == 0 (mod n).  Since
 * P([n])=(n(n+1)/2)^2, n == 2 (mod 4) is rejected immediately.
 *
 * Differences from 291355_01.c:
 *
 *   - Memo keys use only ceil(batch_max/8) bytes.
 *   - Dense key/value records are separate from a uint32_t hash index.
 *     Only the small index is copied during growth.
 *   - Counts below UINT32_MAX are stored in four bytes.  Larger counts
 *     are kept exactly in a sparse 128-bit overflow array.
 *   - An exact bounded lookahead (four steps by default) rejects a branch
 *     only when no valid continuation of that length exists.
 *   - The same memo is reused for each block 4m+3,4m+4,4m+5.  C(U)
 *     depends only on U, and the search graph for each valid [n] is a
 *     subgraph of that for [n+1] through the last value n+1.
 *   - If the configured memo limit is reached, insertion is frozen and
 *     the exact search continues by recomputing uncached states.
 *
 * Counts use checked unsigned 128-bit arithmetic.  The memo allocation
 * limit defaults to 5120 MiB and is set by A291355_MEMORY_MIB.
 * Completed terms are atomically stored in b291355_02.txt.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       291355_02.c -o 291355_02
 *
 * Examples:
 *   ./291355_02 --upto 150
 *   ./291355_02 --term 149
 *   ./291355_02 --check
 */

#define _POSIX_C_SOURCE 200809L
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if !defined(__SIZEOF_INT128__)
#error "291355_02.c requires unsigned __int128"
#endif

#if SIZE_MAX < UINT64_MAX
#error "291355_02.c requires a 64-bit size_t"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 150U
#define DEFAULT_MAX_N 150U
#define DIRECT_CHECK_MAX_N 16U
#define MAX_KEY_WORDS ((MAX_N + 63U) / 64U)
#define U128_TEXT_SIZE 40U
#define BFILE_PATH "b291355_02.txt"
#define LOCK_PATH "b291355_02.txt.lock"
#define DEFAULT_MEMORY_MIB UINT64_C(5120)
#define MIN_MEMORY_MIB UINT64_C(32)
#define MAX_MEMORY_MIB UINT64_C(65536)
#define INITIAL_INDEX_CAPACITY ((size_t)32768U)
#define INDEX_LOAD_NUMERATOR ((size_t)85U)
#define INDEX_LOAD_DENOMINATOR ((size_t)100U)
#define RECORD_CHUNK_SHIFT 18U
#define RECORDS_PER_CHUNK ((size_t)1U << RECORD_CHUNK_SHIFT)
#define RECORD_CHUNK_MASK (RECORDS_PER_CHUNK - 1U)
#define MAX_RECORD_CHUNKS ((size_t)1U << (32U - RECORD_CHUNK_SHIFT))
#define DEFAULT_LOOKAHEAD 4U
#define MAX_LOOKAHEAD 6U
#define PROGRESS_INTERVAL UINT64_C(10000000)

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} Mode;

typedef struct {
    uint64_t word[MAX_KEY_WORDS];
} Bitset;

/*
 * A dense record is key_bytes packed key bytes followed by a uint32_t
 * value.  UINT32_MAX marks a value stored in the exact overflow array.
 */
typedef struct {
    uint32_t identifier;
    uint32_t reserved;
    uint64_t low;
    uint64_t high;
} OverflowEntry;

typedef struct {
    unsigned char **chunks;
    uint32_t *index;
    OverflowEntry *overflow;
    size_t overflow_size;
    size_t overflow_capacity;
    size_t index_capacity;
    size_t size;
    unsigned key_words;
    unsigned key_bytes;
    unsigned full_key_words;
    unsigned tail_key_bytes;
    size_t record_bytes;
    uint64_t memory_limit;
    uint64_t memory_current;
    uint64_t memory_peak;
    uint64_t calls;
    uint64_t hits;
    uint64_t pruned;
    uint64_t uncached;
    uint64_t root_calls;
    uint64_t root_hits;
    uint64_t root_pruned;
    uint64_t root_uncached;
    size_t root_size;
    uint64_t next_report_calls;
    unsigned root_n;
    double root_started;
    bool frozen;
    bool freeze_reported;
} Memo;

typedef struct {
    U128 value;
    uint64_t calls;
    uint64_t hits;
    uint64_t pruned;
    uint64_t uncached;
    size_t states_added;
    size_t states_total;
    uint64_t memory_peak;
    bool frozen;
} SearchResult;

static uint64_t cubes[MAX_N + 1U];
static Bitset residue_masks[MAX_N + 1U][MAX_N];
static bool tables_ready;
static bool quiet;
static uint64_t configured_memory_limit;
static unsigned lookahead_depth = DEFAULT_LOOKAHEAD;

static const uint64_t known_terms[DIRECT_CHECK_MAX_N + 1U] = {
    1U, 1U, 0U, 2U, 4U, 8U, 0U, 8U, 16U, 24U, 0U, 46U, 46U,
    46U, 0U, 218U, 1658U
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
    const char *text = getenv("A291355_MEMORY_MIB");
    if (text != NULL && *text != '\0') {
        char *end = NULL;
        errno = 0;
        const unsigned long long parsed = strtoull(text, &end, 10);
        if (errno != 0 || end == text || *end != '\0' ||
            parsed < MIN_MEMORY_MIB || parsed > MAX_MEMORY_MIB) {
            fprintf(stderr,
                    "error: A291355_MEMORY_MIB must be in %" PRIu64
                    "..%" PRIu64 ": %s\n",
                    MIN_MEMORY_MIB, MAX_MEMORY_MIB, text);
            exit(EXIT_FAILURE);
        }
        mib = (uint64_t)parsed;
    }
    configured_memory_limit = mib << 20U;
}

static void checked_add(U128 *target, U128 addition)
{
    const U128 maximum = ~(U128)0;
    if (*target > maximum - addition) {
        die("unsigned 128-bit count overflow");
    }
    *target += addition;
}

static void u128_to_text(U128 value, char text[U128_TEXT_SIZE])
{
    char reverse[U128_TEXT_SIZE];
    size_t length = 0U;
    do {
        if (length + 1U >= U128_TEXT_SIZE) {
            die("internal decimal conversion overflow");
        }
        reverse[length++] = (char)('0' + (unsigned)(value % 10U));
        value /= 10U;
    } while (value != 0U);
    for (size_t i = 0U; i < length; ++i) {
        text[i] = reverse[length - 1U - i];
    }
    text[length] = '\0';
}

static int print_u128(FILE *stream, U128 value)
{
    char text[U128_TEXT_SIZE];
    u128_to_text(value, text);
    return fputs(text, stream) == EOF ? -1 : 0;
}

static bool parse_u128(const char *text, U128 *result)
{
    const U128 maximum = ~(U128)0;
    U128 value = 0U;
    if (*text == '\0') {
        return false;
    }
    while (*text != '\0') {
        if (*text < '0' || *text > '9') {
            return false;
        }
        const unsigned digit = (unsigned)(*text++ - '0');
        if (value > (maximum - digit) / 10U) {
            return false;
        }
        value = value * 10U + digit;
    }
    *result = value;
    return true;
}

static inline Bitset bitset_intersection(Bitset left, Bitset right)
{
    Bitset result;
    for (unsigned i = 0U; i < MAX_KEY_WORDS; ++i) {
        result.word[i] = left.word[i] & right.word[i];
    }
    return result;
}

static inline Bitset bitset_without(Bitset set, unsigned value)
{
    const unsigned bit_index = value - 1U;
    set.word[bit_index / 64U] ^=
        UINT64_C(1) << (bit_index % 64U);
    return set;
}

static Bitset full_set(unsigned n)
{
    Bitset result = {{0U}};
    const unsigned full_words = n / 64U;
    const unsigned remaining_bits = n % 64U;
    for (unsigned i = 0U; i < full_words; ++i) {
        result.word[i] = UINT64_MAX;
    }
    if (remaining_bits != 0U) {
        result.word[full_words] =
            (UINT64_C(1) << remaining_bits) - 1U;
    }
    return result;
}

static unsigned words_for_n(unsigned n)
{
    const unsigned words = (n + 63U) / 64U;
    return words == 0U ? 1U : words;
}

static void prepare_tables(void)
{
    if (tables_ready) {
        return;
    }
    for (unsigned value = 1U; value <= MAX_N; ++value) {
        cubes[value] = (uint64_t)value * value * value;
    }
    for (unsigned modulus = 1U; modulus <= MAX_N; ++modulus) {
        for (unsigned value = 1U; value <= MAX_N; ++value) {
            Bitset *bucket =
                &residue_masks[modulus][cubes[value] % modulus];
            const unsigned bit_index = value - 1U;
            bucket->word[bit_index / 64U] |=
                UINT64_C(1) << (bit_index % 64U);
        }
    }
    tables_ready = true;
}

static uint64_t mix64(uint64_t value)
{
    value ^= value >> 30U;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27U;
    value *= UINT64_C(0x94d049bb133111eb);
    value ^= value >> 31U;
    return value;
}

static uint64_t hash_words(const uint64_t *words, unsigned count)
{
    uint64_t hash = UINT64_C(0x6a09e667f3bcc909);
    for (unsigned i = 0U; i < count; ++i) {
        const uint64_t mixed = mix64(
            words[i] + UINT64_C(0x9e3779b97f4a7c15) * (i + 1U));
        hash ^= mixed;
        hash = (hash << 17U | hash >> 47U) *
            UINT64_C(0x9e3779b97f4a7c15);
    }
    return hash;
}

static bool memory_fits(const Memo *memo, uint64_t bytes)
{
    return memo->memory_current <= memo->memory_limit &&
        bytes <= memo->memory_limit - memo->memory_current;
}

static void account_allocation(Memo *memo, uint64_t bytes)
{
    memo->memory_current += bytes;
    if (memo->memory_current > memo->memory_peak) {
        memo->memory_peak = memo->memory_current;
    }
}

static unsigned char *memo_record(const Memo *memo, size_t identifier)
{
    const size_t chunk = identifier >> RECORD_CHUNK_SHIFT;
    const size_t offset = identifier & RECORD_CHUNK_MASK;
    return memo->chunks[chunk] + offset * memo->record_bytes;
}

static uint64_t record_key_word(const Memo *memo,
                                const unsigned char *record,
                                unsigned word)
{
    uint64_t value = 0U;
    if (word < memo->full_key_words) {
        memcpy(&value, record + (size_t)word * sizeof(value),
               sizeof(value));
    } else {
        const size_t start =
            (size_t)memo->full_key_words * sizeof(value);
        for (unsigned byte = 0U; byte < memo->tail_key_bytes; ++byte) {
            value |= (uint64_t)record[start + byte] << (8U * byte);
        }
    }
    return value;
}

static bool record_equals(const Memo *memo, const unsigned char *record,
                          Bitset key)
{
    for (unsigned i = 0U; i < memo->key_words; ++i) {
        if (record_key_word(memo, record, i) != key.word[i]) {
            return false;
        }
    }
    return true;
}

static uint64_t record_hash(const Memo *memo,
                            const unsigned char *record)
{
    uint64_t words[MAX_KEY_WORDS];
    for (unsigned i = 0U; i < memo->key_words; ++i) {
        words[i] = record_key_word(memo, record, i);
    }
    return hash_words(words, memo->key_words);
}

static U128 overflow_value(const Memo *memo, uint32_t identifier)
{
    size_t low = 0U;
    size_t high = memo->overflow_size;
    while (low < high) {
        const size_t middle = low + (high - low) / 2U;
        const uint32_t saved = memo->overflow[middle].identifier;
        if (saved < identifier) {
            low = middle + 1U;
        } else {
            high = middle;
        }
    }
    if (low >= memo->overflow_size ||
        memo->overflow[low].identifier != identifier) {
        die("internal overflow-value lookup failure");
    }
    return (U128)memo->overflow[low].low |
        (U128)memo->overflow[low].high << 64U;
}

static U128 record_value(const Memo *memo, const unsigned char *record,
                         uint32_t identifier)
{
    uint32_t compact;
    memcpy(&compact, record + memo->key_bytes, sizeof(compact));
    if (compact != UINT32_MAX) {
        return compact;
    }
    return overflow_value(memo, identifier);
}

static void index_place(const Memo *memo, uint32_t *index,
                        size_t capacity, size_t identifier)
{
    const unsigned char *record = memo_record(memo, identifier);
    const uint64_t hash = record_hash(memo, record);
    size_t position = (size_t)hash & (capacity - 1U);
    const size_t step = ((size_t)(hash >> 32U) | 1U) &
        (capacity - 1U);
    while (index[position] != 0U) {
        position = (position + step) & (capacity - 1U);
    }
    index[position] = (uint32_t)(identifier + 1U);
}

static void memo_report_freeze(Memo *memo)
{
    if (!memo->freeze_reported && !quiet) {
        fprintf(stderr,
                "291355_02: n=%u memo frozen at %zu states, %.1f MiB; "
                "exact search continues with recomputation\n",
                memo->root_n, memo->size,
                (double)memo->memory_current / (1024.0 * 1024.0));
        memo->freeze_reported = true;
    }
}

static void memo_freeze(Memo *memo)
{
    memo->frozen = true;
    memo_report_freeze(memo);
}

static bool memo_grow_index(Memo *memo)
{
    if (memo->index_capacity > SIZE_MAX / 2U) {
        return false;
    }
    const size_t new_capacity = memo->index_capacity * 2U;
    if (new_capacity > SIZE_MAX / sizeof(*memo->index) ||
        new_capacity > UINT64_MAX / sizeof(*memo->index)) {
        return false;
    }
    const uint64_t new_bytes =
        (uint64_t)new_capacity * sizeof(*memo->index);
    const uint64_t old_bytes =
        (uint64_t)memo->index_capacity * sizeof(*memo->index);
    if (memo->memory_current < old_bytes ||
        new_bytes > memo->memory_limit -
            (memo->memory_current - old_bytes)) {
        return false;
    }
    /* Dense records are authoritative, so the old index contents are not
       needed while rebuilding.  realloc avoids an explicit old+new index
       allocation peak. */
    uint32_t *new_index = realloc(memo->index, (size_t)new_bytes);
    if (new_index == NULL) {
        return false;
    }
    memo->index = new_index;
    memo->index_capacity = new_capacity;
    memo->memory_current = memo->memory_current - old_bytes + new_bytes;
    if (memo->memory_current > memo->memory_peak) {
        memo->memory_peak = memo->memory_current;
    }
    memset(memo->index, 0, (size_t)new_bytes);
    for (size_t identifier = 0U; identifier < memo->size; ++identifier) {
        index_place(memo, memo->index, memo->index_capacity, identifier);
    }
    return true;
}

static bool memo_allocate_chunk(Memo *memo)
{
    const size_t chunk_index = memo->size >> RECORD_CHUNK_SHIFT;
    if (chunk_index >= MAX_RECORD_CHUNKS) {
        return false;
    }
    if (memo->record_bytes >
        UINT64_MAX / (uint64_t)RECORDS_PER_CHUNK) {
        return false;
    }
    const uint64_t bytes =
        (uint64_t)RECORDS_PER_CHUNK * memo->record_bytes;
    if (!memory_fits(memo, bytes)) {
        return false;
    }
    unsigned char *chunk = malloc((size_t)bytes);
    if (chunk == NULL) {
        return false;
    }
    memo->chunks[chunk_index] = chunk;
    account_allocation(memo, bytes);
    return true;
}

static bool memo_grow_overflow(Memo *memo)
{
    const size_t new_capacity = memo->overflow_capacity == 0U ?
        256U : memo->overflow_capacity * 2U;
    if (new_capacity < memo->overflow_capacity ||
        new_capacity > SIZE_MAX / sizeof(*memo->overflow) ||
        new_capacity > UINT64_MAX / sizeof(*memo->overflow)) {
        return false;
    }
    const uint64_t old_bytes =
        (uint64_t)memo->overflow_capacity * sizeof(*memo->overflow);
    const uint64_t new_bytes =
        (uint64_t)new_capacity * sizeof(*memo->overflow);
    if (memo->memory_current < old_bytes ||
        new_bytes > memo->memory_limit -
            (memo->memory_current - old_bytes)) {
        return false;
    }
    OverflowEntry *resized = realloc(memo->overflow, (size_t)new_bytes);
    if (resized == NULL) {
        return false;
    }
    memo->overflow = resized;
    memo->overflow_capacity = new_capacity;
    memo->memory_current = memo->memory_current - old_bytes + new_bytes;
    if (memo->memory_current > memo->memory_peak) {
        memo->memory_peak = memo->memory_current;
    }
    return true;
}

static void memo_store_key(const Memo *memo, unsigned char *record,
                           Bitset key)
{
    for (unsigned word = 0U; word < memo->full_key_words; ++word) {
        memcpy(record + (size_t)word * sizeof(uint64_t),
               &key.word[word], sizeof(uint64_t));
    }
    const size_t start =
        (size_t)memo->full_key_words * sizeof(uint64_t);
    if (memo->tail_key_bytes != 0U) {
        const uint64_t tail = key.word[memo->full_key_words];
        for (unsigned byte = 0U; byte < memo->tail_key_bytes; ++byte) {
            record[start + byte] = (unsigned char)(tail >> (8U * byte));
        }
    }
}

static void memo_init(Memo *memo, unsigned maximum_value)
{
    memset(memo, 0, sizeof(*memo));
    memo->key_words = words_for_n(maximum_value);
    memo->key_bytes = (maximum_value + 7U) / 8U;
    if (memo->key_bytes == 0U) {
        memo->key_bytes = 1U;
    }
    memo->full_key_words = memo->key_bytes / sizeof(uint64_t);
    memo->tail_key_bytes = memo->key_bytes % sizeof(uint64_t);
    memo->record_bytes = memo->key_bytes + sizeof(uint32_t);
    memo->memory_limit = configured_memory_limit;
    memo->index_capacity = INITIAL_INDEX_CAPACITY;

    const uint64_t pointer_bytes =
        (uint64_t)MAX_RECORD_CHUNKS * sizeof(*memo->chunks);
    const uint64_t index_bytes =
        (uint64_t)memo->index_capacity * sizeof(*memo->index);
    if (pointer_bytes + index_bytes > memo->memory_limit) {
        die("A291355_MEMORY_MIB is too small for the initial memo");
    }
    memo->chunks = calloc(MAX_RECORD_CHUNKS, sizeof(*memo->chunks));
    memo->index = calloc(memo->index_capacity, sizeof(*memo->index));
    if (memo->chunks == NULL || memo->index == NULL) {
        free(memo->chunks);
        free(memo->index);
        die("cannot allocate initial memo structures");
    }
    memo->memory_current = pointer_bytes + index_bytes;
    memo->memory_peak = memo->memory_current;
}

static void memo_destroy(Memo *memo)
{
    const size_t chunk_count =
        (memo->size + RECORDS_PER_CHUNK - 1U) >> RECORD_CHUNK_SHIFT;
    for (size_t i = 0U; i < chunk_count; ++i) {
        free(memo->chunks[i]);
    }
    free(memo->chunks);
    free(memo->index);
    free(memo->overflow);
    memset(memo, 0, sizeof(*memo));
}

static bool memo_lookup(Memo *memo, Bitset key, U128 *value)
{
    const uint64_t hash = hash_words(key.word, memo->key_words);
    size_t position = (size_t)hash & (memo->index_capacity - 1U);
    const size_t step = ((size_t)(hash >> 32U) | 1U) &
        (memo->index_capacity - 1U);
    for (;;) {
        const uint32_t tag = memo->index[position];
        if (tag == 0U) {
            return false;
        }
        const size_t identifier = (size_t)tag - 1U;
        const unsigned char *record = memo_record(memo, identifier);
        if (record_equals(memo, record, key)) {
            *value = record_value(memo, record, (uint32_t)identifier);
            ++memo->hits;
            return true;
        }
        position = (position + step) & (memo->index_capacity - 1U);
    }
}

static bool memo_insert(Memo *memo, Bitset key, U128 value)
{
    if (memo->frozen) {
        ++memo->uncached;
        return false;
    }
    if (memo->size >= (size_t)UINT32_MAX - 1U) {
        memo_freeze(memo);
        ++memo->uncached;
        return false;
    }
    if (memo->size + 1U >
        memo->index_capacity * INDEX_LOAD_NUMERATOR /
            INDEX_LOAD_DENOMINATOR &&
        !memo_grow_index(memo)) {
        memo_freeze(memo);
        ++memo->uncached;
        return false;
    }
    if ((memo->size & RECORD_CHUNK_MASK) == 0U &&
        !memo_allocate_chunk(memo)) {
        memo_freeze(memo);
        ++memo->uncached;
        return false;
    }

    const bool overflow = value >= UINT32_MAX;
    if (overflow && memo->overflow_size == memo->overflow_capacity &&
        !memo_grow_overflow(memo)) {
        memo_freeze(memo);
        ++memo->uncached;
        return false;
    }

    const size_t identifier = memo->size;
    unsigned char *record = memo_record(memo, identifier);
    memo_store_key(memo, record, key);
    uint32_t compact = (uint32_t)value;
    if (overflow) {
        compact = UINT32_MAX;
        OverflowEntry *entry = &memo->overflow[memo->overflow_size++];
        entry->identifier = (uint32_t)identifier;
        entry->reserved = 0U;
        entry->low = (uint64_t)value;
        entry->high = (uint64_t)(value >> 64U);
    }
    memcpy(record + memo->key_bytes, &compact, sizeof(compact));
    index_place(memo, memo->index, memo->index_capacity, identifier);
    ++memo->size;
    return true;
}

static void increment_u64_saturating(uint64_t *value)
{
    if (*value != UINT64_MAX) {
        ++*value;
    }
}

static void report_progress(Memo *memo)
{
    if (quiet || memo->calls < memo->next_report_calls) {
        return;
    }
    fprintf(stderr,
            "291355_02: n=%u new_states=%zu total_states=%zu "
            "calls=%" PRIu64 " hits=%" PRIu64 " pruned=%" PRIu64
            " uncached=%" PRIu64 " memo=%.1f MiB%s time=%.1fs\n",
            memo->root_n, memo->size - memo->root_size, memo->size,
            memo->calls - memo->root_calls,
            memo->hits - memo->root_hits,
            memo->pruned - memo->root_pruned,
            memo->uncached - memo->root_uncached,
            (double)memo->memory_current / (1024.0 * 1024.0),
            memo->frozen ? " frozen" : "",
            monotonic_seconds() - memo->root_started);
    do {
        memo->next_report_calls += PROGRESS_INTERVAL;
    } while (memo->calls >= memo->next_report_calls);
}

/*
 * Return false only if no legal path of the requested depth exists.
 * Therefore using false to prune a branch cannot remove a solution.
 */
static bool has_continuation(unsigned cardinality, Bitset remaining,
                             uint64_t prefix_sum, unsigned depth)
{
    if (cardinality <= 1U || depth == 0U) {
        return true;
    }
    const unsigned modulus = cardinality - 1U;
    const unsigned residue = (unsigned)(prefix_sum % modulus);
    const Bitset candidates = bitset_intersection(
        remaining, residue_masks[modulus][residue]);
    for (unsigned word = 0U; word < MAX_KEY_WORDS; ++word) {
        uint64_t bits = candidates.word[word];
        while (bits != 0U) {
            const uint64_t bit = bits & (0U - bits);
            if (depth == 1U) {
                return true;
            }
            const unsigned value = word * 64U +
                (unsigned)__builtin_ctzll(bit) + 1U;
            if (has_continuation(
                    cardinality - 1U, bitset_without(remaining, value),
                    prefix_sum - cubes[value], depth - 1U)) {
                return true;
            }
            bits ^= bit;
        }
    }
    return false;
}

static U128 reverse_search(unsigned cardinality, Bitset remaining,
                           uint64_t prefix_sum, Memo *memo)
{
    increment_u64_saturating(&memo->calls);
    report_progress(memo);
    if (cardinality <= 1U) {
        return 1U;
    }

    U128 saved;
    if (memo_lookup(memo, remaining, &saved)) {
        return saved;
    }

    const unsigned modulus = cardinality - 1U;
    const unsigned residue = (unsigned)(prefix_sum % modulus);
    const Bitset candidates = bitset_intersection(
        remaining, residue_masks[modulus][residue]);
    U128 result = 0U;

    for (unsigned word = 0U; word < MAX_KEY_WORDS; ++word) {
        uint64_t bits = candidates.word[word];
        while (bits != 0U) {
            const uint64_t bit = bits & (0U - bits);
            const unsigned value = word * 64U +
                (unsigned)__builtin_ctzll(bit) + 1U;
            const Bitset child = bitset_without(remaining, value);
            const uint64_t child_sum = prefix_sum - cubes[value];
            if (lookahead_depth != 0U &&
                !has_continuation(cardinality - 1U, child, child_sum,
                                  lookahead_depth)) {
                increment_u64_saturating(&memo->pruned);
            } else {
                const U128 addition = reverse_search(
                    cardinality - 1U, child, child_sum, memo);
                checked_add(&result, addition);
            }
            bits ^= bit;
        }
    }

    (void)memo_insert(memo, remaining, result);
    return result;
}

static SearchResult count_reverse(unsigned n, Memo *memo)
{
    SearchResult result = {0U, 0U, 0U, 0U, 0U, 0U, memo->size,
                           memo->memory_peak, memo->frozen};
    if (n == 0U) {
        result.value = 1U;
        result.calls = 1U;
        return result;
    }

    const uint64_t triangular = (uint64_t)n * (n + 1U) / 2U;
    const uint64_t total = triangular * triangular;
    if (total % n != 0U) {
        result.calls = 1U;
        return result;
    }

    memo->root_n = n;
    memo->root_size = memo->size;
    memo->root_calls = memo->calls;
    memo->root_hits = memo->hits;
    memo->root_pruned = memo->pruned;
    memo->root_uncached = memo->uncached;
    memo->root_started = monotonic_seconds();
    memo->next_report_calls = memo->calls + PROGRESS_INTERVAL;

    result.value = reverse_search(n, full_set(n), total, memo);
    result.calls = memo->calls - memo->root_calls;
    result.hits = memo->hits - memo->root_hits;
    result.pruned = memo->pruned - memo->root_pruned;
    result.uncached = memo->uncached - memo->root_uncached;
    result.states_added = memo->size - memo->root_size;
    result.states_total = memo->size;
    result.memory_peak = memo->memory_peak;
    result.frozen = memo->frozen;
    return result;
}

/* Independent definition-level forward search used only by --check. */
static U128 forward_search(unsigned n, unsigned position,
                           Bitset remaining, uint64_t prefix_sum)
{
    if (position > n) {
        return 1U;
    }
    U128 result = 0U;
    for (unsigned word = 0U; word < MAX_KEY_WORDS; ++word) {
        uint64_t bits = remaining.word[word];
        while (bits != 0U) {
            const uint64_t bit = bits & (0U - bits);
            const unsigned value = word * 64U +
                (unsigned)__builtin_ctzll(bit) + 1U;
            const uint64_t next_sum = prefix_sum + cubes[value];
            if (next_sum % position == 0U) {
                checked_add(&result, forward_search(
                    n, position + 1U,
                    bitset_without(remaining, value), next_sum));
            }
            bits ^= bit;
        }
    }
    return result;
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
            (void)close(descriptor);
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
                       U128 values[MAX_N + 1U])
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
        char number[U128_TEXT_SIZE];
        char extra;
        if (sscanf(line, "%u %39s %c", &index, number, &extra) != 2 ||
            index > MAX_N || present[index] ||
            !parse_u128(number, &values[index])) {
            (void)fclose(input);
            die("b-file is malformed or contains a duplicate index");
        }
        present[index] = true;
    }
    if (ferror(input) || fclose(input) != 0) {
        die("cannot finish reading b-file");
    }
}

static bool load_saved_term(unsigned n, U128 *value)
{
    bool present[MAX_N + 1U];
    U128 values[MAX_N + 1U];
    const int lock_descriptor = lock_bfile();
    read_bfile(present, values);
    const bool found = present[n];
    if (found) {
        *value = values[n];
    }
    unlock_bfile(lock_descriptor);
    return found;
}

static int write_bfile_line(FILE *stream, unsigned n, U128 value)
{
    if (fprintf(stream, "%u ", n) < 0 || print_u128(stream, value) != 0 ||
        fputc('\n', stream) == EOF) {
        return -1;
    }
    return 0;
}

static void record_term(unsigned n, U128 value)
{
    bool present[MAX_N + 1U];
    U128 values[MAX_N + 1U];
    const int lock_descriptor = lock_bfile();
    read_bfile(present, values);
    if (present[n]) {
        if (values[n] != value) {
            unlock_bfile(lock_descriptor);
            die("computed term disagrees with the b-file");
        }
        unlock_bfile(lock_descriptor);
        return;
    }
    present[n] = true;
    values[n] = value;

    char temporary[] = "b291355_02.txt.tmp.XXXXXX";
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
            write_bfile_line(output, index, values[index]) != 0) {
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
}

static U128 obtain_term(unsigned n, Memo *memo)
{
    U128 value;
    if (load_saved_term(n, &value)) {
        if (!quiet) {
            fprintf(stderr, "291355_02: reusing saved a(%u)\n", n);
        }
        return value;
    }

    if (!quiet) {
        fprintf(stderr,
                "291355_02: n=%u compact reverse DP started "
                "(key=%u bits, lookahead=%u)\n",
                n, memo->key_bytes * 8U, lookahead_depth);
    }
    const double started = monotonic_seconds();
    const SearchResult result = count_reverse(n, memo);
    value = result.value;
    record_term(n, value);
    if (!quiet) {
        char text[U128_TEXT_SIZE];
        u128_to_text(value, text);
        fprintf(stderr,
                "291355_02: n=%u answer=%s new_states=%zu "
                "total_states=%zu calls=%" PRIu64 " hits=%" PRIu64
                " pruned=%" PRIu64 " uncached=%" PRIu64
                " memo_peak=%.1f MiB%s time=%.3fs\n",
                n, text, result.states_added, result.states_total,
                result.calls, result.hits, result.pruned, result.uncached,
                (double)result.memory_peak / (1024.0 * 1024.0),
                result.frozen ? " frozen" : "",
                monotonic_seconds() - started);
    }
    return value;
}

static void print_term(unsigned n, U128 value)
{
    if (printf("%u ", n) < 0 || print_u128(stdout, value) != 0 ||
        putchar('\n') == EOF || fflush(stdout) != 0) {
        die("cannot write standard output");
    }
}

static void check_compact_memo_storage(void)
{
    Memo memo;
    memo_init(&memo, MAX_N);
    const Bitset keys[3] = {full_set(3U), full_set(4U), full_set(5U)};
    const U128 values[3] = {
        (U128)UINT32_MAX - 1U,
        (U128)UINT32_MAX,
        ((U128)UINT64_MAX << 32U) + UINT64_C(123456789)
    };
    for (unsigned i = 0U; i < 3U; ++i) {
        if (!memo_insert(&memo, keys[i], values[i])) {
            memo_destroy(&memo);
            die("compact memo self-check insertion failed");
        }
    }
    for (unsigned i = 0U; i < 3U; ++i) {
        U128 saved;
        if (!memo_lookup(&memo, keys[i], &saved) || saved != values[i]) {
            memo_destroy(&memo);
            die("compact memo self-check lookup failed");
        }
    }
    memo_destroy(&memo);
}

static void check_implementation(unsigned maximum)
{
    const bool saved_quiet = quiet;
    const unsigned saved_lookahead = lookahead_depth;
    quiet = true;
    check_compact_memo_storage();

    Memo memo;
    memo_init(&memo, maximum);
    for (unsigned n = 0U; n <= maximum; ++n) {
        lookahead_depth = DEFAULT_LOOKAHEAD;
        const SearchResult reverse = count_reverse(n, &memo);
        const U128 forward = forward_search(n, 1U, full_set(n), 0U);
        if (reverse.value != forward || reverse.value != known_terms[n]) {
            memo_destroy(&memo);
            quiet = saved_quiet;
            lookahead_depth = saved_lookahead;
            fprintf(stderr, "error: check mismatch at n=%u\n", n);
            exit(EXIT_FAILURE);
        }
    }
    memo_destroy(&memo);
    quiet = saved_quiet;
    lookahead_depth = saved_lookahead;
    printf("ok: packed memo including 128-bit overflow, exact lookahead, "
           "shared reverse DP, forward enumeration, and known terms "
           "agree for n=0..%u\n", maximum);
}

static void usage(const char *program, FILE *stream)
{
    fprintf(stream,
            "Usage:\n"
            "  %s [MAX_N] [--quiet] [--lookahead D]\n"
            "  %s --upto MAX_N [--quiet] [--lookahead D]\n"
            "  %s --term N [--quiet] [--lookahead D]\n"
            "  %s --check [CHECK_N]\n"
            "\n"
            "MAX_N and N may be 0..%u; default MAX_N is %u.\n"
            "D may be 0..%u and defaults to %u.\n"
            "CHECK_N may be 0..%u and defaults to %u.\n"
            "Completed terms are saved atomically in %s.\n"
            "A291355_MEMORY_MIB sets the memo allocation limit "
            "(default %" PRIu64 " MiB).\n",
            program, program, program, program,
            MAX_N, DEFAULT_MAX_N, MAX_LOOKAHEAD, DEFAULT_LOOKAHEAD,
            DIRECT_CHECK_MAX_N, DIRECT_CHECK_MAX_N,
            BFILE_PATH, DEFAULT_MEMORY_MIB);
}

static U128 run_with_new_memo(unsigned n)
{
    Memo memo;
    memo_init(&memo, n);
    const U128 value = obtain_term(n, &memo);
    memo_destroy(&memo);
    return value;
}

static void run_upto(unsigned maximum)
{
    unsigned n = 0U;
    while (n <= maximum) {
        if (n >= 3U && n % 4U == 3U) {
            unsigned end = n + 2U;
            if (end > maximum) {
                end = maximum;
            }
            Memo memo;
            memo_init(&memo, end);
            for (unsigned current = n; current <= end; ++current) {
                print_term(current, obtain_term(current, &memo));
            }
            memo_destroy(&memo);
            n = end + 1U;
        } else {
            print_term(n, run_with_new_memo(n));
            ++n;
        }
    }
}

int main(int argc, char **argv)
{
    configure_memory_limit();
    prepare_tables();

    Mode mode = MODE_UPTO;
    bool mode_seen = false;
    bool n_seen = false;
    bool lookahead_seen = false;
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
        if (strcmp(argument, "--lookahead") == 0) {
            if (lookahead_seen || i + 1 >= argc) {
                usage(argv[0], stderr);
                return EXIT_FAILURE;
            }
            lookahead_depth = parse_unsigned(
                argv[++i], MAX_LOOKAHEAD, "lookahead depth");
            lookahead_seen = true;
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
        const U128 value = run_with_new_memo(n);
        if (print_u128(stdout, value) != 0 || putchar('\n') == EOF ||
            fflush(stdout) != 0) {
            die("cannot write standard output");
        }
    } else {
        run_upto(n);
    }
    return EXIT_SUCCESS;
}
