/*
 * A291355 -- reverse layered-frontier DP for n <= 150.
 *
 * Count permutations s_1,...,s_n of [n] such that
 *
 *     j divides Sum_{i=1..j} s_i^3,       1 <= j <= n.
 *
 * This is a backward search, but unlike 291355_02.c it is not a
 * recursive memoized evaluation of C(U).  It propagates path counts
 * breadth-first through cardinality layers.  A frontier state (U,w)
 * says that w legal choices of the already removed suffix lead from
 * [n] to U.  If |U|=j and P(U)=Sum_{x in U}x^3, removing x is legal iff
 *
 *     P(U)-x^3 == 0 (mod j-1).
 *
 * Equal child subsets are merged by adding their path counts.  After a
 * whole layer is generated, the old layer is freed; hence memory depends
 * on two adjacent frontier widths, not all states ever reached.  A small
 * exact continuation screen rejects only children for which no legal
 * path of the requested depth exists.
 *
 * Each frontier uses packed keys and a dense-record/hash-index map.
 * Counts are checked unsigned 128-bit integers.  If the configured
 * memory limit cannot hold two adjacent layers, the program stops with
 * an error rather than returning a partial or incorrect count.
 *
 * Since Sum_{x=1..n}x^3=(n(n+1)/2)^2, every n == 2 (mod 4) is rejected
 * before a frontier is allocated.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       291355_03.c -o 291355_03
 *
 * Examples:
 *   ./291355_03 --upto 150
 *   ./291355_03 --term 149
 *   ./291355_03 --check
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
#error "291355_03.c requires unsigned __int128"
#endif

#if SIZE_MAX < UINT64_MAX
#error "291355_03.c requires a 64-bit size_t"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 150U
#define DEFAULT_MAX_N 150U
#define DIRECT_CHECK_MAX_N 16U
#define MAX_KEY_WORDS ((MAX_N + 63U) / 64U)
#define U128_TEXT_SIZE 40U
#define BFILE_PATH "b291355_03.txt"
#define LOCK_PATH "b291355_03.txt.lock"
#define DEFAULT_MEMORY_MIB UINT64_C(5120)
#define MIN_MEMORY_MIB UINT64_C(32)
#define MAX_MEMORY_MIB UINT64_C(65536)
#define DEFAULT_SCREEN_DEPTH 2U
#define MAX_SCREEN_DEPTH 6U
#define INITIAL_INDEX_CAPACITY ((size_t)1024U)
#define INDEX_LOAD_NUMERATOR ((size_t)85U)
#define INDEX_LOAD_DENOMINATOR ((size_t)100U)
#define RECORD_CHUNK_SHIFT 18U
#define RECORDS_PER_CHUNK ((size_t)1U << RECORD_CHUNK_SHIFT)
#define RECORD_CHUNK_MASK (RECORDS_PER_CHUNK - 1U)
#define MAX_RECORD_CHUNKS ((size_t)1U << (32U - RECORD_CHUNK_SHIFT))
#define PROGRESS_INTERVAL UINT64_C(10000000)

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} Mode;

typedef struct {
    uint64_t word[MAX_KEY_WORDS];
} Bitset;

typedef struct {
    uint64_t limit;
    uint64_t current;
    uint64_t peak;
} MemoryTracker;

/* Packed record: key bytes, uint64_t prefix sum, then U128 path count. */
typedef struct {
    unsigned char **chunks;
    uint32_t *index;
    size_t index_capacity;
    size_t size;
    unsigned key_words;
    unsigned key_bytes;
    unsigned full_key_words;
    unsigned tail_key_bytes;
    size_t prefix_offset;
    size_t count_offset;
    size_t record_bytes;
    MemoryTracker *memory;
} Frontier;

typedef struct {
    U128 value;
    uint64_t states;
    uint64_t edges;
    uint64_t merged;
    uint64_t pruned;
    size_t maximum_frontier;
    uint64_t memory_peak;
} SearchResult;

static uint64_t cubes[MAX_N + 1U];
static Bitset residue_masks[MAX_N + 1U][MAX_N];
static bool tables_ready;
static bool quiet;
static uint64_t configured_memory_limit;
static unsigned screen_depth = DEFAULT_SCREEN_DEPTH;

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

