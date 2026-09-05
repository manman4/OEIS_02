/*
 * A291519 -- reverse sparse subset DP.
 *
 * Count permutations s_1,...,s_n of [n], with s_n=1 for n>0, such that
 * for every prefix
 *
 *     Sum_{i=1..j} s_i  divides  Sum_{i=1..j} s_i^3.
 *
 * Fixing s_n=1 leaves U={2,...,n} in the first n-1 positions.  For any
 * nonempty subset U put
 *
 *     S(U)=Sum_{x in U}x,       Q(U)=Sum_{x in U}x^3.
 *
 * U may occur as a prefix exactly when S(U) divides Q(U).  If U is good,
 * its valid orders satisfy
 *
 *     C(U) = Sum C(U-{x}),
 *
 * over x for which U-{x} is good, with C(U)=1 for |U|<=1.  The subset U
 * alone determines its cardinality, S(U), and Q(U), so it is a complete
 * memoization key.
 *
 * For a good U and d=S(U)-x,
 *
 *     Q(U)-x^3 == Q(U)-S(U)^3 (mod d),
 *
 * because x == S(U) (mod d).  Thus a child is good exactly when d divides
 * S(U)^3-Q(U).  This tests every possible last element with one modulus.
 *
 * Reachable subsets are stored in a sparse exact hash table.  Dense state
 * records are separate from a uint32_t hash index, so index growth does not
 * duplicate all keys and count values.  In --upto mode one memo is shared
 * by all n: C(U) is independent of the ambient [n].  If the configured memo
 * limit is reached, insertion stops and uncached states are recomputed; this
 * can slow the search but cannot change the answer.
 *
 * Counts use checked unsigned 384-bit arithmetic.  The supported range is
 * n=0..300; this is needed because the terms exceed 256 bits above this
 * program's former range.  Any unexpected overflow stops safely instead of
 * wrapping.
 * Completed terms are saved atomically in b291519_01.txt.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       291519_01.c -o 291519_01
 *
 * Examples:
 *   ./291519_01 --upto 120
 *   ./291519_01 --term 120
 *   ./291519_01 --check
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
#error "291519_01.c requires unsigned __int128"
#endif

#if SIZE_MAX < UINT64_MAX
#error "291519_01.c requires a 64-bit size_t"
#endif

__extension__ typedef unsigned __int128 Wide;

#define MAX_N 300U
#define DEFAULT_MAX_N 300U
#define DIRECT_CHECK_MAX_N 16U
#define BITSET_WORDS ((MAX_N + 63U) / 64U)
#define COUNT_LIMBS 6U
#define COUNT_TEXT_SIZE 120U
#define BFILE_PATH "b291519_01.txt"
#define LOCK_PATH "b291519_01.txt.lock"
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
#define PROGRESS_INTERVAL UINT64_C(10000000)

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} Mode;

typedef struct {
    uint64_t word[BITSET_WORDS];
} Bitset;

typedef struct {
    uint64_t limb[COUNT_LIMBS];
} Count;

typedef struct {
    uint64_t word[BITSET_WORDS];
    Count value;
} MemoRecord;

typedef struct {
    MemoRecord **chunks;
    uint32_t *index;
    size_t index_capacity;
    size_t size;
    uint64_t memory_limit;
    uint64_t memory_current;
    uint64_t memory_peak;
    uint64_t calls;
    uint64_t hits;
    uint64_t branches;
    uint64_t uncached;
    uint64_t root_calls;
    uint64_t root_hits;
    uint64_t root_branches;
    uint64_t root_uncached;
    size_t root_size;
    uint64_t next_report_calls;
    unsigned root_n;
    double root_started;
    bool frozen;
    bool freeze_reported;
} Memo;

typedef struct {
    Count value;
    uint64_t calls;
    uint64_t hits;
    uint64_t branches;
    uint64_t uncached;
    size_t states_added;
    size_t states_total;
    uint64_t memory_peak;
    bool frozen;
} SearchResult;

static uint64_t cubes[MAX_N + 1U];
static bool quiet;
static uint64_t configured_memory_limit;

static const uint64_t known_terms[DIRECT_CHECK_MAX_N + 1U] = {
    1U, 1U, 1U, 2U, 6U, 18U, 42U, 90U, 228U, 498U, 1152U,
    2274U, 5460U, 10308U, 20868U, 39222U, 78126U
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
    const char *text = getenv("A291519_MEMORY_MIB");
    if (text != NULL && *text != '\0') {
        char *end = NULL;
        errno = 0;
        const unsigned long long parsed = strtoull(text, &end, 10);
        if (errno != 0 || end == text || *end != '\0' ||
            parsed < MIN_MEMORY_MIB || parsed > MAX_MEMORY_MIB) {
            fprintf(stderr,
                    "error: A291519_MEMORY_MIB must be in %" PRIu64
                    "..%" PRIu64 ": %s\n",
                    MIN_MEMORY_MIB, MAX_MEMORY_MIB, text);
            exit(EXIT_FAILURE);
        }
        mib = (uint64_t)parsed;
    }
    configured_memory_limit = mib << 20U;
}

static void prepare_cubes(void)
{
    for (unsigned value = 1U; value <= MAX_N; ++value) {
        cubes[value] = (uint64_t)value * value * value;
    }
}

static Count count_from_u64(uint64_t value)
{
    Count result = {{value}};
    return result;
}

static bool counts_equal(Count left, Count right)
{
    for (unsigned limb = 0U; limb < COUNT_LIMBS; ++limb) {
        if (left.limb[limb] != right.limb[limb]) {
            return false;
        }
    }
    return true;
}

static void checked_add(Count *target, Count addition)
{
    uint64_t carry = 0U;
    for (unsigned limb = 0U; limb < COUNT_LIMBS; ++limb) {
        const uint64_t first = target->limb[limb] + addition.limb[limb];
        const uint64_t carry_first = first < target->limb[limb];
        const uint64_t second = first + carry;
        const uint64_t carry_second = second < first;
        target->limb[limb] = second;
        carry = carry_first | carry_second;
    }
    if (carry != 0U) {
        die("unsigned 384-bit count overflow");
    }
}

static void increment_u64_saturating(uint64_t *value)
{
    if (*value != UINT64_MAX) {
        ++*value;
    }
}

static bool count_is_zero(Count value)
{
    for (unsigned limb = 0U; limb < COUNT_LIMBS; ++limb) {
        if (value.limb[limb] != 0U) {
            return false;
        }
    }
    return true;
}

static unsigned count_divide_small(Count *value, unsigned divisor)
{
    unsigned remainder = 0U;
    for (unsigned limb = COUNT_LIMBS; limb-- > 0U;) {
        const Wide current = ((Wide)remainder << 64U) | value->limb[limb];
        value->limb[limb] = (uint64_t)(current / divisor);
        remainder = (unsigned)(current % divisor);
    }
    return remainder;
}

static void count_to_text(Count value, char text[COUNT_TEXT_SIZE])
{
    char reverse[COUNT_TEXT_SIZE];
    size_t length = 0U;
    do {
        if (length + 1U >= COUNT_TEXT_SIZE) {
            die("internal decimal conversion overflow");
        }
        reverse[length++] =
            (char)('0' + count_divide_small(&value, 10U));
    } while (!count_is_zero(value));
    for (size_t i = 0U; i < length; ++i) {
        text[i] = reverse[length - 1U - i];
    }
    text[length] = '\0';
}

static int print_count(FILE *stream, Count value)
{
    char text[COUNT_TEXT_SIZE];
    count_to_text(value, text);
    return fputs(text, stream) == EOF ? -1 : 0;
}

static bool parse_count(const char *text, Count *result)
{
    Count value = {{0U}};
    if (*text == '\0') {
        return false;
    }
    while (*text != '\0') {
        if (*text < '0' || *text > '9') {
            return false;
        }
        Wide carry = (unsigned)(*text++ - '0');
        for (unsigned limb = 0U; limb < COUNT_LIMBS; ++limb) {
            const Wide product = (Wide)value.limb[limb] * 10U + carry;
            value.limb[limb] = (uint64_t)product;
            carry = product >> 64U;
        }
        if (carry != 0U) {
            return false;
        }
    }
    *result = value;
    return true;
}

static Bitset full_set_without_one(unsigned n)
{
    Bitset result = {{0U}};
    for (unsigned value = 2U; value <= n; ++value) {
        const unsigned bit_index = value - 1U;
        result.word[bit_index / 64U] |=
            UINT64_C(1) << (bit_index % 64U);
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

static uint64_t mix64(uint64_t value)
{
    value ^= value >> 30U;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27U;
    value *= UINT64_C(0x94d049bb133111eb);
    value ^= value >> 31U;
    return value;
}

static uint64_t memo_hash(Bitset key)
{
    uint64_t hash = UINT64_C(0x6a09e667f3bcc909);
    for (unsigned word = 0U; word < BITSET_WORDS; ++word) {
        hash ^= mix64(key.word[word] +
                      UINT64_C(0x9e3779b97f4a7c15) * (word + 1U));
        hash = (hash << 17U | hash >> 47U) *
            UINT64_C(0x9e3779b97f4a7c15);
    }
    return hash;
}

static bool keys_equal(Bitset key, const MemoRecord *record)
{
    for (unsigned word = 0U; word < BITSET_WORDS; ++word) {
        if (key.word[word] != record->word[word]) {
            return false;
        }
    }
    return true;
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

static MemoRecord *memo_record(const Memo *memo, size_t identifier)
{
    const size_t chunk = identifier >> RECORD_CHUNK_SHIFT;
    const size_t offset = identifier & RECORD_CHUNK_MASK;
    return &memo->chunks[chunk][offset];
}

static void index_place(const Memo *memo, uint32_t *index,
                        size_t capacity, size_t identifier)
{
    const MemoRecord *record = memo_record(memo, identifier);
    Bitset key;
    for (unsigned word = 0U; word < BITSET_WORDS; ++word) {
        key.word[word] = record->word[word];
    }
    const uint64_t hash = memo_hash(key);
    size_t position = (size_t)hash & (capacity - 1U);
    const size_t step = ((size_t)(hash >> 32U) | 1U) & (capacity - 1U);
    while (index[position] != 0U) {
        position = (position + step) & (capacity - 1U);
    }
    index[position] = (uint32_t)(identifier + 1U);
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
    const uint64_t old_bytes =
        (uint64_t)memo->index_capacity * sizeof(*memo->index);
    const uint64_t new_bytes =
        (uint64_t)new_capacity * sizeof(*memo->index);
    if (memo->memory_current < old_bytes ||
        new_bytes > memo->memory_limit -
            (memo->memory_current - old_bytes)) {
        return false;
    }
    uint32_t *resized = realloc(memo->index, (size_t)new_bytes);
    if (resized == NULL) {
        return false;
    }
    memo->index = resized;
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
    if (chunk_index >= MAX_RECORD_CHUNKS ||
        sizeof(MemoRecord) >
            UINT64_MAX / (uint64_t)RECORDS_PER_CHUNK) {
        return false;
    }
    const uint64_t bytes =
        (uint64_t)RECORDS_PER_CHUNK * sizeof(MemoRecord);
    if (!memory_fits(memo, bytes)) {
        return false;
    }
    MemoRecord *chunk = malloc((size_t)bytes);
    if (chunk == NULL) {
        return false;
    }
    memo->chunks[chunk_index] = chunk;
    account_allocation(memo, bytes);
    return true;
}

static void memo_init(Memo *memo)
{
    memset(memo, 0, sizeof(*memo));
    memo->memory_limit = configured_memory_limit;
    memo->index_capacity = INITIAL_INDEX_CAPACITY;
    const uint64_t pointer_bytes =
        (uint64_t)MAX_RECORD_CHUNKS * sizeof(*memo->chunks);
    const uint64_t index_bytes =
        (uint64_t)memo->index_capacity * sizeof(*memo->index);
    if (pointer_bytes + index_bytes > memo->memory_limit) {
        die("A291519_MEMORY_MIB is too small for the initial memo");
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
    for (size_t chunk = 0U; chunk < chunk_count; ++chunk) {
        free(memo->chunks[chunk]);
    }
    free(memo->chunks);
    free(memo->index);
    memset(memo, 0, sizeof(*memo));
}

static bool memo_lookup(Memo *memo, Bitset key, Count *value)
{
    const uint64_t hash = memo_hash(key);
    size_t position =
        (size_t)hash & (memo->index_capacity - 1U);
    const size_t step = ((size_t)(hash >> 32U) | 1U) &
        (memo->index_capacity - 1U);
    for (;;) {
        const uint32_t tag = memo->index[position];
        if (tag == 0U) {
            return false;
        }
        const size_t identifier = (size_t)tag - 1U;
        const MemoRecord *record = memo_record(memo, identifier);
        if (keys_equal(key, record)) {
            *value = record->value;
            increment_u64_saturating(&memo->hits);
            return true;
        }
        position = (position + step) & (memo->index_capacity - 1U);
    }
}

static void memo_report_freeze(Memo *memo)
{
    if (!quiet && !memo->freeze_reported) {
        fprintf(stderr,
                "291519_01: n=%u memo frozen at %zu states, %.1f MiB; "
                "exact search continues with recomputation\n",
                memo->root_n, memo->size,
                (double)memo->memory_current / (1024.0 * 1024.0));
        memo->freeze_reported = true;
    }
}

static bool memo_insert(Memo *memo, Bitset key, Count value)
{
    if (memo->frozen) {
        increment_u64_saturating(&memo->uncached);
        return false;
    }
    if (memo->size >= (size_t)UINT32_MAX - 1U) {
        memo->frozen = true;
        memo_report_freeze(memo);
        increment_u64_saturating(&memo->uncached);
        return false;
    }
    if (memo->size + 1U >
        memo->index_capacity * INDEX_LOAD_NUMERATOR /
            INDEX_LOAD_DENOMINATOR &&
        !memo_grow_index(memo)) {
        memo->frozen = true;
        memo_report_freeze(memo);
        increment_u64_saturating(&memo->uncached);
        return false;
    }
    if ((memo->size & RECORD_CHUNK_MASK) == 0U &&
        !memo_allocate_chunk(memo)) {
        memo->frozen = true;
        memo_report_freeze(memo);
        increment_u64_saturating(&memo->uncached);
        return false;
    }

    const size_t identifier = memo->size;
    MemoRecord *record = memo_record(memo, identifier);
    for (unsigned word = 0U; word < BITSET_WORDS; ++word) {
        record->word[word] = key.word[word];
    }
    record->value = value;
    index_place(memo, memo->index, memo->index_capacity, identifier);
    ++memo->size;
    return true;
}

static void report_progress(Memo *memo)
{
    if (quiet || memo->calls < memo->next_report_calls) {
        return;
    }
    fprintf(stderr,
            "291519_01: n=%u new_states=%zu total_states=%zu "
            "calls=%" PRIu64 " hits=%" PRIu64 " branches=%" PRIu64
            " uncached=%" PRIu64 " memo=%.1f MiB%s time=%.1fs\n",
            memo->root_n, memo->size - memo->root_size, memo->size,
            memo->calls - memo->root_calls,
            memo->hits - memo->root_hits,
            memo->branches - memo->root_branches,
            memo->uncached - memo->root_uncached,
            (double)memo->memory_current / (1024.0 * 1024.0),
            memo->frozen ? " frozen" : "",
            monotonic_seconds() - memo->root_started);
    do {
        memo->next_report_calls += PROGRESS_INTERVAL;
    } while (memo->calls >= memo->next_report_calls);
}

static Count reverse_search(unsigned cardinality, Bitset remaining,
                            uint64_t sum, uint64_t cube_sum, Memo *memo)
{
    increment_u64_saturating(&memo->calls);
    report_progress(memo);
    if (cardinality <= 1U) {
        return count_from_u64(1U);
    }

    Count saved;
    if (memo_lookup(memo, remaining, &saved)) {
        return saved;
    }

    const uint64_t sum_cubed = sum * sum * sum;
    if (sum_cubed < cube_sum) {
        die("internal subset-sum invariant failure");
    }
    const uint64_t difference = sum_cubed - cube_sum;
    Count result = {{0U}};

    for (unsigned word = 0U; word < BITSET_WORDS; ++word) {
        uint64_t bits = remaining.word[word];
        while (bits != 0U) {
            const uint64_t bit = bits & (0U - bits);
            const unsigned value = word * 64U +
                (unsigned)__builtin_ctzll(bit) + 1U;
            const uint64_t child_sum = sum - value;
            if (difference % child_sum == 0U) {
                increment_u64_saturating(&memo->branches);
                const Count addition = reverse_search(
                    cardinality - 1U,
                    bitset_without(remaining, value), child_sum,
                    cube_sum - cubes[value], memo);
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
    SearchResult result = {{{0U}}, 0U, 0U, 0U, 0U, 0U, memo->size,
                           memo->memory_peak, memo->frozen};
    if (n == 0U || n == 1U) {
        result.value = count_from_u64(1U);
        result.calls = 1U;
        return result;
    }

    const uint64_t total_sum = (uint64_t)n * (n + 1U) / 2U;
    const uint64_t total_cube_sum = total_sum * total_sum;
    const uint64_t root_sum = total_sum - 1U;
    const uint64_t root_cube_sum = total_cube_sum - 1U;
    if (root_cube_sum % root_sum != 0U) {
        result.calls = 1U;
        return result;
    }

    memo->root_n = n;
    memo->root_size = memo->size;
    memo->root_calls = memo->calls;
    memo->root_hits = memo->hits;
    memo->root_branches = memo->branches;
    memo->root_uncached = memo->uncached;
    memo->next_report_calls = memo->calls + PROGRESS_INTERVAL;
    memo->root_started = monotonic_seconds();

    result.value = reverse_search(
        n - 1U, full_set_without_one(n), root_sum, root_cube_sum, memo);
    result.calls = memo->calls - memo->root_calls;
    result.hits = memo->hits - memo->root_hits;
    result.branches = memo->branches - memo->root_branches;
    result.uncached = memo->uncached - memo->root_uncached;
    result.states_added = memo->size - memo->root_size;
    result.states_total = memo->size;
    result.memory_peak = memo->memory_peak;
    result.frozen = memo->frozen;
    return result;
}

/* Independent definition-level forward enumeration used by --check. */
static Count forward_search(unsigned n, unsigned position,
                            Bitset remaining, uint64_t sum,
                            uint64_t cube_sum)
{
    if (position == n) {
        return count_from_u64(1U);
    }
    Count result = {{0U}};
    for (unsigned word = 0U; word < BITSET_WORDS; ++word) {
        uint64_t bits = remaining.word[word];
        while (bits != 0U) {
            const uint64_t bit = bits & (0U - bits);
            const unsigned value = word * 64U +
                (unsigned)__builtin_ctzll(bit) + 1U;
            const uint64_t next_sum = sum + value;
            const uint64_t next_cube_sum = cube_sum + cubes[value];
            if (next_cube_sum % next_sum == 0U) {
                checked_add(&result, forward_search(
                    n, position + 1U,
                    bitset_without(remaining, value),
                    next_sum, next_cube_sum));
            }
            bits ^= bit;
        }
    }
    return result;
}

