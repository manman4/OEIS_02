/*
 * A279214 -- exact sparse subset DP with a two-value frontier.
 *
 * Count permutations sigma of {1,...,n} satisfying
 *
 *   |sigma(i+1)-sigma(i)| >= 3,  1 <= i <= n-1,
 *   |sigma(i+2)-sigma(i)| >= 3,  1 <= i <= n-2.
 *
 * If a valid prefix ends in a,b, appending c creates new constraints only
 * against a and b.  Thus the exact DP state is
 *
 *                  (used values, last-but-one, last).
 *
 * From (S,a,b), append every unused c with |c-a|>=3 and |c-b|>=3:
 *
 *        D[S union {c}, b, c] += D[S,a,b].
 *
 * A bit mask represents S.  The permitted-c mask for every ordered pair
 * (a,b) is precomputed, so a transition needs one mask AND followed by
 * low-bit iteration.  Only reachable states are stored, in two hash-table
 * layers.  Completed earlier layers are cleared before the next layer.
 * This changes the definition's factorial search to worst-case
 *
 *        O(n^3 2^n) time and O(n^2 2^n) space,
 *
 * with substantially fewer states in practice because invalid prefixes
 * never enter the table.
 *
 * The default evaluator also quotients the value-complement symmetry
 * v -> n-1-v.  Every length-at-least-two ordered-pair state belongs to an
 * orbit of size two: a fixed state would require two distinct endpoint
 * values to be the unique middle value.  Counts of both orbit members are
 * stored together under the smaller packed key.  --no-symmetry disables
 * this optimization, and --check compares both evaluators.
 *
 * Safety:
 *   - every state count is at most n!;
 *   - 34! < 2^128, while 35! > 2^128;
 *   - therefore unsigned 128-bit counts are proved exact for n<=34;
 *   - all additions, key packing, table growth, and allocation sizes are
 *     nevertheless checked;
 *   - --memory-mib places a hard budget on DP hash-table allocations;
 *   - a range run flushes b279214_01_part.txt after every completed term
 *     and atomically renames it to b279214_01.txt only after success.
 *
 * The exponential state count, rather than the integer type, is normally
 * the practical limit.  Increasing --memory-mib permits a larger run but
 * is not a promise that a requested n will finish.
 *
 * Verification:
 *   - the OEIS b-file terms a(0)..a(17) are built in;
 *   - --check compares a direct full-permutation scan through n=10;
 *   - --check compares symmetry and no-symmetry DP through n=12;
 *   - the OEIS parity property 2 | a(n), n>1, is checked.
 *
 * Reference:
 *   https://oeis.org/A279214
 *
 * Build:
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     279214_01.c -o 279214_01
 *
 * Usage:
 *
 *   ./279214_01                    # write a(0)..a(17)
 *   ./279214_01 --upto 18
 *   ./279214_01 --term 18
 *   ./279214_01 --term 18 --memory-mib 2048
 *   ./279214_01 --no-symmetry --term 12
 *   ./279214_01 --check
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

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#if !defined(__SIZEOF_INT128__)
#error "279214_01 requires compiler support for unsigned __int128"
#endif

__extension__ typedef unsigned __int128 uint128_t;

#define SEQUENCE_OFFSET 0
#define DEFAULT_MAX_N 17
#define MAX_SUPPORTED_N 34
#define KNOWN_MAX_N 17
#define DEFAULT_CHECK_N 17
#define DIRECT_CHECK_MAX_N 10
#define SYMMETRY_CHECK_MAX_N 12
#define DEFAULT_MEMORY_MIB UINT64_C(1024)
#define MIN_MEMORY_MIB UINT64_C(16)
#define INITIAL_CAPACITY ((size_t)16)
#define LOAD_NUMERATOR ((size_t)7)
#define LOAD_DENOMINATOR ((size_t)10)
#define ENDPOINT_BITS 6U
#define ENDPOINT_MASK UINT64_C(63)
#define STATE_SHIFT (2U * ENDPOINT_BITS)

/* A279214, n=0..17, from https://oeis.org/A279214/b279214.txt. */
static const char *const known_terms[KNOWN_MAX_N + 1] = {
    "1",
    "1",
    "0",
    "0",
    "0",
    "0",
    "0",
    "0",
    "0",
    "2",
    "40",
    "792",
    "15374",
    "281434",
    "5089060",
    "93082532",
    "1743601076",
    "33694028152"
};

