/*
 * A291355 -- reverse sparse subset DP.
 *
 * Count permutations s_1,...,s_n of [n], for n<=200, such that
 *
 *     j divides Sum_{i=1..j} s_i^3,       1 <= j <= n.
 *
 * Let U be the set of values occupying positions 1,...,j and put
 * P(U)=Sum_{x in U} x^3.  In a reverse construction, choosing x=s_j
 * leaves U-{x} at positions 1,...,j-1, so x is admissible exactly when
 *
 *     P(U)-x^3 == 0 (mod j-1).
 *
 * Thus, for j=|U|,
 *
 *     C(U) = Sum C(U-{x}),
 *              x in U, x^3 == P(U) (mod j-1),
 *
 * with C(U)=1 when |U|<=1.  The root is valid only when
 * n divides Sum_{x=1..n}x^3 = (n(n+1)/2)^2.  In particular, every
 * n == 2 (mod 4) is rejected immediately.
 *
 * The remaining set U determines both j and P(U), so it is a sufficient
 * memoization key.  Reachable subsets are stored in a sparse open-addressed
 * hash table.  Candidate sets for every modulus and residue are precomputed
 * as fixed 256-bit bitsets, making each transition an intersection followed by
 * iteration over its set bits.  The worst case remains exponential, but the
 * large moduli encountered first make the reachable state graph very sparse.
 *
 * Counts use checked unsigned 128-bit arithmetic.  n is limited to 200, so
 * values and their cubes fit comfortably in uint64_t and the set uses only
 * 200 of 256 available bits.  A291355_MEMORY_MIB bounds transient memo-table
 * storage (default 5120 MiB).
 *
 * Completed terms are saved atomically in b291355_01.txt and reused by later
 * runs.  --check compares the reverse memoized algorithm with an independent
 * definition-level forward search and built-in known terms for n<=16.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       291355_01.c -o 291355_01
 *
 * Examples:
 *   ./291355_01 --term 100
 *   ./291355_01 --upto 100
 *   ./291355_01 --check
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
#error "291355_01.c requires unsigned __int128"
#endif

#if SIZE_MAX < UINT64_MAX
#error "291355_01.c requires a 64-bit size_t"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 200U
#define DEFAULT_MAX_N 200U
#define DIRECT_CHECK_MAX_N 16U
#define BITSET_WORDS ((MAX_N + 63U) / 64U)
#define U128_TEXT_SIZE 40U
#define BFILE_PATH "b291355_01.txt"
#define LOCK_PATH "b291355_01.txt.lock"
#define INITIAL_MEMO_CAPACITY ((size_t)1024U)
#define DEFAULT_MEMORY_MIB UINT64_C(5120)
#define MIN_MEMORY_MIB UINT64_C(32)
#define MAX_MEMORY_MIB UINT64_C(65536)

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} Mode;

typedef struct {
    uint64_t word[BITSET_WORDS];
} Bitset;

/* The all-zero key is unused: only sets with at least two bits are cached. */
typedef struct {
    uint64_t word[BITSET_WORDS];
    U128 value;
} MemoEntry;

typedef struct {
    MemoEntry *entries;
    size_t capacity;
    size_t size;
    uint64_t memory_limit;
    uint64_t memory_current;
    uint64_t memory_peak;
    uint64_t calls;
    uint64_t hits;
    size_t next_report;
    unsigned n;
    double started;
} Memo;

