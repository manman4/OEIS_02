/*
 * A291551 -- independent fixed-middle frontier-intersection DP.
 *
 * Count permutations s_1,...,s_n of 1,...,n such that every prefix sum
 * divides the corresponding prefix product.
 *
 * This is intentionally different from 291551_01.c.  Put
 *
 *     good(S) := sum(S) divides product(S).
 *
 * At the fixed middle depth m=ceil(n/2), this program constructs
 *
 *   F(S): valid orders of S from the empty set,
 *   B(S): valid orders of the complement after S,
 *
 * and returns sum F(S)B(S).  The forward frontier F at depth m is completed
 * first.  During the backward pass, a state at depth m+1 is retained only if
 * deleting one element reaches F, and the final backward layer is restricted
 * exactly to F.  These tests are necessary for every complete permutation and
 * therefore discard no answer.
 *
 * Since n<=32 and m=ceil(n/2), both F(S) and B(S) are at most 16!, so each
 * state count safely fits uint64_t.  Only the final products and sum use
 * unsigned 128-bit integers; 32! fits in that type.  Prefix products are never
 * formed.  Divisibility is tested from prime valuations using popcount masks.
 *
 * States use 32-bit subset masks and are densely allocated in fixed blocks.
 * A separate chained bucket array provides exact lookup without the large
 * resize copies of open addressing.  Full keys are compared, so collisions
 * affect running time only.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       291551_02.c -o 291551_02
 *
 * Examples:
 *   ./291551_02 32
 *   ./291551_02 --term 32 --memory-limit-mib 8192
 *   ./291551_02 --check
 *
 * Every completed term is atomically saved in b291551_02.txt.  Normal runs
 * reuse saved terms; --check independently recomputes the known prefix.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if !defined(__SIZEOF_INT128__)
#error "291551_02.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 32
#define DEFAULT_MAX_N 32
#define KNOWN_MAX_N 31
#define DEFAULT_MEMORY_MIB 8192
#define MIN_MEMORY_MIB 16
#define MAX_MEMORY_MIB 65536
#define MAX_DISTINCT_FACTORS 5
#define MAX_PRIME_POWERS 6
#define MASK_BYTE_COUNT 4
#define STATE_BLOCK_SHIFT 18
#define STATE_BLOCK_CAPACITY (UINT32_C(1) << STATE_BLOCK_SHIFT)
#define STATE_BLOCK_MASK (STATE_BLOCK_CAPACITY - 1U)
#define MAX_STATE_BLOCKS \
    ((size_t)(UINT64_C(1) << 32) / STATE_BLOCK_CAPACITY)
#define MAX_BUCKET_LOAD 2U

_Static_assert(MAX_N == 32, "full mask construction assumes 32 values");

typedef struct {
    uint16_t prime;
    uint8_t exponent;
} PrimeRequirement;

typedef struct {
    uint8_t count;
    PrimeRequirement item[MAX_DISTINCT_FACTORS];
} SumRequirement;

typedef struct {
    unsigned n;
    unsigned total_sum;
    uint32_t full_mask;
    uint32_t power_masks[MAX_N + 1][MAX_PRIME_POWERS];
    uint8_t power_count[MAX_N + 1];
    uint16_t byte_sums[MASK_BYTE_COUNT][256];
    uint8_t mask_byte_count;
    SumRequirement requirement[MAX_N * (MAX_N + 1) / 2 + 1];
} DivisibilityContext;

typedef struct {
    uint32_t *keys;
    uint64_t *values;
    uint32_t *next;
} StateBlock;

typedef struct {
    StateBlock *blocks;
    uint32_t *heads;
    size_t bucket_count;
    size_t block_count;
    size_t count;
    bool store_values;
} StateMap;

static size_t memory_limit;
static size_t memory_used;
static size_t peak_memory;
static bool quiet;
static const char *const bfile_path = "b291551_02.txt";
static const char *const bfile_lock_path = "b291551_02.txt.lock";

static const char *const known_terms[KNOWN_MAX_N + 1] = {
    "1", "1", "0", "0", "0", "0", "0", "0", "0", "0", "0",
    "0", "0", "0", "0", "26", "0", "262", "0", "10226",
    "43964", "139484", "0", "13936472", "59652396", "301235944",
    "1915640632", "7969506364", "0", "465804291196", "0",
    "235940304387700"
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

static unsigned parse_bounded(const char *text, unsigned low, unsigned high,
                              const char *label)
{
    char *end = NULL;
    errno = 0;
    const unsigned long value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < low || value > high) {
        fprintf(stderr, "error: %s must be in %u..%u: %s\n",
                label, low, high, text);
        exit(EXIT_FAILURE);
    }
    return (unsigned)value;
}

static void u128_to_text(U128 value, char text[40])
{
    char reverse[40];
    size_t length = 0;
    do {
        reverse[length++] = (char)('0' + (unsigned)(value % 10U));
        value /= 10U;
    } while (value != 0);
    for (size_t i = 0; i < length; ++i) {
        text[i] = reverse[length - 1U - i];
    }
    text[length] = '\0';
}

static bool parse_u128(const char *text, U128 *result)
{
    const U128 maximum = ~(U128)0;
    U128 value = 0;
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
        value = 10U * value + digit;
    }
    *result = value;
    return true;
}

static void print_term(unsigned n, U128 value)
{
    char text[40];
    u128_to_text(value, text);
    if (printf("%u %s\n", n, text) < 0 || fflush(stdout) != 0) {
        die("cannot write output");
    }
}

static bool add_u128(U128 *destination, U128 amount)
{
    const U128 old = *destination;
    *destination += amount;
    return *destination >= old;
}

static int lock_bfile(void)
{
    const int descriptor = open(bfile_lock_path, O_RDWR | O_CREAT, 0666);
    if (descriptor < 0) {
        die("cannot open b-file lock");
    }
    struct flock lock = {
        .l_type = F_WRLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0
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
        .l_type = F_UNLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0
    };
    if (fcntl(descriptor, F_SETLK, &lock) != 0) {
        close(descriptor);
        die("cannot unlock b-file");
    }
    if (close(descriptor) != 0) {
        die("cannot close b-file lock");
    }
}

static void read_bfile(bool present[MAX_N + 1],
                       char values[MAX_N + 1][40])
{
    memset(present, 0, (MAX_N + 1U) * sizeof(*present));
    FILE *input = fopen(bfile_path, "r");
    if (input == NULL) {
        if (errno == ENOENT) {
            return;
        }
        die("cannot read b-file");
    }
    char line[256];
    while (fgets(line, sizeof(line), input) != NULL) {
        char *cursor = line;
        while (*cursor == ' ' || *cursor == '\t') {
            ++cursor;
        }
        if (*cursor == '\0' || *cursor == '\n' || *cursor == '#') {
            continue;
        }
        unsigned index;
        char number[40], extra;
        if (sscanf(cursor, "%u %39s %c", &index, number, &extra) != 2 ||
            index > MAX_N || present[index]) {
            fclose(input);
            die("b-file is malformed or has a duplicate term");
        }
        U128 parsed;
        if (!parse_u128(number, &parsed)) {
            fclose(input);
            die("b-file contains an invalid value");
        }
        present[index] = true;
        strcpy(values[index], number);
    }
    if (ferror(input) || fclose(input) != 0) {
        die("cannot finish reading b-file");
    }
}

static bool load_saved_term(unsigned n, U128 *value)
{
    bool present[MAX_N + 1];
    char values[MAX_N + 1][40];
    const int lock_descriptor = lock_bfile();
    read_bfile(present, values);
    const bool found = present[n];
    if (found && !parse_u128(values[n], value)) {
        unlock_bfile(lock_descriptor);
        die("saved term is invalid");
    }
    unlock_bfile(lock_descriptor);
    return found;
}

static void record_term(unsigned n, U128 value)
{
    bool present[MAX_N + 1];
    char values[MAX_N + 1][40], text[40];
    u128_to_text(value, text);
    const int lock_descriptor = lock_bfile();
    read_bfile(present, values);
    if (present[n]) {
        if (strcmp(values[n], text) != 0) {
            unlock_bfile(lock_descriptor);
            die("computed term disagrees with b-file");
        }
        unlock_bfile(lock_descriptor);
        return;
    }
    present[n] = true;
    strcpy(values[n], text);

    char temporary[] = "b291551_02.txt.tmp.XXXXXX";
    const int descriptor = mkstemp(temporary);
    if (descriptor < 0 || fchmod(descriptor, 0644) != 0) {
        if (descriptor >= 0) {
            close(descriptor);
            unlink(temporary);
        }
        unlock_bfile(lock_descriptor);
        die("cannot create temporary b-file");
    }
    FILE *output = fdopen(descriptor, "w");
    if (output == NULL) {
        close(descriptor);
        unlink(temporary);
        unlock_bfile(lock_descriptor);
        die("cannot open temporary b-file stream");
    }
    bool failed = false;
    for (unsigned index = 0; index <= MAX_N; ++index) {
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
    if (failed || rename(temporary, bfile_path) != 0) {
        unlink(temporary);
        unlock_bfile(lock_descriptor);
        die("cannot atomically update b-file");
    }
    unlock_bfile(lock_descriptor);
    if (!quiet) {
        fprintf(stderr, "A291551(%u): saved in %s\n", n, bfile_path);
    }
}

static void account_allocation(size_t bytes)
{
    if (bytes > memory_limit || memory_used > memory_limit - bytes) {
        fprintf(stderr,
                "error: memory limit exceeded: need another %.2f MiB; "
                "using %.2f of %.2f MiB\n",
                (double)bytes / 1048576.0,
                (double)memory_used / 1048576.0,
                (double)memory_limit / 1048576.0);
        exit(EXIT_FAILURE);
    }
    memory_used += bytes;
    if (memory_used > peak_memory) {
        peak_memory = memory_used;
    }
}

static void *budget_calloc(size_t count, size_t size)
{
    if (count != 0 && size > SIZE_MAX / count) {
        die("allocation size overflow");
    }
    const size_t bytes = count * size;
    account_allocation(bytes);
    void *result = calloc(count, size);
    if (result == NULL) {
        memory_used -= bytes;
        die("memory allocation failed");
    }
    return result;
}

static void budget_free(void *pointer, size_t bytes)
{
    free(pointer);
    if (bytes > memory_used) {
        die("internal memory accounting error");
    }
    memory_used -= bytes;
}

static uint64_t mix64(uint64_t value)
{
    value ^= value >> 30;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27;
    value *= UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31);
}

static StateBlock *map_block(StateMap *map, size_t index)
{
    const size_t block_index = index >> STATE_BLOCK_SHIFT;
    if (map->blocks == NULL || block_index >= map->block_count ||
        map->blocks[block_index].keys == NULL) {
        die("internal state-block index is invalid");
    }
    return &map->blocks[block_index];
}

static const StateBlock *map_const_block(const StateMap *map, size_t index)
{
    const size_t block_index = index >> STATE_BLOCK_SHIFT;
    if (map->blocks == NULL || block_index >= map->block_count ||
        map->blocks[block_index].keys == NULL) {
        die("internal state-block index is invalid");
    }
    return &map->blocks[block_index];
}

static uint32_t map_key_at(const StateMap *map, size_t index)
{
    const StateBlock *block = map_const_block(map, index);
    return block->keys[index & STATE_BLOCK_MASK];
}

static uint64_t map_value_at(const StateMap *map, size_t index)
{
    const StateBlock *block = map_const_block(map, index);
    return block->values[index & STATE_BLOCK_MASK];
}

static size_t bucket_count_for_expected(size_t expected)
{
    size_t result = 16;
    const size_t needed = expected / MAX_BUCKET_LOAD +
                          (expected % MAX_BUCKET_LOAD != 0);
    while (result < needed) {
        if (result > SIZE_MAX / 2U) {
            die("hash bucket count overflow");
        }
        result *= 2U;
    }
    return result;
}

static void map_init_common(StateMap *map, size_t expected,
                            bool store_values)
{
    *map = (StateMap){0};
    map->store_values = store_values;
    map->blocks = budget_calloc(MAX_STATE_BLOCKS, sizeof(*map->blocks));
    map->bucket_count = bucket_count_for_expected(expected);
    map->heads = budget_calloc(map->bucket_count, sizeof(*map->heads));
    memset(map->heads, 0xff,
           map->bucket_count * sizeof(*map->heads));
}

static void map_init(StateMap *map, size_t expected)
{
    map_init_common(map, expected, true);
}

static void map_allocate_block(StateMap *map)
{
    if (map->block_count == MAX_STATE_BLOCKS) {
        die("too many states for 32-bit chain indices");
    }
    StateBlock *block = &map->blocks[map->block_count];
    block->keys = budget_calloc(STATE_BLOCK_CAPACITY, sizeof(*block->keys));
    if (map->store_values) {
        block->values = budget_calloc(STATE_BLOCK_CAPACITY,
                                      sizeof(*block->values));
    }
    block->next = budget_calloc(STATE_BLOCK_CAPACITY, sizeof(*block->next));
    ++map->block_count;
}

static void map_destroy(StateMap *map)
{
    for (size_t block = 0; block < map->block_count; ++block) {
        budget_free(map->blocks[block].keys,
                    STATE_BLOCK_CAPACITY * sizeof(*map->blocks[block].keys));
        if (map->blocks[block].values != NULL) {
            budget_free(
                map->blocks[block].values,
                STATE_BLOCK_CAPACITY * sizeof(*map->blocks[block].values));
        }
        budget_free(map->blocks[block].next,
                    STATE_BLOCK_CAPACITY * sizeof(*map->blocks[block].next));
    }
    budget_free(map->heads, map->bucket_count * sizeof(*map->heads));
    budget_free(map->blocks, MAX_STATE_BLOCKS * sizeof(*map->blocks));
    *map = (StateMap){0};
}

static void map_rehash(StateMap *map)
{
    if (map->bucket_count > SIZE_MAX / 2U) {
        die("hash bucket count overflow");
    }
    const size_t new_count = 2U * map->bucket_count;
    uint32_t *heads = budget_calloc(new_count, sizeof(*heads));
    memset(heads, 0xff, new_count * sizeof(*heads));
    for (size_t index = 0; index < map->count; ++index) {
        StateBlock *block = map_block(map, index);
        const size_t offset = index & STATE_BLOCK_MASK;
        const size_t bucket =
            (size_t)mix64(block->keys[offset]) & (new_count - 1U);
        block->next[offset] = heads[bucket];
        heads[bucket] = (uint32_t)index;
    }
    budget_free(map->heads, map->bucket_count * sizeof(*map->heads));
    map->heads = heads;
    map->bucket_count = new_count;
}

static void map_insert_new(StateMap *map, uint32_t key, uint64_t value)
{
    if (map->count >= UINT32_MAX) {
        die("too many states for 32-bit chain indices");
    }
    if (map->count >= map->bucket_count * MAX_BUCKET_LOAD) {
        map_rehash(map);
    }
    if (map->count == map->block_count * STATE_BLOCK_CAPACITY) {
        map_allocate_block(map);
    }
    const size_t index = map->count;
    const size_t offset = index & STATE_BLOCK_MASK;
    StateBlock *block = map_block(map, index);
    const size_t bucket = (size_t)mix64(key) & (map->bucket_count - 1U);
    block->keys[offset] = key;
    if (map->store_values) {
        block->values[offset] = value;
    }
    block->next[offset] = map->heads[bucket];
    map->heads[bucket] = (uint32_t)index;
    ++map->count;
}

static bool map_get(const StateMap *map, uint32_t key, uint64_t *value)
{
    const size_t bucket = (size_t)mix64(key) & (map->bucket_count - 1U);
    uint32_t index = map->heads[bucket];
    while (index != UINT32_MAX) {
        const StateBlock *block = map_const_block(map, index);
        const size_t offset = index & STATE_BLOCK_MASK;
        if (block->keys[offset] == key) {
            if (value != NULL) {
                *value = block->values[offset];
            }
            return true;
        }
        index = block->next[offset];
    }
    return false;
}

static bool map_add_existing(StateMap *map, uint32_t key, uint64_t amount)
{
    const size_t bucket = (size_t)mix64(key) & (map->bucket_count - 1U);
    uint32_t index = map->heads[bucket];
    while (index != UINT32_MAX) {
        StateBlock *block = map_block(map, index);
        const size_t offset = index & STATE_BLOCK_MASK;
        if (block->keys[offset] == key) {
            if (UINT64_MAX - block->values[offset] < amount) {
                die("middle-frontier count overflowed uint64_t");
            }
            block->values[offset] += amount;
            return true;
        }
        index = block->next[offset];
    }
    return false;
}

static void make_context(DivisibilityContext *context, unsigned n)
{
    memset(context, 0, sizeof(*context));
    context->n = n;
    context->total_sum = n * (n + 1U) / 2U;
    context->full_mask = n == 32 ? UINT32_MAX :
                         (n == 0 ? 0 : (UINT32_C(1) << n) - 1U);
    context->mask_byte_count = (uint8_t)((n + 7U) / 8U);

    for (unsigned byte = 0; byte < context->mask_byte_count; ++byte) {
        for (unsigned pattern = 0; pattern < 256; ++pattern) {
            unsigned sum = 0;
            for (unsigned bit = 0; bit < 8; ++bit) {
                const unsigned value = 8U * byte + bit + 1U;
                if (value <= n && (pattern & (1U << bit)) != 0) {
                    sum += value;
                }
            }
            context->byte_sums[byte][pattern] = (uint16_t)sum;
        }
    }

    bool prime[MAX_N + 1];
    for (unsigned p = 0; p <= MAX_N; ++p) {
        prime[p] = p >= 2U;
    }
    for (unsigned p = 2; p <= MAX_N / p; ++p) {
        if (prime[p]) {
            for (unsigned multiple = p * p; multiple <= MAX_N;
                 multiple += p) {
                prime[multiple] = false;
            }
        }
    }
    for (unsigned p = 2; p <= n; ++p) {
        if (!prime[p]) {
            continue;
        }
        for (unsigned power = p; power <= n; ) {
            const unsigned level = context->power_count[p];
            if (level == MAX_PRIME_POWERS) {
                die("prime-power table is too short");
            }
            uint32_t mask = 0;
            for (unsigned value = power; value <= n; value += power) {
                mask |= UINT32_C(1) << (value - 1U);
            }
            context->power_masks[p][level] = mask;
            context->power_count[p] = (uint8_t)(level + 1U);
            if (power > n / p) {
                break;
            }
            power *= p;
        }
    }
    for (unsigned sum = 2; sum <= context->total_sum; ++sum) {
        unsigned remaining = sum;
        SumRequirement *requirement = &context->requirement[sum];
        for (unsigned p = 2; p <= remaining / p; ++p) {
            if (remaining % p != 0) {
                continue;
            }
            unsigned exponent = 0;
            do {
                remaining /= p;
                ++exponent;
            } while (remaining % p == 0);
            if (requirement->count == MAX_DISTINCT_FACTORS) {
                die("factor table is too short");
            }
            requirement->item[requirement->count++] =
                (PrimeRequirement){(uint16_t)p, (uint8_t)exponent};
        }
        if (remaining > 1U) {
            if (requirement->count == MAX_DISTINCT_FACTORS) {
                die("factor table is too short");
            }
            requirement->item[requirement->count++] =
                (PrimeRequirement){(uint16_t)remaining, 1};
        }
    }
}

static unsigned subset_sum(const DivisibilityContext *context, uint32_t mask)
{
    unsigned sum = 0;
    for (unsigned byte = 0; byte < context->mask_byte_count; ++byte) {
        sum += context->byte_sums[byte][mask & UINT32_C(255)];
        mask >>= 8;
    }
    return sum;
}

static bool subset_is_good(const DivisibilityContext *context,
                           uint32_t mask, unsigned sum)
{
    if (sum == 0) {
        return mask == 0;
    }
    const SumRequirement *requirement = &context->requirement[sum];
    for (unsigned i = 0; i < requirement->count; ++i) {
        const unsigned p = requirement->item[i].prime;
        const unsigned needed = requirement->item[i].exponent;
        if (p > context->n) {
            return false;
        }
        unsigned available = 0;
        for (unsigned level = 0;
             level < context->power_count[p] && available < needed; ++level) {
            available += (unsigned)__builtin_popcount(
                mask & context->power_masks[p][level]);
        }
        if (available < needed) {
            return false;
        }
    }
    return true;
}

static size_t middle_filter_word_count(const DivisibilityContext *context)
{
    return context->n <= 6 ? 1 :
        (size_t)UINT64_C(1) << (context->n - 6U);
}

static uint64_t *make_middle_extension_filter(
    const DivisibilityContext *context, const StateMap *forward)
{
    const size_t word_count = middle_filter_word_count(context);
    uint64_t *filter = budget_calloc(word_count, sizeof(*filter));
    for (size_t index = 0; index < forward->count; ++index) {
        const uint32_t mask = map_key_at(forward, index);
        uint32_t choices = context->full_mask ^ mask;
        while (choices != 0) {
            const uint32_t bit = choices & (UINT32_C(0) - choices);
            choices ^= bit;
            const uint32_t child = mask | bit;
            const size_t word = child >> 6;
            if (word >= word_count) {
                budget_free(filter, word_count * sizeof(*filter));
                die("middle-filter index is out of range");
            }
            filter[word] |= UINT64_C(1) << (child & 63U);
        }
    }
    return filter;
}

static bool middle_filter_contains(const uint64_t *filter, uint32_t mask)
{
    return (filter[mask >> 6] &
            (UINT64_C(1) << (mask & 63U))) != 0;
}

static size_t middle_filter_count(const uint64_t *filter,
                                  size_t word_count)
{
    size_t count = 0;
    for (size_t word = 0; word < word_count; ++word) {
        count += (size_t)__builtin_popcountll(filter[word]);
    }
    return count;
}

static StateMap expand_forward(const DivisibilityContext *context,
                               const StateMap *current,
                               U128 *transitions)
{
    StateMap next;
    map_init(&next, current->count == 0 ? 1 : current->count);
    for (size_t index = 0; index < current->count; ++index) {
        const uint32_t mask = map_key_at(current, index);
        const uint64_t value = map_value_at(current, index);
        const unsigned old_sum = subset_sum(context, mask);
        uint32_t choices = context->full_mask ^ mask;
        while (choices != 0) {
            const unsigned bit_index = (unsigned)__builtin_ctz(choices);
            const uint32_t bit = UINT32_C(1) << bit_index;
            choices ^= bit;
            const uint32_t child = mask | bit;
            if (!map_add_existing(&next, child, value) &&
                subset_is_good(context, child,
                               old_sum + bit_index + 1U)) {
                map_insert_new(&next, child, value);
            }
            if (!add_u128(transitions, 1)) {
                die("transition counter overflowed");
            }
        }
    }
    return next;
}

static StateMap expand_backward(const DivisibilityContext *context,
                                const StateMap *current,
                                const StateMap *forward_middle,
                                const uint64_t *allowed_middle_extensions,
                                unsigned target_depth, unsigned middle,
                                U128 *transitions)
{
    StateMap next;
    map_init(&next, current->count == 0 ? 1 : current->count);
    for (size_t index = 0; index < current->count; ++index) {
        const uint32_t mask = map_key_at(current, index);
        const uint64_t value = map_value_at(current, index);
        const unsigned old_sum = subset_sum(context, mask);
        uint32_t choices = mask;
        while (choices != 0) {
            const unsigned bit_index = (unsigned)__builtin_ctz(choices);
            const uint32_t bit = UINT32_C(1) << bit_index;
            choices ^= bit;
            const uint32_t child = mask ^ bit;

            if (!map_add_existing(&next, child, value)) {
                bool keep;
                if (target_depth == middle) {
                    keep = map_get(forward_middle, child, NULL);
                } else {
                    keep = subset_is_good(context, child,
                                          old_sum - bit_index - 1U);
                    if (keep && target_depth == middle + 1U) {
                        if (allowed_middle_extensions == NULL) {
                            die("missing middle-extension filter");
                        }
                        keep = middle_filter_contains(
                            allowed_middle_extensions, child);
                    }
                }
                if (keep) {
                    map_insert_new(&next, child, value);
                }
            }
            if (!add_u128(transitions, 1)) {
                die("transition counter overflowed");
            }
        }
    }
    return next;
}

static U128 join_middle(const StateMap *forward, const StateMap *backward,
                        size_t *matches)
{
    U128 answer = 0;
    *matches = 0;
    for (size_t index = 0; index < backward->count; ++index) {
        const uint32_t key = map_key_at(backward, index);
        uint64_t left;
        if (!map_get(forward, key, &left)) {
            die("backward intersection contains a non-forward state");
        }
        const U128 contribution =
            (U128)left * map_value_at(backward, index);
        if (!add_u128(&answer, contribution)) {
            die("final answer overflowed 128 bits");
        }
        ++*matches;
    }
    return answer;
}

static U128 compute_term(unsigned n)
{
    if (n == 0) {
        return 1;
    }
    DivisibilityContext context;
    make_context(&context, n);
    if (!subset_is_good(&context, context.full_mask, context.total_sum)) {
        if (!quiet) {
            fprintf(stderr,
                    "A291551(%u): 0 (final sum %u does not divide n!)\n",
                    n, context.total_sum);
        }
        return 0;
    }

    const unsigned middle = (n + 1U) / 2U;
    const double started = monotonic_seconds();
    U128 transitions = 0;

    StateMap forward;
    map_init(&forward, 1);
    map_insert_new(&forward, 0, 1);
    for (unsigned depth = 0; depth < middle; ++depth) {
        const double layer_started = monotonic_seconds();
        StateMap next = expand_forward(&context, &forward, &transitions);
        map_destroy(&forward);
        forward = next;
        if (!quiet) {
            fprintf(stderr,
                    "A291551(%u): F[%u] = %zu states (%.3f s)\n",
                    n, depth + 1U, forward.count,
                    monotonic_seconds() - layer_started);
        }
        if (forward.count == 0) {
            map_destroy(&forward);
            return 0;
        }
    }

    StateMap backward;
    map_init(&backward, 1);
    map_insert_new(&backward, context.full_mask, 1);
    for (unsigned depth = n; depth > middle; --depth) {
        uint64_t *allowed = NULL;
        size_t allowed_word_count = 0;
        if (depth - 1U == middle + 1U) {
            const double filter_started = monotonic_seconds();
            allowed_word_count = middle_filter_word_count(&context);
            allowed = make_middle_extension_filter(&context, &forward);
            if (!quiet) {
                fprintf(stderr,
                        "A291551(%u): middle+1 filter = %zu masks "
                        "(%.3f s)\n",
                        n, middle_filter_count(allowed,
                                               allowed_word_count),
                        monotonic_seconds() - filter_started);
            }
        }
        const double layer_started = monotonic_seconds();
        StateMap next = expand_backward(&context, &backward, &forward,
                                        allowed, depth - 1U,
                                        middle, &transitions);
        if (allowed != NULL) {
            budget_free(allowed,
                        allowed_word_count * sizeof(*allowed));
        }
        map_destroy(&backward);
        backward = next;
        if (!quiet) {
            fprintf(stderr,
                    "A291551(%u): B[%u] = %zu states (%.3f s)\n",
                    n, depth - 1U, backward.count,
                    monotonic_seconds() - layer_started);
        }
        if (backward.count == 0) {
            map_destroy(&forward);
            map_destroy(&backward);
            return 0;
        }
    }

    size_t matches;
    const U128 answer = join_middle(&forward, &backward, &matches);
    const size_t forward_count = forward.count;
    const size_t backward_count = backward.count;
    map_destroy(&forward);
    map_destroy(&backward);

    if (!quiet) {
        char transition_text[40];
        u128_to_text(transitions, transition_text);
        fprintf(stderr,
                "A291551(%u): fixed join at %u, F=%zu, B=%zu, "
                "matches=%zu, transitions=%s, time=%.3f s, peak=%.2f MiB\n",
                n, middle, forward_count, backward_count, matches,
                transition_text, monotonic_seconds() - started,
                (double)peak_memory / 1048576.0);
    }
    return answer;
}

static void verify_known(unsigned n, U128 value)
{
    if (n > KNOWN_MAX_N) {
        return;
    }
    char text[40];
    u128_to_text(value, text);
    if (strcmp(text, known_terms[n]) != 0) {
        fprintf(stderr,
                "error: A291551(%u) computed as %s, expected %s\n",
                n, text, known_terms[n]);
        exit(EXIT_FAILURE);
    }
}

static void usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s [MAX_N] [options]\n"
            "       %s --term N [options]\n"
            "Options:\n"
            "  -n, --max N              compute a(0)..a(N) (default: %d)\n"
            "  -t, --term N             compute only a(N)\n"
            "  -m, --memory-limit-mib M memory budget (default: %d)\n"
            "  -q, --quiet              suppress progress on stderr\n"
            "  -c, --check              recompute known a(0)..a(%d)\n"
            "  -h, --help               show this help\n"
            "Completed terms are saved in %s.\n",
            program, program, DEFAULT_MAX_N, DEFAULT_MEMORY_MIB,
            KNOWN_MAX_N, bfile_path);
}

int main(int argc, char **argv)
{
    unsigned maximum = DEFAULT_MAX_N;
    unsigned single_term = 0;
    unsigned memory_mib = DEFAULT_MEMORY_MIB;
    bool term_mode = false;
    bool check_mode = false;
    bool maximum_seen = false;

    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if ((!strcmp(text, "-n") || !strcmp(text, "--max")) &&
            argument + 1 < argc) {
            maximum = parse_bounded(argv[++argument], 0, MAX_N, "maximum N");
            maximum_seen = true;
        } else if ((!strcmp(text, "-t") || !strcmp(text, "--term")) &&
                   argument + 1 < argc) {
            single_term = parse_bounded(argv[++argument], 0, MAX_N, "term N");
            term_mode = true;
        } else if ((!strcmp(text, "-m") ||
                    !strcmp(text, "--memory-limit-mib")) &&
                   argument + 1 < argc) {
            memory_mib = parse_bounded(argv[++argument], MIN_MEMORY_MIB,
                                       MAX_MEMORY_MIB, "memory limit MiB");
        } else if (!strcmp(text, "-q") || !strcmp(text, "--quiet")) {
            quiet = true;
        } else if (!strcmp(text, "-c") || !strcmp(text, "--check")) {
            check_mode = true;
            maximum = KNOWN_MAX_N;
        } else if (!strcmp(text, "-h") || !strcmp(text, "--help")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (text[0] != '-' && !maximum_seen) {
            maximum = parse_bounded(text, 0, MAX_N, "maximum N");
            maximum_seen = true;
        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (term_mode && maximum_seen) {
        die("--term cannot be combined with a maximum N");
    }
    if (check_mode && (term_mode || maximum_seen)) {
        die("--check cannot be combined with --term or a maximum N");
    }
    if ((size_t)memory_mib > SIZE_MAX / (size_t)1048576U) {
        die("memory limit conversion overflow");
    }
    memory_limit = (size_t)memory_mib * (size_t)1048576U;

    if (term_mode) {
        U128 value;
        if (load_saved_term(single_term, &value)) {
            if (!quiet) {
                fprintf(stderr, "A291551(%u): loaded from %s\n",
                        single_term, bfile_path);
            }
        } else {
            peak_memory = memory_used = 0;
            value = compute_term(single_term);
            verify_known(single_term, value);
            record_term(single_term, value);
        }
        verify_known(single_term, value);
        print_term(single_term, value);
    } else {
        for (unsigned n = 0; n <= maximum; ++n) {
            U128 value;
            if (!check_mode && load_saved_term(n, &value)) {
                if (!quiet) {
                    fprintf(stderr, "A291551(%u): loaded from %s\n",
                            n, bfile_path);
                }
            } else {
                peak_memory = memory_used = 0;
                value = compute_term(n);
                verify_known(n, value);
                record_term(n, value);
            }
            verify_known(n, value);
            print_term(n, value);
        }
        if (check_mode && !quiet) {
            fprintf(stderr, "A291551: independent a(0)..a(%d) check passed\n",
                    KNOWN_MAX_N);
        }
    }
    if (memory_used != 0) {
        die("internal memory leak detected");
    }
    return EXIT_SUCCESS;
}