typedef struct {
    uint64_t low;
    uint64_t high;
} Count128;

/* Three 64-bit words keep each hash slot at 24 bytes on 64-bit targets. */
typedef struct {
    uint64_t key_plus_one; /* zero means empty */
    Count128 count;
} StateSlot;

typedef struct {
    size_t limit;
    size_t in_use;
    size_t peak;
} MemoryBudget;

typedef struct {
    StateSlot *slots;
    size_t capacity;
    size_t size;
    MemoryBudget *budget;
} StateMap;

typedef struct {
    uint64_t transitions;
    size_t peak_states;
    size_t peak_bytes;
    double seconds;
} DpStats;

typedef enum {
    RUN_UPTO,
    RUN_TERM,
    RUN_CHECK
} RunMode;

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

static size_t checked_product(size_t left, size_t right)
{
    if (left != 0U && right > SIZE_MAX / left) {
        die("allocation size overflow");
    }
    return left * right;
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

static uint64_t parse_memory_mib(const char *text)
{
    char *end = NULL;
    errno = 0;
    unsigned long long value = strtoull(text, &end, 10);
    uint64_t maximum = (uint64_t)(SIZE_MAX / (size_t)UINT64_C(1048576));
    if (errno != 0 || end == text || *end != '\0' ||
        value < MIN_MEMORY_MIB || value > maximum) {
        fprintf(stderr,
                "error: MEMORY_MIB must be in %" PRIu64 "..%" PRIu64
                ": %s\n",
                MIN_MEMORY_MIB, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (uint64_t)value;
}

static Count128 count_from_u64(uint64_t value)
{
    Count128 result = {value, 0};
    return result;
}

static bool count_equal(Count128 left, Count128 right)
{
    return left.low == right.low && left.high == right.high;
}

static void count_add_to(Count128 *destination, Count128 addend)
{
    uint64_t old_low = destination->low;
    uint64_t new_low = old_low + addend.low;
    uint64_t carry = new_low < old_low ? UINT64_C(1) : UINT64_C(0);

    uint64_t old_high = destination->high;
    uint64_t new_high = old_high + addend.high;
    if (new_high < old_high) {
        die("unsigned 128-bit count overflow");
    }
    uint64_t final_high = new_high + carry;
    if (final_high < new_high) {
        die("unsigned 128-bit count overflow");
    }
    destination->low = new_low;
    destination->high = final_high;
}

static uint128_t count_to_u128(Count128 value)
{
    return ((uint128_t)value.high << 64U) | (uint128_t)value.low;
}

static Count128 parse_count(const char *text)
{
    uint128_t value = 0;
    for (const unsigned char *p = (const unsigned char *)text;
         *p != '\0'; ++p) {
        if (*p < (unsigned char)'0' || *p > (unsigned char)'9') {
            die("invalid built-in decimal value");
        }
        unsigned digit = (unsigned)(*p - (unsigned char)'0');
        uint128_t maximum = ~(uint128_t)0;
        if (value > (maximum - (uint128_t)digit) / (uint128_t)10U) {
            die("built-in decimal value exceeds unsigned 128-bit range");
        }
        value = value * (uint128_t)10U + (uint128_t)digit;
    }
    Count128 result = {(uint64_t)value, (uint64_t)(value >> 64U)};
    return result;
}

static int fprint_count(FILE *stream, Count128 count)
{
    char digits[40];
    size_t length = 0;
    uint128_t value = count_to_u128(count);
    do {
        digits[length++] = (char)('0' + (unsigned)(value % 10U));
        value /= 10U;
    } while (value != 0);

    while (length != 0U) {
        if (fputc((unsigned char)digits[--length], stream) == EOF) {
            return -1;
        }
    }
    return 0;
}

static void budget_reserve(MemoryBudget *budget, size_t bytes)
{
    if (bytes > budget->limit - budget->in_use) {
        fprintf(stderr,
                "error: DP memory budget exceeded: need another %.1f MiB "
                "with %.1f/%.1f MiB already in use; rerun with a larger "
                "--memory-mib value\n",
                (double)bytes / 1048576.0,
                (double)budget->in_use / 1048576.0,
                (double)budget->limit / 1048576.0);
        exit(EXIT_FAILURE);
    }
    budget->in_use += bytes;
    if (budget->peak < budget->in_use) {
        budget->peak = budget->in_use;
    }
}

static void budget_release(MemoryBudget *budget, size_t bytes)
{
    if (bytes > budget->in_use) {
        die("internal memory-budget underflow");
    }
    budget->in_use -= bytes;
}

static StateSlot *allocate_slots(MemoryBudget *budget, size_t capacity)
{
    size_t bytes = checked_product(capacity, sizeof(StateSlot));
    budget_reserve(budget, bytes);
    StateSlot *slots = calloc(capacity, sizeof(StateSlot));
    if (slots == NULL) {
        fprintf(stderr,
                "error: could not allocate %.1f MiB for the DP table\n",
                (double)bytes / 1048576.0);
        exit(EXIT_FAILURE);
    }
    return slots;
}

static void free_slots(MemoryBudget *budget, StateSlot *slots,
                       size_t capacity)
{
    size_t bytes = checked_product(capacity, sizeof(StateSlot));
    free(slots);
    budget_release(budget, bytes);
}

static uint64_t mix_key(uint64_t value)
{
    value ^= value >> 30U;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27U;
    value *= UINT64_C(0x94d049bb133111eb);
    value ^= value >> 31U;
    return value;
}

static void map_init(StateMap *map, MemoryBudget *budget)
{
    map->capacity = INITIAL_CAPACITY;
    map->size = 0;
    map->budget = budget;
    map->slots = allocate_slots(budget, map->capacity);
}

static void map_destroy(StateMap *map)
{
    if (map->slots != NULL) {
        free_slots(map->budget, map->slots, map->capacity);
    }
    map->slots = NULL;
    map->capacity = 0;
    map->size = 0;
}

static StateSlot *map_find_slot(StateMap *map, uint64_t key_plus_one)
{
    size_t index = (size_t)mix_key(key_plus_one) & (map->capacity - 1U);
    for (;;) {
        StateSlot *slot = &map->slots[index];
        if (slot->key_plus_one == 0 ||
            slot->key_plus_one == key_plus_one) {
            return slot;
        }
        index = (index + 1U) & (map->capacity - 1U);
    }
}

static void map_grow(StateMap *map)
{
    if (map->capacity > SIZE_MAX / 2U) {
        die("DP hash-table capacity overflow");
    }
    size_t old_capacity = map->capacity;
    StateSlot *old_slots = map->slots;
    size_t new_capacity = old_capacity * 2U;
    StateSlot *new_slots = allocate_slots(map->budget, new_capacity);

    map->capacity = new_capacity;
    map->slots = new_slots;
    for (size_t i = 0; i < old_capacity; ++i) {
        if (old_slots[i].key_plus_one != 0) {
            StateSlot *destination =
                map_find_slot(map, old_slots[i].key_plus_one);
            *destination = old_slots[i];
        }
    }
    free_slots(map->budget, old_slots, old_capacity);
}

static void map_add(StateMap *map, uint64_t packed_key, Count128 addend)
{
    if (packed_key == UINT64_MAX) {
        die("packed state key overflow");
    }
    uint64_t key_plus_one = packed_key + UINT64_C(1);
    StateSlot *slot = map_find_slot(map, key_plus_one);
    if (slot->key_plus_one != 0) {
        count_add_to(&slot->count, addend);
        return;
    }

    if (map->size + 1U >
        (map->capacity / LOAD_DENOMINATOR) * LOAD_NUMERATOR) {
        map_grow(map);
        slot = map_find_slot(map, key_plus_one);
        if (slot->key_plus_one != 0) {
            die("internal duplicate state during hash-table growth");
        }
    }
    slot->key_plus_one = key_plus_one;
    slot->count = addend;
    ++map->size;
}

static void map_clear(StateMap *map)
{
    size_t bytes = checked_product(map->capacity, sizeof(StateSlot));
    memset(map->slots, 0, bytes);
    map->size = 0;
}

static uint64_t reverse_bits_64(uint64_t value)
{
    value = ((value & UINT64_C(0x5555555555555555)) << 1U) |
            ((value >> 1U) & UINT64_C(0x5555555555555555));
    value = ((value & UINT64_C(0x3333333333333333)) << 2U) |
            ((value >> 2U) & UINT64_C(0x3333333333333333));
    value = ((value & UINT64_C(0x0f0f0f0f0f0f0f0f)) << 4U) |
            ((value >> 4U) & UINT64_C(0x0f0f0f0f0f0f0f));
    value = ((value & UINT64_C(0x00ff00ff00ff00ff)) << 8U) |
            ((value >> 8U) & UINT64_C(0x00ff00ff00ff00ff));
    value = ((value & UINT64_C(0x0000ffff0000ffff)) << 16U) |
            ((value >> 16U) & UINT64_C(0x0000ffff0000ffff));
    return (value << 32U) | (value >> 32U);
}

static uint64_t reverse_low_bits(uint64_t mask, int n)
{
    return reverse_bits_64(mask) >> (64U - (unsigned)n);
}

static uint64_t pack_state(uint64_t used, unsigned a, unsigned b)
{
    return (used << STATE_SHIFT) |
           ((uint64_t)a << ENDPOINT_BITS) | (uint64_t)b;
}

static void unpack_state(uint64_t packed, uint64_t *used,
                         unsigned *a, unsigned *b)
{
    *b = (unsigned)(packed & ENDPOINT_MASK);
    *a = (unsigned)((packed >> ENDPOINT_BITS) & ENDPOINT_MASK);
    *used = packed >> STATE_SHIFT;
}

static uint64_t canonical_state(uint64_t used, unsigned a, unsigned b,
                                int n, bool use_symmetry)
{
    uint64_t original = pack_state(used, a, b);
    if (!use_symmetry) {
        return original;
    }
    uint64_t complemented = pack_state(
        reverse_low_bits(used, n),
        (unsigned)(n - 1 - (int)a),
        (unsigned)(n - 1 - (int)b));
    return original < complemented ? original : complemented;
}

static void update_stats(DpStats *stats, const StateMap *map,
                         const MemoryBudget *budget)
{
    if (stats->peak_states < map->size) {
        stats->peak_states = map->size;
    }
    if (stats->peak_bytes < budget->peak) {
        stats->peak_bytes = budget->peak;
    }
}

static Count128 compute_dp(int n, bool use_symmetry,
                           uint64_t memory_mib, DpStats *stats)
{
    double started = monotonic_seconds();
    memset(stats, 0, sizeof(*stats));
    if (n == 0 || n == 1) {
        stats->seconds = monotonic_seconds() - started;
        return count_from_u64(UINT64_C(1));
    }

    MemoryBudget budget;
    budget.limit = checked_product((size_t)memory_mib,
                                   (size_t)UINT64_C(1048576));
    budget.in_use = 0;
    budget.peak = 0;

    uint64_t full = (UINT64_C(1) << (unsigned)n) - UINT64_C(1);
    uint64_t bad[MAX_SUPPORTED_N] = {0};
    uint64_t allowed[MAX_SUPPORTED_N][MAX_SUPPORTED_N] = {{0}};
    for (int value = 0; value < n; ++value) {
        for (int other = 0; other < n; ++other) {
            int difference = value - other;
            if (difference > -3 && difference < 3) {
                bad[value] |= UINT64_C(1) << (unsigned)other;
            }
        }
    }
    for (int a = 0; a < n; ++a) {
        for (int b = 0; b < n; ++b) {
            allowed[a][b] = full & ~(bad[a] | bad[b]);
        }
    }

    StateMap current;
    StateMap next;
    map_init(&current, &budget);
    map_init(&next, &budget);

    Count128 one = count_from_u64(UINT64_C(1));
    for (int a = 0; a < n; ++a) {
        for (int b = 0; b < n; ++b) {
            int difference = a - b;
            if (difference > -3 && difference < 3) {
                continue;
            }
            uint64_t used = (UINT64_C(1) << (unsigned)a) |
                            (UINT64_C(1) << (unsigned)b);
            uint64_t key = canonical_state(used, (unsigned)a,
                                           (unsigned)b, n,
                                           use_symmetry);
            map_add(&current, key, one);
        }
    }
    update_stats(stats, &current, &budget);

    for (int length = 2; length < n && current.size != 0U; ++length) {
        map_clear(&next);
        for (size_t index = 0; index < current.capacity; ++index) {
            const StateSlot *slot = &current.slots[index];
            if (slot->key_plus_one == 0) {
                continue;
            }
            uint64_t used;
            unsigned a;
            unsigned b;
            unpack_state(slot->key_plus_one - UINT64_C(1),
                         &used, &a, &b);
            uint64_t candidates = allowed[a][b] & ~used;
            while (candidates != 0) {
                uint64_t bit = candidates & (UINT64_C(0) - candidates);
                candidates ^= bit;
#if defined(__GNUC__) || defined(__clang__)
                unsigned c = (unsigned)__builtin_ctzll(bit);
#else
                unsigned c = 0;
                while ((bit >> c) != UINT64_C(1)) {
                    ++c;
                }
#endif
                uint64_t new_used = used | bit;
                uint64_t key = canonical_state(new_used, b, c, n,
                                               use_symmetry);
                map_add(&next, key, slot->count);
                if (stats->transitions == UINT64_MAX) {
                    die("transition counter overflow");
                }
                ++stats->transitions;
            }
        }

        StateMap temporary = current;
        current = next;
        next = temporary;
        update_stats(stats, &current, &budget);
    }

    Count128 result = count_from_u64(UINT64_C(0));
    if (n == 2 || current.size != 0U) {
        for (size_t index = 0; index < current.capacity; ++index) {
            if (current.slots[index].key_plus_one != 0) {
                count_add_to(&result, current.slots[index].count);
            }
        }
    }

    stats->peak_bytes = budget.peak;
    map_destroy(&current);
    map_destroy(&next);
    if (budget.in_use != 0U) {
        die("internal DP memory leak");
    }
    stats->seconds = monotonic_seconds() - started;
    return result;
}

static bool permutation_is_valid(const int *permutation, int n)
{
    for (int i = 0; i < n; ++i) {
        if (i + 1 < n) {
            int difference = permutation[i + 1] - permutation[i];
            if (difference > -3 && difference < 3) {
                return false;
            }
        }
        if (i + 2 < n) {
            int difference = permutation[i + 2] - permutation[i];
            if (difference > -3 && difference < 3) {
                return false;
            }
        }
    }
    return true;
}

static bool next_permutation(int *permutation, int n)
{
    int i = n - 2;
    while (i >= 0 && permutation[i] >= permutation[i + 1]) {
        --i;
    }
    if (i < 0) {
        return false;
    }
    int j = n - 1;
    while (permutation[j] <= permutation[i]) {
        --j;
    }
    int temporary = permutation[i];
    permutation[i] = permutation[j];
    permutation[j] = temporary;
    for (int left = i + 1, right = n - 1; left < right;
         ++left, --right) {
        temporary = permutation[left];
        permutation[left] = permutation[right];
        permutation[right] = temporary;
    }
    return true;
}

static Count128 direct_count(int n)
{
    if (n == 0) {
        return count_from_u64(UINT64_C(1));
    }
    int permutation[DIRECT_CHECK_MAX_N];
    for (int i = 0; i < n; ++i) {
        permutation[i] = i;
    }
    Count128 result = count_from_u64(UINT64_C(0));
    do {
        if (permutation_is_valid(permutation, n)) {
            count_add_to(&result, count_from_u64(UINT64_C(1)));
        }
    } while (next_permutation(permutation, n));
    return result;
}

static void verify_known(Count128 value, int n)
{
    if (n <= KNOWN_MAX_N) {
        Count128 expected = parse_count(known_terms[n]);
        if (!count_equal(value, expected)) {
            fprintf(stderr, "error: A279214 mismatch at n=%d: got ", n);
            (void)fprint_count(stderr, value);
            fprintf(stderr, ", expected %s\n", known_terms[n]);
            exit(EXIT_FAILURE);
        }
    }
    if (n > 1 && (value.low & UINT64_C(1)) != 0) {
        fprintf(stderr, "error: parity check failed at n=%d\n", n);
        exit(EXIT_FAILURE);
    }
}

static Count128 compute_value(int n, bool use_symmetry,
                              uint64_t memory_mib, bool verbose)
{
    DpStats stats;
    Count128 value = compute_dp(n, use_symmetry, memory_mib, &stats);
    verify_known(value, n);
    if (verbose) {
        fprintf(stderr,
                "279214_01: n=%d, %s, peak states=%zu, "
                "transitions=%" PRIu64 ", peak table memory=%.1f MiB, "
                "%.3f s\n",
                n, use_symmetry ? "complement-quotient DP" : "plain DP",
                stats.peak_states, stats.transitions,
                (double)stats.peak_bytes / 1048576.0, stats.seconds);
    }
    return value;
}

static int check_implementation(int maximum_n, uint64_t memory_mib)
{
    if (maximum_n > KNOWN_MAX_N) {
        fprintf(stderr, "error: CHECK_N must be in 0..%d: %d\n",
                KNOWN_MAX_N, maximum_n);
        return EXIT_FAILURE;
    }

    for (int n = 0; n <= maximum_n; ++n) {
        Count128 quotient = compute_value(n, true, memory_mib, false);
        if (n <= SYMMETRY_CHECK_MAX_N) {
            Count128 plain = compute_value(n, false, memory_mib, false);
            if (!count_equal(quotient, plain)) {
                fprintf(stderr,
                        "error: symmetry/plain DP mismatch at n=%d: ", n);
                (void)fprint_count(stderr, quotient);
                fputs(" versus ", stderr);
                (void)fprint_count(stderr, plain);
                fputc('\n', stderr);
                return EXIT_FAILURE;
            }
        }
        if (n <= DIRECT_CHECK_MAX_N) {
            Count128 direct = direct_count(n);
            if (!count_equal(quotient, direct)) {
                fprintf(stderr,
                        "error: DP/direct-permutation mismatch at n=%d: ",
                        n);
                (void)fprint_count(stderr, quotient);
                fputs(" versus ", stderr);
                (void)fprint_count(stderr, direct);
                fputc('\n', stderr);
                return EXIT_FAILURE;
            }
        }
    }
    printf("ok: A279214 b-file verified through n=%d; independent full "
           "permutation scan through n=%d; complement-quotient and plain "
           "subset DP through n=%d; parity checked for n>1\n",
           maximum_n,
           maximum_n < DIRECT_CHECK_MAX_N
               ? maximum_n : DIRECT_CHECK_MAX_N,
           maximum_n < SYMMETRY_CHECK_MAX_N
               ? maximum_n : SYMMETRY_CHECK_MAX_N);
    return EXIT_SUCCESS;
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
                              sizeof(executable) - 1U);
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
        memcpy(executable, argv0, length + 1U);
    }
    const char *base = realpath(executable, resolved);
    if (base == NULL) {
        base = executable;
    }
    const char *slash = strrchr(base, '/');
    size_t directory_length =
        slash == NULL ? 1U : (size_t)(slash - base);
    if (slash != NULL && directory_length == 0U) {
        directory_length = 1U;
    }
    size_t filename_length = strlen(filename);
    if (directory_length > SIZE_MAX - filename_length - 2U) {
        die("output path length overflow");
    }
    char *path = malloc(directory_length + filename_length + 2U);
    if (path == NULL) {
        die("out of memory while making output path");
    }
    if (slash == NULL) {
        path[0] = '.';
    } else if (slash == base) {
        path[0] = '/';
    } else {
        memcpy(path, base, directory_length);
    }
    path[directory_length] = '/';
    memcpy(path + directory_length + 1U, filename,
           filename_length + 1U);
    return path;
}

static void produce_file(const char *argv0, int maximum_n,
                         bool use_symmetry, uint64_t memory_mib)
{
    char *final_path =
        path_beside_executable(argv0, "b279214_01.txt");
    char *part_path =
        path_beside_executable(argv0, "b279214_01_part.txt");
    FILE *stream = fopen(part_path, "w");
    if (stream == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part_path, strerror(errno));
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }

    for (int n = SEQUENCE_OFFSET; n <= maximum_n; ++n) {
        Count128 value =
            compute_value(n, use_symmetry, memory_mib, true);
        if (fprintf(stream, "%d ", n) < 0 ||
            fprint_count(stream, value) != 0 ||
            fputc('\n', stream) == EOF || fflush(stream) != 0) {
            (void)fclose(stream);
            free(final_path);
            free(part_path);
            die("could not write the A279214 output file");
        }
    }
    if (fclose(stream) != 0) {
        free(final_path);
        free(part_path);
        die("could not close the A279214 output file");
    }
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        free(final_path);
        free(part_path);
        exit(EXIT_FAILURE);
    }
    printf("wrote %s (n=%d..%d)\n",
           final_path, SEQUENCE_OFFSET, maximum_n);
    free(final_path);
    free(part_path);
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--memory-mib M] [--no-symmetry]\n"
            "       %s --upto MAX_N [--memory-mib M] [--no-symmetry]\n"
            "       %s --term N [--memory-mib M] [--no-symmetry]\n"
            "       %s --check [MAX_N] [--memory-mib M]\n"
            "\n"
            "N and MAX_N may be %d..%d; the 128-bit count is proved "
            "safe throughout.\n"
            "The default is --upto %d with a %" PRIu64
            " MiB DP-table budget.\n"
            "A range run writes b279214_01.txt beside the executable.\n"
            "--no-symmetry is slower and is mainly an independent "
            "diagnostic.\n",
            program, program, program, program,
            SEQUENCE_OFFSET, MAX_SUPPORTED_N, DEFAULT_MAX_N,
            DEFAULT_MEMORY_MIB);
}

