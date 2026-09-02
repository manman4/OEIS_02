/*
 * A291551
 *
 * Number of permutations s_1,...,s_n of 1,...,n for which every prefix
 * sum divides the corresponding prefix product.
 *
 * For a subset S of {1,...,n}, put
 *
 *     good(S) := sum(S) divides product(S).
 *
 * The condition at a prefix depends only on its set of entries, not on their
 * order.  Thus, if F(S) is the number of valid orders ending at S,
 *
 *     F(empty) = 1,
 *     F(S) = good(S) * sum_{x in S} F(S - {x}).
 *
 * A dense implementation needs O(2^n) memory.  This program instead keeps
 * only reachable states and runs the recurrence from both ends.  Forward
 * counts F(S) and backward counts B(S) are joined at an adaptively selected
 * cardinality:
 *
 *     a(n) = sum_{|S|=k} F(S) B(S).
 *
 * The two frontiers are expanded one layer at a time; the side with the
 * smaller estimated transition count is selected.  This normally uses much
 * less time and memory than either one-way sparse DP.
 *
 * No prefix product is ever formed.  To test q | product(S), q is factored
 * and v_p(product(S)) is obtained as
 *
 *     sum_{r>=1} popcount(S & {multiples of p^r}).
 *
 * Hence the divisibility test cannot overflow.  Counts use unsigned 128-bit
 * integers.  MAX_N is 34 because 34! fits in 128 bits, which proves that all
 * intermediate path counts and the answer fit; additions and the final
 * products are checked anyway.  States are densely stored in fixed-size
 * blocks and linked from a compact bucket array.  Growing a layer therefore
 * never duplicates or copies its large state payload.  Complete masks are
 * compared, so hash collisions affect speed only, never correctness.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       291551_01.c -o 291551_01
 *
 * On compilers and targets that support it, -march=native may be added as an
 * optional machine-specific optimization.  It is not required for correctness.
 *
 * Usage examples:
 *   ./291551_01 30                  # print a(0),...,a(30)
 *   ./291551_01 --term 29           # print only a(29)
 *   ./291551_01 --check             # recompute and verify known terms
 *   ./291551_01 --term 29 -m 8192   # allow 8192 MiB for hash tables
 *
 * Every completed term is printed in OEIS b-file format and atomically saved
 * in b291551_01.txt.  A normal rerun reuses saved terms; --check recomputes
 * them.  Progress goes to stderr.
 */

#define _POSIX_C_SOURCE 200809L

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
#error "291551_01.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 34
#define DEFAULT_MAX_N 30
#define DEFAULT_MEMORY_MIB 4096
#define MIN_MEMORY_MIB 16
#define MAX_MEMORY_MIB 65536
#define MAX_DISTINCT_FACTORS 5
#define MAX_PRIME_POWERS 6
#define MASK_BYTE_COUNT ((MAX_N + 7) / 8)
#define STATE_BLOCK_SHIFT 18
#define STATE_BLOCK_CAPACITY (UINT32_C(1) << STATE_BLOCK_SHIFT)
#define STATE_BLOCK_MASK (STATE_BLOCK_CAPACITY - 1U)
#define MAX_STATE_BLOCKS \
    ((size_t)(UINT64_C(1) << 32) / STATE_BLOCK_CAPACITY)
#define MAX_BUCKET_LOAD 2U

_Static_assert(MAX_N < 63, "subset mask must fit in uint64_t");
_Static_assert(MAX_N * (MAX_N + 1) / 2 <= UINT16_MAX,
               "subset sum must fit in uint16_t");

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
    uint64_t full_mask;
    uint64_t power_masks[MAX_N + 1][MAX_PRIME_POWERS];
    uint8_t power_count[MAX_N + 1];
    uint16_t byte_sums[MASK_BYTE_COUNT][256];
    uint8_t mask_byte_count;
    SumRequirement requirement[MAX_N * (MAX_N + 1) / 2 + 1];
} DivisibilityContext;

typedef struct {
    uint64_t *keys;
    U128 *values;
    uint32_t *next;
} StateBlock;

typedef struct {
    StateBlock *blocks;
    uint32_t *heads;
    size_t bucket_count;
    size_t block_count;
    size_t count;
} StateMap;

static size_t memory_limit;
static size_t memory_used;
static size_t peak_memory;
static bool quiet;
static const char *const bfile_path = "b291551_01.txt";
static const char *const bfile_lock_path = "b291551_01.txt.lock";