static uint64_t cubes[MAX_N + 1U];
static Bitset residue_masks[MAX_N + 1U][MAX_N];
static bool tables_ready;
static bool quiet;
static uint64_t configured_memory_limit;

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
    for (unsigned i = 0U; i < BITSET_WORDS; ++i) {
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

static bool bitset_is_empty_words(const uint64_t words[BITSET_WORDS])
{
    uint64_t combined = 0U;
    for (unsigned i = 0U; i < BITSET_WORDS; ++i) {
        combined |= words[i];
    }
    return combined == 0U;
}

static bool bitset_equals_words(Bitset key,
                                const uint64_t words[BITSET_WORDS])
{
    for (unsigned i = 0U; i < BITSET_WORDS; ++i) {
        if (key.word[i] != words[i]) {
            return false;
        }
    }
    return true;
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

static uint64_t memo_hash(Bitset key)
{
    uint64_t hash = UINT64_C(0x6a09e667f3bcc909);
    for (unsigned i = 0U; i < BITSET_WORDS; ++i) {
        const uint64_t mixed = mix64(
            key.word[i] + UINT64_C(0x9e3779b97f4a7c15) * (i + 1U));
        hash ^= mixed;
        hash = (hash << 17U | hash >> 47U) *
            UINT64_C(0x9e3779b97f4a7c15);
    }
    return hash;
}

static uint64_t memo_bytes(size_t capacity)
{
    if (capacity > SIZE_MAX / sizeof(MemoEntry) ||
        capacity > UINT64_MAX / sizeof(MemoEntry)) {
        die("memo-table allocation size overflow");
    }
    return (uint64_t)capacity * sizeof(MemoEntry);
}

static void memo_allocate(Memo *memo, size_t capacity)
{
    const uint64_t bytes = memo_bytes(capacity);
    if (bytes > memo->memory_limit - memo->memory_current) {
        die("memo tables exceed A291355_MEMORY_MIB");
    }
    MemoEntry *entries = calloc(capacity, sizeof(*entries));
    if (entries == NULL) {
        die("cannot allocate memo table");
    }
    memo->entries = entries;
    memo->capacity = capacity;
    memo->memory_current += bytes;
    if (memo->memory_current > memo->memory_peak) {
        memo->memory_peak = memo->memory_current;
    }
}

static bool memo_lookup(Memo *memo, Bitset key, U128 *value)
{
    const uint64_t hash = memo_hash(key);
    size_t index = (size_t)hash & (memo->capacity - 1U);
    const size_t step = ((size_t)(hash >> 32U) | 1U) &
        (memo->capacity - 1U);
    for (;;) {
        const MemoEntry *entry = &memo->entries[index];
        if (bitset_is_empty_words(entry->word)) {
            return false;
        }
        if (bitset_equals_words(key, entry->word)) {
            *value = entry->value;
            ++memo->hits;
            return true;
        }
        index = (index + step) & (memo->capacity - 1U);
    }
}

static void memo_place(MemoEntry *entries, size_t capacity,
                       Bitset key, U128 value)
{
    const uint64_t hash = memo_hash(key);
    size_t index = (size_t)hash & (capacity - 1U);
    const size_t step = ((size_t)(hash >> 32U) | 1U) & (capacity - 1U);
    while (!bitset_is_empty_words(entries[index].word)) {
        index = (index + step) & (capacity - 1U);
    }
    for (unsigned i = 0U; i < BITSET_WORDS; ++i) {
        entries[index].word[i] = key.word[i];
    }
    entries[index].value = value;
}

static void memo_grow(Memo *memo)
{
    if (memo->capacity > SIZE_MAX / 2U) {
        die("memo-table capacity overflow");
    }
    const size_t old_capacity = memo->capacity;
    MemoEntry *old_entries = memo->entries;
    const uint64_t old_bytes = memo_bytes(old_capacity);
    memo->entries = NULL;
    memo->capacity = 0U;
    memo_allocate(memo, old_capacity * 2U);
    for (size_t i = 0U; i < old_capacity; ++i) {
        const MemoEntry *entry = &old_entries[i];
        if (!bitset_is_empty_words(entry->word)) {
            Bitset key;
            for (unsigned word = 0U; word < BITSET_WORDS; ++word) {
                key.word[word] = entry->word[word];
            }
            memo_place(memo->entries, memo->capacity,
                       key, entry->value);
        }
    }
    free(old_entries);
    memo->memory_current -= old_bytes;
}

static void memo_insert(Memo *memo, Bitset key, U128 value)
{
    /* Double hashing remains practical at this density and avoids a large
       otherwise unnecessary table doubling for the sparse n=200 search. */
    if (memo->size + 1U > memo->capacity * 9U / 10U) {
        const uint64_t next_bytes = memo_bytes(memo->capacity * 2U);
        if (next_bytes <= memo->memory_limit - memo->memory_current) {
            memo_grow(memo);
        } else if (memo->size + 1U > memo->capacity * 49U / 50U) {
            die("memo table is full within A291355_MEMORY_MIB");
        }
    }
    memo_place(memo->entries, memo->capacity, key, value);
    ++memo->size;
    if (!quiet && memo->size >= memo->next_report) {
        fprintf(stderr,
                "291355_01: n=%u states=%zu hits=%" PRIu64
                " memo=%.1f MiB time=%.1fs\n",
                memo->n, memo->size, memo->hits,
                (double)memo->memory_current / (1024.0 * 1024.0),
                monotonic_seconds() - memo->started);
        memo->next_report += (size_t)10000000U;
    }
}

static void increment_u64_saturating(uint64_t *value)
{
    if (*value != UINT64_MAX) {
        ++*value;
    }
}

static U128 reverse_search(unsigned cardinality, Bitset remaining,
                           uint64_t prefix_sum, Memo *memo)
{
    increment_u64_saturating(&memo->calls);
    if (cardinality <= 1U) {
        return 1U;
    }

    U128 saved;
    if (memo_lookup(memo, remaining, &saved)) {
        return saved;
    }

    const unsigned modulus = cardinality - 1U;
    const unsigned residue = (unsigned)(prefix_sum % modulus);
    Bitset candidates = bitset_intersection(
        remaining, residue_masks[modulus][residue]);
    U128 result = 0U;

    for (unsigned word = 0U; word < BITSET_WORDS; ++word) {
        uint64_t bits = candidates.word[word];
        while (bits != 0U) {
            const uint64_t bit = bits & (0U - bits);
            const unsigned value = word * 64U +
                (unsigned)__builtin_ctzll(bit) + 1U;
            const U128 addition = reverse_search(
                cardinality - 1U, bitset_without(remaining, value),
                prefix_sum - cubes[value], memo);
            checked_add(&result, addition);
            bits ^= bit;
        }
    }

    memo_insert(memo, remaining, result);
    return result;
}

typedef struct {
    U128 value;
    uint64_t calls;
    uint64_t hits;
    size_t states;
    uint64_t memory_peak;
} SearchResult;

static SearchResult count_reverse(unsigned n)
{
    SearchResult result = {0U, 0U, 0U, 0U, 0U};
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

    Memo memo = {
        .entries = NULL,
        .capacity = 0U,
        .size = 0U,
        .memory_limit = configured_memory_limit,
        .memory_current = 0U,
        .memory_peak = 0U,
        .calls = 0U,
        .hits = 0U,
        .next_report = (size_t)10000000U,
        .n = n,
        .started = monotonic_seconds()
    };
    memo_allocate(&memo, INITIAL_MEMO_CAPACITY);
    result.value = reverse_search(n, full_set(n), total, &memo);
    result.calls = memo.calls;
    result.hits = memo.hits;
    result.states = memo.size;
    result.memory_peak = memo.memory_peak;
    free(memo.entries);
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
    for (unsigned word = 0U; word < BITSET_WORDS; ++word) {
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

    char temporary[] = "b291355_01.txt.tmp.XXXXXX";
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
            fprintf(stderr, "291355_01: reusing saved a(%u)\n", n);
        }
        return value;
    }

    if (!quiet) {
        fprintf(stderr, "291355_01: n=%u reverse sparse DP started\n", n);
    }
    const double started = monotonic_seconds();
    const SearchResult result = count_reverse(n);
    value = result.value;
    record_term(n, value);
    if (!quiet) {
        char text[U128_TEXT_SIZE];
        u128_to_text(value, text);
        fprintf(stderr,
                "291355_01: n=%u answer=%s states=%zu calls=%" PRIu64
                " hits=%" PRIu64 " memo_peak=%.1f MiB time=%.3fs\n",
                n, text, result.states, result.calls, result.hits,
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
        const SearchResult reverse = count_reverse(n);
        const U128 forward = forward_search(n, 1U, full_set(n), 0U);
        if (reverse.value != forward || reverse.value != known_terms[n]) {
            quiet = saved_quiet;
            fprintf(stderr, "error: check mismatch at n=%u\n", n);
            exit(EXIT_FAILURE);
        }
    }
    quiet = saved_quiet;
    printf("ok: reverse sparse DP, forward enumeration, and known terms "
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
            "A291355_MEMORY_MIB sets the memo allocation limit "
            "(default %" PRIu64 " MiB).\n",
            program, program, program, program,
            MAX_N, DEFAULT_MAX_N,
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