int main(int argc, char **argv)
{
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 ||
         strcmp(argv[1], "-h") == 0)) {
        usage(argv[0]);
        return EXIT_SUCCESS;
    }

    RunMode mode = RUN_UPTO;
    int n = -1;
    bool have_n = false;
    bool have_mode = false;
    bool use_symmetry = true;
    bool have_symmetry_option = false;
    uint64_t memory_mib = DEFAULT_MEMORY_MIB;
    bool have_memory = false;

    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--memory-mib") == 0) {
            if (have_memory || argument + 1 >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            memory_mib = parse_memory_mib(argv[++argument]);
            have_memory = true;
        } else if (strcmp(text, "--no-symmetry") == 0) {
            if (have_symmetry_option) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            use_symmetry = false;
            have_symmetry_option = true;
        } else if (strcmp(text, "--term") == 0 ||
                   strcmp(text, "--upto") == 0) {
            if (have_mode || have_n || argument + 1 >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = strcmp(text, "--term") == 0 ? RUN_TERM : RUN_UPTO;
            have_mode = true;
            n = parse_n(argv[++argument],
                        mode == RUN_TERM ? "N" : "MAX_N");
            have_n = true;
        } else if (strcmp(text, "--check") == 0) {
            if (have_mode || have_n) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = RUN_CHECK;
            have_mode = true;
            if (argument + 1 < argc && argv[argument + 1][0] != '-') {
                n = parse_n(argv[++argument], "CHECK_N");
                have_n = true;
            }
        } else if (text[0] == '-' || have_n) {
            usage(argv[0]);
            return EXIT_FAILURE;
        } else {
            n = parse_n(text,
                        mode == RUN_CHECK ? "CHECK_N" : "MAX_N");
            have_n = true;
        }
    }

    if (!have_n) {
        n = mode == RUN_CHECK ? DEFAULT_CHECK_N : DEFAULT_MAX_N;
    }
    if (mode == RUN_CHECK) {
        return check_implementation(n, memory_mib);
    }
    if (mode == RUN_TERM) {
        Count128 value =
            compute_value(n, use_symmetry, memory_mib, true);
        printf("%d ", n);
        if (fprint_count(stdout, value) != 0 || fputc('\n', stdout) == EOF) {
            die("could not write result");
        }
        return EXIT_SUCCESS;
    }
    produce_file(argv[0], n, use_symmetry, memory_mib);
    return EXIT_SUCCESS;
}