static const char *const known_terms[DEFAULT_MAX_N + 1] = {
    "1", "1", "0", "0", "0", "0", "0", "0", "0", "0", "0",
    "0", "0", "0", "0", "26", "0", "262", "0", "10226",
    "43964", "139484", "0", "13936472", "59652396", "301235944",
    "1915640632", "7969506364", "0", "465804291196", "0"
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

static void print_u128(FILE *stream, U128 value)
{
    char digits[40];
    size_t length = 0;
    do {
        digits[length++] = (char)('0' + (unsigned)(value % 10U));
        value /= 10U;
    } while (value != 0);
    while (length != 0) {
        if (fputc(digits[--length], stream) == EOF) {
            die("cannot write output");
        }
    }
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

static int lock_bfile(void)
{
    const int descriptor = open(bfile_lock_path, O_RDWR | O_CREAT, 0666);
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
        char number[40];
        char extra;
        if (sscanf(cursor, "%u %39s %c", &index, number, &extra) != 2 ||
            index > MAX_N) {
            fclose(input);
            die("b-file is malformed");
        }
        U128 parsed;
        if (!parse_u128(number, &parsed) || present[index]) {
            fclose(input);
            die("b-file has an invalid or duplicate term");
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
        die("saved b-file term is invalid");
    }
    unlock_bfile(lock_descriptor);
    return found;
}

static void record_term(unsigned n, U128 value)
{
    bool present[MAX_N + 1];
    char values[MAX_N + 1][40];
    char value_text[40];
    u128_to_text(value, value_text);

    const int lock_descriptor = lock_bfile();
    read_bfile(present, values);
    if (present[n]) {
        if (strcmp(values[n], value_text) != 0) {
            unlock_bfile(lock_descriptor);
            die("computed term disagrees with b-file");
        }
        unlock_bfile(lock_descriptor);
        return;
    }
    present[n] = true;
    strcpy(values[n], value_text);

    char temporary[] = "b291551_01.txt.tmp.XXXXXX";
    const int temporary_descriptor = mkstemp(temporary);
    if (temporary_descriptor < 0) {
        unlock_bfile(lock_descriptor);
        die("cannot create temporary b-file");
    }
    if (fchmod(temporary_descriptor, 0644) != 0) {
        close(temporary_descriptor);
        unlink(temporary);
        unlock_bfile(lock_descriptor);
        die("cannot set temporary b-file permissions");
    }
    FILE *output = fdopen(temporary_descriptor, "w");
    if (output == NULL) {
        close(temporary_descriptor);
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

static bool add_u128(U128 *destination, U128 amount)
{
    const U128 old = *destination;
    *destination += amount;
    return *destination >= old;
}

static bool multiply_u128(U128 left, U128 right, U128 *product)
{
    const U128 maximum = ~(U128)0;
    if (left != 0 && right > maximum / left) {
        return false;
    }
    *product = left * right;
    return true;
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

static size_t bucket_count_for_expected(size_t expected)
{
    size_t bucket_count = 16;
    const size_t needed = expected / MAX_BUCKET_LOAD +
                          (expected % MAX_BUCKET_LOAD != 0);
    while (bucket_count < needed) {
        if (bucket_count > SIZE_MAX / 2U) {
            die("hash bucket count overflow");
        }
        bucket_count *= 2U;
    }
    return bucket_count;
}

static StateBlock *map_block(StateMap *map, size_t index)
{
    const size_t block_index = index >> STATE_BLOCK_SHIFT;
    if (map->blocks == NULL || block_index >= map->block_count ||
        map->blocks[block_index].keys == NULL ||
        map->blocks[block_index].values == NULL ||
        map->blocks[block_index].next == NULL) {
        die("internal state-block index is out of range");
    }
    return &map->blocks[block_index];
}

static const StateBlock *map_const_block(const StateMap *map, size_t index)
{
    const size_t block_index = index >> STATE_BLOCK_SHIFT;
    if (map->blocks == NULL || block_index >= map->block_count ||
        map->blocks[block_index].keys == NULL ||
        map->blocks[block_index].values == NULL ||
        map->blocks[block_index].next == NULL) {
        die("internal state-block index is out of range");
    }
    return &map->blocks[block_index];
}

static uint64_t map_key_at(const StateMap *map, size_t index)
{
    const StateBlock *block = map_const_block(map, index);
    return block->keys[index & STATE_BLOCK_MASK];
}

static U128 map_value_at(const StateMap *map, size_t index)
{
    const StateBlock *block = map_const_block(map, index);
    return block->values[index & STATE_BLOCK_MASK];
}

static void map_set_next(StateMap *map, size_t index, uint32_t next)
{
    StateBlock *block = map_block(map, index);
    block->next[index & STATE_BLOCK_MASK] = next;
}

static uint32_t map_next_at(const StateMap *map, size_t index)
{
    const StateBlock *block = map_const_block(map, index);
    return block->next[index & STATE_BLOCK_MASK];
}

static void map_init(StateMap *map, size_t expected)
{
    *map = (StateMap){0};
    map->blocks = budget_calloc(MAX_STATE_BLOCKS, sizeof(*map->blocks));
    map->bucket_count = bucket_count_for_expected(expected);
    map->heads = budget_calloc(map->bucket_count, sizeof(*map->heads));
    memset(map->heads, 0xff,
           map->bucket_count * sizeof(*map->heads));
}

static void map_allocate_block(StateMap *map)
{
    if (map->block_count == MAX_STATE_BLOCKS) {
        die("too many DP states for 32-bit chain indices");
    }
    StateBlock *block = &map->blocks[map->block_count];
    block->keys = budget_calloc(STATE_BLOCK_CAPACITY,
                                sizeof(*block->keys));
    block->values = budget_calloc(STATE_BLOCK_CAPACITY,
                                  sizeof(*block->values));
    block->next = budget_calloc(STATE_BLOCK_CAPACITY,
                                sizeof(*block->next));
    ++map->block_count;
}

static void map_destroy(StateMap *map)
{
    for (size_t block = 0; block < map->block_count; ++block) {
        budget_free(map->blocks[block].keys,
                    STATE_BLOCK_CAPACITY * sizeof(*map->blocks[block].keys));
        budget_free(map->blocks[block].values,
                    STATE_BLOCK_CAPACITY * sizeof(*map->blocks[block].values));
        budget_free(map->blocks[block].next,
                    STATE_BLOCK_CAPACITY * sizeof(*map->blocks[block].next));
    }
    if (map->heads != NULL) {
        budget_free(map->heads,
                    map->bucket_count * sizeof(*map->heads));
    }
    if (map->blocks != NULL) {
        budget_free(map->blocks,
                    MAX_STATE_BLOCKS * sizeof(*map->blocks));
    }
    *map = (StateMap){0};
}

static void map_rehash(StateMap *map)
{
    if (map->bucket_count > SIZE_MAX / 2U) {
        die("hash bucket count overflow");
    }
    const size_t new_count = map->bucket_count * 2U;
    if (new_count < 16 || new_count <= map->bucket_count) {
        die("invalid enlarged hash bucket count");
    }
    uint32_t *new_heads = budget_calloc(new_count, sizeof(*new_heads));
    if (new_heads == NULL) {
        die("hash bucket allocation returned null");
    }
    memset(new_heads, 0xff, new_count * sizeof(*new_heads));

    for (size_t index = 0; index < map->count; ++index) {
        const uint64_t key = map_key_at(map, index);
        const size_t bucket = (size_t)mix64(key) & (new_count - 1U);
        map_set_next(map, index, new_heads[bucket]);
        new_heads[bucket] = (uint32_t)index;
    }
    budget_free(map->heads,
                map->bucket_count * sizeof(*map->heads));
    map->heads = new_heads;
    map->bucket_count = new_count;
}

static void map_insert_new(StateMap *map, uint64_t key, U128 value)
{
    if (map->heads == NULL || map->bucket_count == 0) {
        die("internal hash bucket table is missing");
    }
    if (map->count >= UINT32_MAX) {
        die("too many DP states for 32-bit chain indices");
    }
    if (map->count >= map->bucket_count * MAX_BUCKET_LOAD) {
        map_rehash(map);
    }
    if (map->heads == NULL || map->bucket_count < 16) {
        die("internal hash bucket table became invalid");
    }
    if (map->count == map->block_count * STATE_BLOCK_CAPACITY) {
        map_allocate_block(map);
    }

    const size_t index = map->count;
    const size_t offset = index & STATE_BLOCK_MASK;
    StateBlock *block = map_block(map, index);
    const size_t bucket = (size_t)mix64(key) & (map->bucket_count - 1U);
    block->keys[offset] = key;
    block->values[offset] = value;
    block->next[offset] = map->heads[bucket];
    map->heads[bucket] = (uint32_t)index;
    ++map->count;
}

static bool map_get(const StateMap *map, uint64_t key, U128 *value)
{
    const size_t bucket = (size_t)mix64(key) & (map->bucket_count - 1U);
    uint32_t index = map->heads[bucket];
    while (index != UINT32_MAX) {
        if (map_key_at(map, index) == key) {
            *value = map_value_at(map, index);
            return true;
        }
        index = map_next_at(map, index);
    }
    return false;
}

static void make_divisibility_context(DivisibilityContext *context,
                                      unsigned n)
{
    memset(context, 0, sizeof(*context));
    context->n = n;
    context->total_sum = n * (n + 1U) / 2U;
    context->full_mask = n == 0 ? 0 : (UINT64_C(1) << n) - 1U;
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
        if (!prime[p]) {
            continue;
        }
        for (unsigned multiple = p * p; multiple <= MAX_N;
             multiple += p) {
            prime[multiple] = false;
        }
    }

    for (unsigned p = 2; p <= n; ++p) {
        if (!prime[p]) {
            continue;
        }
        unsigned power = p;
        while (power <= n) {
            const unsigned level = context->power_count[p];
            if (level == MAX_PRIME_POWERS) {
                die("internal prime-power table is too short");
            }
            uint64_t mask = 0;
            for (unsigned value = power; value <= n; value += power) {
                mask |= UINT64_C(1) << (value - 1U);
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
                die("internal factor table is too short");
            }
            requirement->item[requirement->count++] =
                (PrimeRequirement){(uint16_t)p, (uint8_t)exponent};
        }
        if (remaining > 1U) {
            if (requirement->count == MAX_DISTINCT_FACTORS) {
                die("internal factor table is too short");
            }
            requirement->item[requirement->count++] =
                (PrimeRequirement){(uint16_t)remaining, 1};
        }
    }
}

static unsigned subset_sum(const DivisibilityContext *context,
                           uint64_t mask)
{
    unsigned sum = 0;
    for (unsigned byte = 0; byte < context->mask_byte_count; ++byte) {
        sum += context->byte_sums[byte][mask & UINT64_C(255)];
        mask >>= 8;
    }
    return sum;
}

static bool subset_is_good(const DivisibilityContext *context,
                           uint64_t mask, unsigned sum)
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
        const unsigned levels = context->power_count[p];
        for (unsigned level = 0;
             level < levels && available < needed; ++level) {
            available += (unsigned)__builtin_popcountll(
                mask & context->power_masks[p][level]);
        }
        if (available < needed) {
            return false;
        }
    }
    return true;
}

/* Existing entries have already passed subset_is_good, so duplicate
   transitions need no repeated divisibility test. */
static void map_add_good(StateMap *map,
                         const DivisibilityContext *context,
                         uint64_t key, unsigned sum, U128 amount)
{
    if (sum > context->total_sum) {
        die("internal subset sum is out of range");
    }
    const size_t bucket = (size_t)mix64(key) & (map->bucket_count - 1U);
    uint32_t index = map->heads[bucket];
    while (index != UINT32_MAX) {
        StateBlock *block = map_block(map, index);
        const size_t offset = index & STATE_BLOCK_MASK;
        if (block->keys[offset] == key) {
            if (!add_u128(&block->values[offset], amount)) {
                die("DP count overflowed 128 bits");
            }
            return;
        }
        index = block->next[offset];
    }
    if (!subset_is_good(context, key, sum)) {
        return;
    }
    map_insert_new(map, key, amount);
}

static size_t expected_next_size(const StateMap *current)
{
    if (current->count == 0) {
        return 1;
    }
    /* This sizes only the compact bucket-head array.  Dense state storage is
       allocated in fixed blocks as accepted states actually arrive. */
    return current->count;
}

static StateMap expand_forward(const DivisibilityContext *context,
                               const StateMap *current,
                               U128 *transition_count)
{
    StateMap next;
    map_init(&next, expected_next_size(current));
    const uint64_t full = context->full_mask;
    for (size_t index = 0; index < current->count; ++index) {
        const uint64_t mask = map_key_at(current, index);
        const U128 value = map_value_at(current, index);
        const unsigned old_sum = subset_sum(context, mask);
        uint64_t choices = full ^ mask;
        while (choices != 0) {
            const unsigned bit_index = (unsigned)__builtin_ctzll(choices);
            const uint64_t bit = UINT64_C(1) << bit_index;
            choices ^= bit;
            const unsigned sum = old_sum + bit_index + 1U;
            map_add_good(&next, context, mask | bit, sum, value);
            if (!add_u128(transition_count, 1)) {
                die("transition counter overflowed 128 bits");
            }
        }
    }
    return next;
}

static StateMap expand_backward(const DivisibilityContext *context,
                                const StateMap *current,
                                U128 *transition_count)
{
    StateMap next;
    map_init(&next, expected_next_size(current));
    for (size_t index = 0; index < current->count; ++index) {
        const uint64_t mask = map_key_at(current, index);
        const U128 value = map_value_at(current, index);
        const unsigned old_sum = subset_sum(context, mask);
        uint64_t choices = mask;
        while (choices != 0) {
            const unsigned bit_index = (unsigned)__builtin_ctzll(choices);
            const uint64_t bit = UINT64_C(1) << bit_index;
            choices ^= bit;
            const unsigned sum = old_sum - bit_index - 1U;
            map_add_good(&next, context, mask ^ bit, sum, value);
            if (!add_u128(transition_count, 1)) {
                die("transition counter overflowed 128 bits");
            }
        }
    }
    return next;
}

static U128 work_estimate(size_t states, unsigned choices)
{
    return (U128)states * choices;
}

static U128 join_maps(const StateMap *left, const StateMap *right,
                      size_t *matches)
{
    const StateMap *scan = left->count <= right->count ? left : right;
    const StateMap *lookup = scan == left ? right : left;
    U128 answer = 0;
    *matches = 0;
    for (size_t index = 0; index < scan->count; ++index) {
        const uint64_t key = map_key_at(scan, index);
        const U128 value = map_value_at(scan, index);
        U128 other;
        if (!map_get(lookup, key, &other)) {
            continue;
        }
        U128 contribution;
        if (!multiply_u128(value, other, &contribution) ||
            !add_u128(&answer, contribution)) {
            die("final count overflowed 128 bits");
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
    make_divisibility_context(&context, n);

    /* The final prefix is always {1,...,n}; this inexpensive necessary test
       proves all zero terms without constructing a DP table. */
    if (!subset_is_good(&context, context.full_mask, context.total_sum)) {
        if (!quiet) {
            fprintf(stderr,
                    "A291551(%u): 0 (final sum %u does not divide n!)\n",
                    n, context.total_sum);
        }
        return 0;
    }

    const double started = monotonic_seconds();
    StateMap forward, backward;
    map_init(&forward, 1);
    map_insert_new(&forward, 0, 1);
    map_init(&backward, 1);
    map_insert_new(&backward, context.full_mask, 1);

    unsigned forward_depth = 0;
    unsigned backward_depth = n;
    U128 transitions = 0;

    while (forward_depth < backward_depth) {
        const U128 forward_work =
            work_estimate(forward.count, n - forward_depth);
        const U128 backward_work =
            work_estimate(backward.count, backward_depth);
        const bool take_forward = forward_work <= backward_work;
        const double layer_started = monotonic_seconds();

        if (take_forward) {
            StateMap next = expand_forward(&context, &forward, &transitions);
            map_destroy(&forward);
            forward = next;
            ++forward_depth;
            if (!quiet) {
                fprintf(stderr,
                        "A291551(%u): F[%u] = %zu states (%.3f s)\n",
                        n, forward_depth, forward.count,
                        monotonic_seconds() - layer_started);
            }
        } else {
            StateMap next = expand_backward(&context, &backward,
                                            &transitions);
            map_destroy(&backward);
            backward = next;
            --backward_depth;
            if (!quiet) {
                fprintf(stderr,
                        "A291551(%u): B[%u] = %zu states (%.3f s)\n",
                        n, backward_depth, backward.count,
                        monotonic_seconds() - layer_started);
            }
        }

        if (forward.count == 0 || backward.count == 0) {
            map_destroy(&forward);
            map_destroy(&backward);
            if (!quiet) {
                fprintf(stderr, "A291551(%u): frontier became empty\n", n);
            }
            return 0;
        }
    }

    size_t matches;
    const U128 answer = join_maps(&forward, &backward, &matches);
    const size_t forward_states = forward.count;
    const size_t backward_states = backward.count;
    map_destroy(&forward);
    map_destroy(&backward);

    if (!quiet) {
        char transition_text[40];
        u128_to_text(transitions, transition_text);
        fprintf(stderr,
                "A291551(%u): join at %u, F=%zu, B=%zu, matches=%zu, "
                "transitions=%s, time=%.3f s, peak=%.2f MiB\n",
                n, forward_depth, forward_states, backward_states, matches,
                transition_text, monotonic_seconds() - started,
                (double)peak_memory / 1048576.0);
    }
    return answer;
}

static void verify_known(unsigned n, U128 value)
{
    if (n > DEFAULT_MAX_N) {
        return;
    }
    char actual[40];
    u128_to_text(value, actual);
    if (strcmp(actual, known_terms[n]) != 0) {
        fprintf(stderr,
                "error: A291551(%u) computed as %s, expected %s\n",
                n, actual, known_terms[n]);
        exit(EXIT_FAILURE);
    }
}

static void print_term(unsigned n, U128 value)
{
    if (printf("%u ", n) < 0) {
        die("cannot write output");
    }
    print_u128(stdout, value);
    if (putchar('\n') == EOF || fflush(stdout) != 0) {
        die("cannot write output");
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
            "  -m, --memory-limit-mib M hash-table budget (default: %d)\n"
            "  -q, --quiet              suppress progress on stderr\n"
            "  -c, --check              recompute a(0)..a(%d) and verify\n"
            "  -h, --help               show this help\n"
            "Completed terms are atomically saved in %s.\n",
            program, program, DEFAULT_MAX_N, DEFAULT_MEMORY_MIB,
            DEFAULT_MAX_N, bfile_path);
}

int main(int argc, char **argv)
{
    unsigned maximum = DEFAULT_MAX_N;
    unsigned single_term = 0;
    unsigned memory_mib = DEFAULT_MEMORY_MIB;
    bool term_mode = false;
    bool check_mode = false;
    bool positional_seen = false;
    bool maximum_option_seen = false;

    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if ((!strcmp(text, "-n") || !strcmp(text, "--max")) &&
            argument + 1 < argc) {
            maximum = parse_bounded(argv[++argument], 0, MAX_N, "maximum N");
            maximum_option_seen = true;
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
            maximum = DEFAULT_MAX_N;
        } else if (!strcmp(text, "-h") || !strcmp(text, "--help")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (text[0] != '-' && !positional_seen) {
            maximum = parse_bounded(text, 0, MAX_N, "maximum N");
            positional_seen = true;
        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (term_mode && (positional_seen || maximum_option_seen)) {
        die("--term cannot be combined with a maximum N");
    }
    if (check_mode && (positional_seen || maximum_option_seen || term_mode)) {
        die("--check cannot be combined with --term or a maximum N");
    }

    if ((size_t)memory_mib > SIZE_MAX / (size_t)1048576U) {
        die("memory limit conversion overflow");
    }
    memory_limit = (size_t)memory_mib * (size_t)1048576U;

    if (term_mode) {
        peak_memory = memory_used = 0;
        U128 value;
        if (load_saved_term(single_term, &value)) {
            if (!quiet) {
                fprintf(stderr, "A291551(%u): loaded from %s\n",
                        single_term, bfile_path);
            }
        } else {
            value = compute_term(single_term);
            verify_known(single_term, value);
            record_term(single_term, value);
        }
        verify_known(single_term, value);
        print_term(single_term, value);
    } else {
        for (unsigned n = 0; n <= maximum; ++n) {
            peak_memory = memory_used = 0;
            U128 value;
            if (!check_mode && load_saved_term(n, &value)) {
                if (!quiet) {
                    fprintf(stderr, "A291551(%u): loaded from %s\n",
                            n, bfile_path);
                }
            } else {
                value = compute_term(n);
                verify_known(n, value);
                record_term(n, value);
            }
            verify_known(n, value);
            print_term(n, value);
        }
        if (check_mode && !quiet) {
            fprintf(stderr, "A291551: all known terms a(0)..a(%d) verified\n",
                    DEFAULT_MAX_N);
        }
    }

    if (memory_used != 0) {
        die("internal memory leak detected");
    }
    return EXIT_SUCCESS;
}