static Count count_forward(unsigned n)
{
    if (n == 0U) {
        return count_from_u64(1U);
    }
    return forward_search(n, 1U, full_set_without_one(n), 0U, 0U);
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
                       Count values[MAX_N + 1U])
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
        char number[COUNT_TEXT_SIZE];
        char extra;
        if (sscanf(line, "%u %119s %c", &index, number, &extra) != 2 ||
            index > MAX_N || present[index] ||
            !parse_count(number, &values[index])) {
            (void)fclose(input);
            die("b-file is malformed or contains a duplicate index");
        }
        present[index] = true;
    }
    if (ferror(input) || fclose(input) != 0) {
        die("cannot finish reading b-file");
    }
}

static bool load_saved_term(unsigned n, Count *value)
{
    bool present[MAX_N + 1U];
    Count values[MAX_N + 1U];
    const int lock_descriptor = lock_bfile();
    read_bfile(present, values);
    const bool found = present[n];
    if (found) {
        *value = values[n];
    }
    unlock_bfile(lock_descriptor);
    return found;
}

static int write_bfile_line(FILE *stream, unsigned n, Count value)
{
    if (fprintf(stream, "%u ", n) < 0 || print_count(stream, value) != 0 ||
        fputc('\n', stream) == EOF) {
        return -1;
    }
    return 0;
}