static void add_u64_saturating(uint64_t *target, uint64_t addition)
{
    if (*target > UINT64_MAX - addition) {
        *target = UINT64_MAX;
    } else {
        *target += addition;
    }
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

static bool memory_fits(const MemoryTracker *memory, uint64_t bytes)
{
    return memory->current <= memory->limit &&
        bytes <= memory->limit - memory->current;
}

static void memory_add(MemoryTracker *memory, uint64_t bytes)
{
    memory->current += bytes;
    if (memory->current > memory->peak) {
        memory->peak = memory->current;
    }
}

static unsigned char *frontier_record(const Frontier *frontier,
                                      size_t identifier)
{
    const size_t chunk = identifier >> RECORD_CHUNK_SHIFT;
    const size_t offset = identifier & RECORD_CHUNK_MASK;
    return frontier->chunks[chunk] + offset * frontier->record_bytes;
}

static uint64_t record_key_word(const Frontier *frontier,
                                const unsigned char *record,
                                unsigned word)
{
    uint64_t value = 0U;
    if (word < frontier->full_key_words) {
        memcpy(&value, record + (size_t)word * sizeof(value),
               sizeof(value));
    } else {
        const size_t start =
            (size_t)frontier->full_key_words * sizeof(value);
        for (unsigned byte = 0U; byte < frontier->tail_key_bytes; ++byte) {
            value |= (uint64_t)record[start + byte] << (8U * byte);
        }
    }
    return value;
}

static Bitset record_key(const Frontier *frontier,
                         const unsigned char *record)
{
    Bitset key = {{0U}};
    for (unsigned word = 0U; word < frontier->key_words; ++word) {
        key.word[word] = record_key_word(frontier, record, word);
    }
    return key;
}

static bool record_equals(const Frontier *frontier,
                          const unsigned char *record, Bitset key)
{
    for (unsigned word = 0U; word < frontier->key_words; ++word) {
        if (record_key_word(frontier, record, word) != key.word[word]) {
            return false;
        }
    }
    return true;
}

static uint64_t record_hash(const Frontier *frontier,
                            const unsigned char *record)
{
    uint64_t words[MAX_KEY_WORDS];
    for (unsigned word = 0U; word < frontier->key_words; ++word) {
        words[word] = record_key_word(frontier, record, word);
    }
    return hash_words(words, frontier->key_words);
}

static uint64_t record_prefix_sum(const Frontier *frontier,
                                  const unsigned char *record)
{
    uint64_t value;
    memcpy(&value, record + frontier->prefix_offset, sizeof(value));
    return value;
}

static U128 record_count(const Frontier *frontier,
                         const unsigned char *record)
{
    U128 value;
    memcpy(&value, record + frontier->count_offset, sizeof(value));
    return value;
}

static void record_set_count(const Frontier *frontier,
                             unsigned char *record, U128 value)
{
    memcpy(record + frontier->count_offset, &value, sizeof(value));
}

static void record_store_key(const Frontier *frontier,
                             unsigned char *record, Bitset key)
{
    for (unsigned word = 0U; word < frontier->full_key_words; ++word) {
        memcpy(record + (size_t)word * sizeof(uint64_t),
               &key.word[word], sizeof(uint64_t));
    }
    const size_t start =
        (size_t)frontier->full_key_words * sizeof(uint64_t);
    if (frontier->tail_key_bytes != 0U) {
        const uint64_t tail = key.word[frontier->full_key_words];
        for (unsigned byte = 0U; byte < frontier->tail_key_bytes; ++byte) {
            record[start + byte] = (unsigned char)(tail >> (8U * byte));
        }
    }
}

static void index_place(const Frontier *frontier, uint32_t *index,
                        size_t capacity, size_t identifier)
{
    const unsigned char *record = frontier_record(frontier, identifier);
    const uint64_t hash = record_hash(frontier, record);
    size_t position = (size_t)hash & (capacity - 1U);
    const size_t step = ((size_t)(hash >> 32U) | 1U) & (capacity - 1U);
    while (index[position] != 0U) {
        position = (position + step) & (capacity - 1U);
    }
    index[position] = (uint32_t)(identifier + 1U);
}

static bool frontier_grow_index(Frontier *frontier)
{
    if (frontier->index_capacity > SIZE_MAX / 2U) {
        return false;
    }
    const size_t new_capacity = frontier->index_capacity * 2U;
    if (new_capacity > SIZE_MAX / sizeof(*frontier->index) ||
        new_capacity > UINT64_MAX / sizeof(*frontier->index)) {
        return false;
    }
    const uint64_t old_bytes =
        (uint64_t)frontier->index_capacity * sizeof(*frontier->index);
    const uint64_t new_bytes =
        (uint64_t)new_capacity * sizeof(*frontier->index);
    MemoryTracker *memory = frontier->memory;
    if (memory->current < old_bytes ||
        new_bytes > memory->limit - (memory->current - old_bytes)) {
        return false;
    }
    uint32_t *resized = realloc(frontier->index, (size_t)new_bytes);
    if (resized == NULL) {
        return false;
    }
    frontier->index = resized;
    frontier->index_capacity = new_capacity;
    memory->current = memory->current - old_bytes + new_bytes;
    if (memory->current > memory->peak) {
        memory->peak = memory->current;
    }
    memset(frontier->index, 0, (size_t)new_bytes);
    for (size_t identifier = 0U; identifier < frontier->size;
         ++identifier) {
        index_place(frontier, frontier->index,
                    frontier->index_capacity, identifier);
    }
    return true;
}

static bool frontier_allocate_chunk(Frontier *frontier)
{
    const size_t chunk_index = frontier->size >> RECORD_CHUNK_SHIFT;
    if (chunk_index >= MAX_RECORD_CHUNKS ||
        frontier->record_bytes >
            UINT64_MAX / (uint64_t)RECORDS_PER_CHUNK) {
        return false;
    }
    const uint64_t bytes =
        (uint64_t)RECORDS_PER_CHUNK * frontier->record_bytes;
    if (!memory_fits(frontier->memory, bytes)) {
        return false;
    }
    unsigned char *chunk = malloc((size_t)bytes);
    if (chunk == NULL) {
        return false;
    }
    frontier->chunks[chunk_index] = chunk;
    memory_add(frontier->memory, bytes);
    return true;
}

static void frontier_init(Frontier *frontier, unsigned n,
                          MemoryTracker *memory)
{
    memset(frontier, 0, sizeof(*frontier));
    frontier->key_words = words_for_n(n);
    frontier->key_bytes = (n + 7U) / 8U;
    if (frontier->key_bytes == 0U) {
        frontier->key_bytes = 1U;
    }
    frontier->full_key_words =
        frontier->key_bytes / sizeof(uint64_t);
    frontier->tail_key_bytes =
        frontier->key_bytes % sizeof(uint64_t);
    frontier->prefix_offset = frontier->key_bytes;
    frontier->count_offset =
        frontier->prefix_offset + sizeof(uint64_t);
    frontier->record_bytes =
        frontier->count_offset + sizeof(U128);
    frontier->index_capacity = INITIAL_INDEX_CAPACITY;
    frontier->memory = memory;

    const uint64_t pointer_bytes =
        (uint64_t)MAX_RECORD_CHUNKS * sizeof(*frontier->chunks);
    const uint64_t index_bytes =
        (uint64_t)frontier->index_capacity * sizeof(*frontier->index);
    if (!memory_fits(memory, pointer_bytes + index_bytes)) {
        die("memory limit is too small for two frontier maps");
    }
    frontier->chunks =
        calloc(MAX_RECORD_CHUNKS, sizeof(*frontier->chunks));
    frontier->index =
        calloc(frontier->index_capacity, sizeof(*frontier->index));
    if (frontier->chunks == NULL || frontier->index == NULL) {
        free(frontier->chunks);
        free(frontier->index);
        die("cannot allocate initial frontier structures");
    }
    memory_add(memory, pointer_bytes + index_bytes);
}

static void frontier_destroy(Frontier *frontier)
{
    if (frontier->chunks == NULL) {
        return;
    }
    const size_t chunk_count =
        (frontier->size + RECORDS_PER_CHUNK - 1U) >> RECORD_CHUNK_SHIFT;
    const uint64_t chunk_bytes =
        (uint64_t)RECORDS_PER_CHUNK * frontier->record_bytes;
    for (size_t chunk = 0U; chunk < chunk_count; ++chunk) {
        free(frontier->chunks[chunk]);
    }
    const uint64_t pointer_bytes =
        (uint64_t)MAX_RECORD_CHUNKS * sizeof(*frontier->chunks);
    const uint64_t index_bytes =
        (uint64_t)frontier->index_capacity * sizeof(*frontier->index);
    const uint64_t released =
        pointer_bytes + index_bytes + (uint64_t)chunk_count * chunk_bytes;
    if (frontier->memory->current < released) {
        die("internal frontier memory accounting failure");
    }
    frontier->memory->current -= released;
    free(frontier->chunks);
    free(frontier->index);
    memset(frontier, 0, sizeof(*frontier));
}

static bool frontier_find(const Frontier *frontier, Bitset key,
                          size_t *identifier)
{
    const uint64_t hash = hash_words(key.word, frontier->key_words);
    size_t position =
        (size_t)hash & (frontier->index_capacity - 1U);
    const size_t step = ((size_t)(hash >> 32U) | 1U) &
        (frontier->index_capacity - 1U);
    for (;;) {
        const uint32_t tag = frontier->index[position];
        if (tag == 0U) {
            return false;
        }
        const size_t saved_identifier = (size_t)tag - 1U;
        const unsigned char *record =
            frontier_record(frontier, saved_identifier);
        if (record_equals(frontier, record, key)) {
            *identifier = saved_identifier;
            return true;
        }
        position = (position + step) & (frontier->index_capacity - 1U);
    }
}

static bool frontier_add(Frontier *frontier, Bitset key,
                         uint64_t prefix_sum, U128 addition)
{
    size_t identifier;
    if (frontier_find(frontier, key, &identifier)) {
        unsigned char *record = frontier_record(frontier, identifier);
        if (record_prefix_sum(frontier, record) != prefix_sum) {
            die("internal prefix-sum inconsistency");
        }
        U128 value = record_count(frontier, record);
        checked_add(&value, addition);
        record_set_count(frontier, record, value);
        return true;
    }

    if (frontier->size >= (size_t)UINT32_MAX - 1U) {
        die("frontier exceeds the uint32_t index range");
    }
    if (frontier->size + 1U >
        frontier->index_capacity * INDEX_LOAD_NUMERATOR /
            INDEX_LOAD_DENOMINATOR &&
        !frontier_grow_index(frontier)) {
        die("two frontier layers exceed A291355_MEMORY_MIB");
    }
    if ((frontier->size & RECORD_CHUNK_MASK) == 0U &&
        !frontier_allocate_chunk(frontier)) {
        die("two frontier layers exceed A291355_MEMORY_MIB");
    }

    identifier = frontier->size;
    unsigned char *record = frontier_record(frontier, identifier);
    record_store_key(frontier, record, key);
    memcpy(record + frontier->prefix_offset,
           &prefix_sum, sizeof(prefix_sum));
    record_set_count(frontier, record, addition);
    index_place(frontier, frontier->index,
                frontier->index_capacity, identifier);
    ++frontier->size;
    return false;
}

/* False means that no legal path of exactly the tested prefix can exist. */
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

static void report_layer_progress(unsigned n, unsigned cardinality,
                                  size_t processed, size_t total,
                                  const Frontier *next, uint64_t edges,
                                  uint64_t pruned,
                                  const MemoryTracker *memory,
                                  double started)
{
    if (quiet) {
        return;
    }
    fprintf(stderr,
            "291355_03: n=%u j=%u processed=%zu/%zu next=%zu "
            "edges=%" PRIu64 " pruned=%" PRIu64
            " memory=%.1f MiB time=%.1fs\n",
            n, cardinality, processed, total, next->size,
            edges, pruned,
            (double)memory->current / (1024.0 * 1024.0),
            monotonic_seconds() - started);
}

static SearchResult count_layered(unsigned n)
{
    SearchResult result = {0U, 0U, 0U, 0U, 0U, 0U, 0U};
    if (n == 0U) {
        result.value = 1U;
        result.states = 1U;
        return result;
    }

    const uint64_t triangular = (uint64_t)n * (n + 1U) / 2U;
    const uint64_t total_sum = triangular * triangular;
    if (total_sum % n != 0U) {
        return result;
    }

    MemoryTracker memory = {
        .limit = configured_memory_limit,
        .current = 0U,
        .peak = 0U
    };
    Frontier current;
    frontier_init(&current, n, &memory);
    (void)frontier_add(&current, full_set(n), total_sum, 1U);
    result.states = 1U;
    result.maximum_frontier = 1U;
    const double search_started = monotonic_seconds();

    unsigned cardinality = n;
    while (cardinality > 1U && current.size != 0U) {
        Frontier next;
        frontier_init(&next, n, &memory);
        uint64_t layer_edges = 0U;
        uint64_t layer_merged = 0U;
        uint64_t layer_pruned = 0U;
        uint64_t next_report = PROGRESS_INTERVAL;
        const double layer_started = monotonic_seconds();

        for (size_t identifier = 0U; identifier < current.size;
             ++identifier) {
            const unsigned char *record =
                frontier_record(&current, identifier);
            const Bitset remaining = record_key(&current, record);
            const uint64_t prefix_sum =
                record_prefix_sum(&current, record);
            const U128 ways = record_count(&current, record);
            const unsigned modulus = cardinality - 1U;
            const unsigned residue = (unsigned)(prefix_sum % modulus);
            const Bitset candidates = bitset_intersection(
                remaining, residue_masks[modulus][residue]);

            for (unsigned word = 0U; word < MAX_KEY_WORDS; ++word) {
                uint64_t bits = candidates.word[word];
                while (bits != 0U) {
                    const uint64_t bit = bits & (0U - bits);
                    const unsigned value = word * 64U +
                        (unsigned)__builtin_ctzll(bit) + 1U;
                    const Bitset child =
                        bitset_without(remaining, value);
                    const uint64_t child_sum =
                        prefix_sum - cubes[value];
                    if (screen_depth != 0U &&
                        !has_continuation(cardinality - 1U, child,
                                          child_sum, screen_depth)) {
                        add_u64_saturating(&layer_pruned, 1U);
                    } else {
                        const bool merged =
                            frontier_add(&next, child, child_sum, ways);
                        add_u64_saturating(&layer_edges, 1U);
                        if (merged) {
                            add_u64_saturating(&layer_merged, 1U);
                        }
                    }
                    bits ^= bit;
                }
            }

            const uint64_t work = layer_edges + layer_pruned;
            if (!quiet && work >= next_report) {
                report_layer_progress(n, cardinality, identifier + 1U,
                                      current.size, &next, layer_edges,
                                      layer_pruned, &memory,
                                      search_started);
                do {
                    next_report += PROGRESS_INTERVAL;
                } while (work >= next_report &&
                         next_report <= UINT64_MAX - PROGRESS_INTERVAL);
            }
        }

        add_u64_saturating(&result.edges, layer_edges);
        add_u64_saturating(&result.merged, layer_merged);
        add_u64_saturating(&result.pruned, layer_pruned);
        add_u64_saturating(&result.states, (uint64_t)next.size);
        if (next.size > result.maximum_frontier) {
            result.maximum_frontier = next.size;
        }
        if (!quiet && (monotonic_seconds() - layer_started >= 0.1 ||
                       cardinality % 10U == 0U)) {
            report_layer_progress(n, cardinality, current.size,
                                  current.size, &next, layer_edges,
                                  layer_pruned, &memory, search_started);
        }

        frontier_destroy(&current);
        current = next;
        --cardinality;
    }

    U128 answer = 0U;
    if (cardinality <= 1U) {
        for (size_t identifier = 0U; identifier < current.size;
             ++identifier) {
            checked_add(&answer, record_count(
                &current, frontier_record(&current, identifier)));
        }
    }
    result.value = answer;
    result.memory_peak = memory.peak;
    frontier_destroy(&current);
    if (memory.current != 0U) {
        die("internal nonzero frontier memory balance");
    }
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

    char temporary[] = "b291355_03.txt.tmp.XXXXXX";
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

static U128 obtain_term(unsigned n)
{
    U128 value;
    if (load_saved_term(n, &value)) {
        if (!quiet) {
            fprintf(stderr, "291355_03: reusing saved a(%u)\n", n);
        }
        return value;
    }
    if (!quiet) {
        fprintf(stderr,
                "291355_03: n=%u reverse layered DP started "
                "(screen=%u)\n", n, screen_depth);
    }
    const double started = monotonic_seconds();
    const SearchResult result = count_layered(n);
    value = result.value;
    record_term(n, value);
    if (!quiet) {
        char text[U128_TEXT_SIZE];
        u128_to_text(value, text);
        fprintf(stderr,
                "291355_03: n=%u answer=%s states=%" PRIu64
                " edges=%" PRIu64 " merged=%" PRIu64
                " pruned=%" PRIu64 " max_frontier=%zu "
                "memory_peak=%.1f MiB time=%.3fs\n",
                n, text, result.states, result.edges, result.merged,
                result.pruned, result.maximum_frontier,
                (double)result.memory_peak / (1024.0 * 1024.0),
                monotonic_seconds() - started);
    }
    return value;
}

static void check_implementation(unsigned maximum)
{
    const bool saved_quiet = quiet;
    quiet = true;
    for (unsigned n = 0U; n <= maximum; ++n) {
        const SearchResult layered = count_layered(n);
        const U128 forward = forward_search(n, 1U, full_set(n), 0U);
        if (layered.value != forward || layered.value != known_terms[n]) {
            quiet = saved_quiet;
            fprintf(stderr, "error: check mismatch at n=%u\n", n);
            exit(EXIT_FAILURE);
        }
    }
    quiet = saved_quiet;
    printf("ok: reverse layered-frontier DP, exact continuation screen, "
           "forward enumeration, and known terms agree for n=0..%u\n",
           maximum);
}

static void usage(const char *program, FILE *stream)
{
    fprintf(stream,
            "Usage:\n"
            "  %s [MAX_N] [--quiet] [--screen D]\n"
            "  %s --upto MAX_N [--quiet] [--screen D]\n"
            "  %s --term N [--quiet] [--screen D]\n"
            "  %s --check [CHECK_N]\n"
            "\n"
            "MAX_N and N may be 0..%u; default MAX_N is %u.\n"
            "D may be 0..%u and defaults to %u.\n"
            "CHECK_N may be 0..%u and defaults to %u.\n"
            "Completed terms are saved atomically in %s.\n"
            "A291355_MEMORY_MIB sets the two-frontier memory limit "
            "(default %" PRIu64 " MiB).\n",
            program, program, program, program,
            MAX_N, DEFAULT_MAX_N,
            MAX_SCREEN_DEPTH, DEFAULT_SCREEN_DEPTH,
            DIRECT_CHECK_MAX_N, DIRECT_CHECK_MAX_N,
            BFILE_PATH, DEFAULT_MEMORY_MIB);
}

int main(int argc, char **argv)
{
    configure_memory_limit();
    prepare_tables();

    Mode mode = MODE_UPTO;
    bool mode_seen = false;
    bool n_seen = false;
    bool screen_seen = false;
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
        if (strcmp(argument, "--screen") == 0) {
            if (screen_seen || i + 1 >= argc) {
                usage(argv[0], stderr);
                return EXIT_FAILURE;
            }
            screen_depth = parse_unsigned(
                argv[++i], MAX_SCREEN_DEPTH, "screen depth");
            screen_seen = true;
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
        const U128 value = obtain_term(n);
        if (print_u128(stdout, value) != 0 || putchar('\n') == EOF ||
            fflush(stdout) != 0) {
            die("cannot write standard output");
        }
    } else {
        for (unsigned index = 0U; index <= n; ++index) {
            const U128 value = obtain_term(index);
            if (write_bfile_line(stdout, index, value) != 0 ||
                fflush(stdout) != 0) {
                die("cannot write standard output");
            }
        }
    }
    return EXIT_SUCCESS;
}