static void record_term(unsigned n, Count value)
{
    bool present[MAX_N + 1U];
    Count values[MAX_N + 1U];
    const int lock_descriptor = lock_bfile();
    read_bfile(present, values);
    if (present[n]) {
        if (!counts_equal(values[n], value)) {
            unlock_bfile(lock_descriptor);
            die("computed term disagrees with the b-file");
        }
        unlock_bfile(lock_descriptor);
        return;
    }
    present[n] = true;
    values[n] = value;

    char temporary[] = "b291519_01.txt.tmp.XXXXXX";
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

static Count obtain_term(unsigned n, Memo *memo)
{
    Count value;
    if (load_saved_term(n, &value)) {
        if (!quiet) {
            fprintf(stderr, "291519_01: reusing saved a(%u)\n", n);
        }
        return value;
    }
    if (!quiet) {
        fprintf(stderr, "291519_01: n=%u reverse sparse DP started\n", n);
    }
    const double started = monotonic_seconds();
    const SearchResult result = count_reverse(n, memo);
    value = result.value;
    record_term(n, value);
    if (!quiet) {
        char text[COUNT_TEXT_SIZE];
        count_to_text(value, text);
        fprintf(stderr,
                "291519_01: n=%u answer=%s new_states=%zu "
                "total_states=%zu calls=%" PRIu64 " hits=%" PRIu64
                " branches=%" PRIu64 " uncached=%" PRIu64
                " memo_peak=%.1f MiB%s time=%.3fs\n",
                n, text, result.states_added, result.states_total,
                result.calls, result.hits, result.branches,
                result.uncached,
                (double)result.memory_peak / (1024.0 * 1024.0),
                result.frozen ? " frozen" : "",
                monotonic_seconds() - started);
    }
    return value;
}

static void check_implementation(unsigned maximum)
{
    const bool saved_quiet = quiet;
    quiet = true;
    Memo memo;
    memo_init(&memo);
    for (unsigned n = 0U; n <= maximum; ++n) {
        const SearchResult reverse = count_reverse(n, &memo);
        const Count forward = count_forward(n);
        if (!counts_equal(reverse.value, forward) ||
            !counts_equal(reverse.value, count_from_u64(known_terms[n]))) {
            memo_destroy(&memo);
            quiet = saved_quiet;
            fprintf(stderr, "error: check mismatch at n=%u\n", n);
            exit(EXIT_FAILURE);
        }
    }
    memo_destroy(&memo);
    quiet = saved_quiet;
    printf("ok: reverse sparse DP, direct forward enumeration, and known "
           "terms agree for n=0..%u\n", maximum);
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
            "A291519_MEMORY_MIB sets the memo allocation limit "
            "(default %" PRIu64 " MiB).\n",
            program, program, program, program,
            MAX_N, DEFAULT_MAX_N,
            DIRECT_CHECK_MAX_N, DIRECT_CHECK_MAX_N,
            BFILE_PATH, DEFAULT_MEMORY_MIB);
}

int main(int argc, char **argv)
{
    configure_memory_limit();
    prepare_cubes();

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
    } else {
        Memo memo;
        memo_init(&memo);
        if (mode == MODE_TERM) {
            const Count value = obtain_term(n, &memo);
            if (print_count(stdout, value) != 0 || putchar('\n') == EOF ||
                fflush(stdout) != 0) {
                memo_destroy(&memo);
                die("cannot write standard output");
            }
        } else {
            for (unsigned index = 0U; index <= n; ++index) {
                const Count value = obtain_term(index, &memo);
                if (write_bfile_line(stdout, index, value) != 0 ||
                    fflush(stdout) != 0) {
                    memo_destroy(&memo);
                    die("cannot write standard output");
                }
            }
        }
        memo_destroy(&memo);
    }
    return EXIT_SUCCESS;
}
