/*
 * A202954 -- exact maximum-first recursion adapted from the pruning idea in
 * Hercher and Niedermeyer, arXiv:2307.00303.
 *
 * Count partitions of {1,...,3*n} into unordered triples of distinct values
 * satisfying x+y=4*z.  The paper's inequalities for x+y=z do NOT carry over:
 * for example {3,5,7} is valid because 5+7=4*3, although 3+5>7.
 * The following replacement is proved specifically for x+y=4*z.
 *
 * Let M be the largest remaining value.  It cannot be z, since two smaller
 * positive values cannot sum to 4*M.  Thus M is one addend.  Once z is
 * selected, the other addend is uniquely
 *
 *                         q = 4*z-M,
 *
 * and positivity plus q<M gives M/4<z<M/2.  The unique triple containing M
 * therefore gives exactly one recursive branch in every valid partition.
 * Induction proves that maximum-first recursion counts each unordered
 * partition exactly once.
 *
 * For a remaining set S of 3*m values, let T be its total, L the sum of its
 * m smallest values, and H the sum of its m largest values.  In a completion,
 * T=5*sum(z).  Since L is no greater than the sum of any m values,
 *
 *                         5*L <= T.                 (1)
 *
 * Equality holds iff the z-values are precisely the m smallest values.  It
 * persists after a valid triple is removed, so a reduced recursion can then
 * require z in the lower block and both addends in the upper block.
 *
 * In each triple label the addends a<b.  From a+b=4*z and a<b follows
 * b>2*z.  The m values b form an m-subset of S, hence
 *
 *                         5*H > 2*T.                (2)
 *
 * Violations of (1) or (2) are exact impossibility tests, not heuristics.
 * At the root T=3*n*(3*n+1)/2 must be divisible by 5, yielding the exact
 * zero condition n==0 or 3 (mod 5).
 *
 * Two uint64_t words hold values 1..72 (MAX_N=24).  T,L,H are at most
 * 1+...+72=2628.  Each level has at most 18 z-candidates, so the answer is
 * bounded by 18^24<2^101; U128 is ample and additions are checked anyway.
 * Root branches are dynamically distributed among pthread workers.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       202954_01.c -o 202954_01
 *
 * Usage:
 *   ./202954_01 15 --threads 8
 *   ./202954_01 --term 24 --threads 8
 *   ./202954_01 --check --threads 8 --no-bfile
 * Results are atomically recorded in b202954_01.txt by default.  Use
 * --output FILE to select another b-file or --no-bfile to disable writing.
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if !defined(__SIZEOF_INT128__)
#error "202954_01.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 24
#define DEFAULT_N 15
#define DEFAULT_CHECK_N 15
#define MAX_THREADS 64
#define MAX_VALUES (3 * MAX_N)
#define MAX_ROOT_TASKS MAX_VALUES

_Static_assert(MAX_VALUES <= 128,
               "remaining-set bit mask requires at most 128 values");

static const char *output_path = "b202954_01.txt";
static bool write_bfile = true;
static int requested_threads = 4;

static const char *const known[] = {
    "1", "0", "0", "0", "0", "0", "0", "0", "6", "0",
    "5", "0", "0", "349", "0", "443", "0", "0", "110757", "0",
    "1254452", "0", "0", "152965479", "0"
};

static _Noreturn void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static int parse_n(const char *text)
{
    char *end = NULL;
    errno = 0;
    const long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < 0 || value > MAX_N) {
        fprintf(stderr, "error: N must be in 0..%d: %s\n", MAX_N, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static int parse_threads(const char *text)
{
    char *end = NULL;
    errno = 0;
    const long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < 1 || value > MAX_THREADS) {
        fprintf(stderr, "error: threads must be in 1..%d: %s\n",
                MAX_THREADS, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static double now_seconds(void)
{
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) {
        die("clock_gettime failed");
    }
    return (double)time.tv_sec + (double)time.tv_nsec / 1e9;
}

static int print_u128(FILE *stream, U128 value)
{
    char digits[40];
    size_t length = 0;
    do {
        digits[length++] = (char)('0' + (unsigned)(value % 10));
        value /= 10;
    } while (value != 0);
    while (length != 0) {
        if (fputc(digits[--length], stream) == EOF) {
            return -1;
        }
    }
    return 0;
}

static void print_term(int n, U128 value)
{
    if (fprintf(stdout, "%d ", n) < 0 ||
        print_u128(stdout, value) != 0 ||
        fputc('\n', stdout) == EOF || fflush(stdout) != 0) {
        die("cannot write result to standard output");
    }
}

static bool parse_u128(const char *text, U128 *result)
{
    const U128 maximum = ~(U128)0;
    U128 value = 0;
    if (*text == '\0') {
        return false;
    }
    for (; *text != '\0'; ++text) {
        if (*text < '0' || *text > '9') {
            return false;
        }
        const unsigned digit = (unsigned)(*text - '0');
        if (value > (maximum - digit) / 10) {
            return false;
        }
        value = value * 10 + digit;
    }
    *result = value;
    return true;
}

static bool add_u128(U128 *destination, U128 addend)
{
    const U128 old = *destination;
    *destination += addend;
    return *destination >= old;
}

static int acquire_bfile_lock(void)
{
    const char suffix[] = ".lock";
    const size_t length = strlen(output_path);
    if (length > SIZE_MAX - sizeof(suffix)) {
        die("b-file lock path is too long");
    }
    char *path = malloc(length + sizeof(suffix));
    if (path == NULL) {
        die("cannot allocate b-file lock path");
    }
    memcpy(path, output_path, length);
    memcpy(path + length, suffix, sizeof(suffix));
    const int fd = open(path, O_RDWR | O_CREAT, 0666);
    free(path);
    if (fd < 0) {
        die("cannot open b-file lock");
    }
    struct flock lock = {
        .l_type = F_WRLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0
    };
    int result;
    do {
        result = fcntl(fd, F_SETLKW, &lock);
    } while (result != 0 && errno == EINTR);
    if (result != 0) {
        close(fd);
        die("cannot lock b-file");
    }
    return fd;
}

static void release_bfile_lock(int fd)
{
    struct flock lock = {
        .l_type = F_UNLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0
    };
    const bool unlock_failed = fcntl(fd, F_SETLK, &lock) != 0;
    const bool close_failed = close(fd) != 0;
    if (unlock_failed || close_failed) {
        die("cannot release b-file lock");
    }
}

static void sync_parent_directory(const char *path)
{
    const char *slash = strrchr(path, '/');
    char *allocated = NULL;
    const char *directory;
    if (slash == NULL) {
        directory = ".";
    } else if (slash == path) {
        directory = "/";
    } else {
        const size_t length = (size_t)(slash - path);
        allocated = malloc(length + 1);
        if (allocated == NULL) {
            die("cannot allocate b-file directory path");
        }
        memcpy(allocated, path, length);
        allocated[length] = '\0';
        directory = allocated;
    }
    const int fd = open(directory, O_RDONLY);
    free(allocated);
    if (fd < 0) {
        die("cannot open b-file parent directory");
    }
    const int result = fsync(fd);
    const int saved_errno = errno;
    const bool close_failed = close(fd) != 0;
    if ((result != 0 && saved_errno != EINVAL && saved_errno != ENOTSUP) ||
        close_failed) {
        die("cannot synchronize b-file parent directory");
    }
}

static void store_bfile_term(int n, U128 value)
{
    const int lock_fd = acquire_bfile_lock();
    U128 values[MAX_N + 1] = {0};
    bool present[MAX_N + 1] = {false};
    int previous = -1;
    mode_t output_mode = 0644;
    struct stat metadata;
    if (stat(output_path, &metadata) == 0) {
        output_mode = metadata.st_mode & 0777;
    } else if (errno != ENOENT) {
        die("cannot inspect b-file");
    }

    FILE *input = fopen(output_path, "r");
    if (input == NULL && errno != ENOENT) {
        die("cannot open existing b-file");
    }
    if (input != NULL) {
        char line[128];
        while (fgets(line, sizeof(line), input) != NULL) {
            int index;
            char number[64];
            char extra;
            if (sscanf(line, "%d %63s %c", &index, number, &extra) != 2 ||
                index < 0 || index > MAX_N || index <= previous ||
                !parse_u128(number, &values[index])) {
                fclose(input);
                die("existing b-file is malformed or not strictly ordered");
            }
            present[index] = true;
            previous = index;
        }
        if (ferror(input) || fclose(input) != 0) {
            die("cannot read existing b-file");
        }
    }

    if (present[n]) {
        if (values[n] != value) {
            die("computed term disagrees with existing b-file");
        }
        release_bfile_lock(lock_fd);
        return;
    }
    values[n] = value;
    present[n] = true;

    const char suffix[] = ".tmp.XXXXXX";
    const size_t path_length = strlen(output_path);
    if (path_length > SIZE_MAX - sizeof(suffix)) {
        die("b-file path is too long");
    }
    char *temporary = malloc(path_length + sizeof(suffix));
    if (temporary == NULL) {
        die("cannot allocate b-file temporary path");
    }
    memcpy(temporary, output_path, path_length);
    memcpy(temporary + path_length, suffix, sizeof(suffix));
    const int fd = mkstemp(temporary);
    if (fd < 0) {
        free(temporary);
        die("cannot create temporary b-file");
    }
    if (fchmod(fd, output_mode) != 0) {
        close(fd);
        unlink(temporary);
        free(temporary);
        die("cannot set temporary b-file permissions");
    }
    FILE *output = fdopen(fd, "w");
    if (output == NULL) {
        close(fd);
        unlink(temporary);
        free(temporary);
        die("cannot open temporary b-file stream");
    }
    bool failed = false;
    for (int index = 0; index <= MAX_N; ++index) {
        if (!present[index]) {
            continue;
        }
        if (fprintf(output, "%d ", index) < 0 ||
            print_u128(output, values[index]) != 0 ||
            fputc('\n', output) == EOF) {
            failed = true;
            break;
        }
    }
    if (!failed && fflush(output) != 0) {
        failed = true;
    }
    if (!failed && fsync(fd) != 0) {
        failed = true;
    }
    if (fclose(output) != 0) {
        failed = true;
    }
    if (failed) {
        unlink(temporary);
        free(temporary);
        die("cannot write temporary b-file");
    }
    if (rename(temporary, output_path) != 0) {
        unlink(temporary);
        free(temporary);
        die("cannot atomically replace b-file");
    }
    sync_parent_directory(output_path);
    free(temporary);
    release_bfile_lock(lock_fd);
    fprintf(stderr, "202954_01: recorded computed A202954 term n=%d in %s\n",
            n, output_path);
}

typedef struct {
    uint64_t low;                       /* Values 1..64. */
    uint64_t high;                      /* Values 65..128. */
} Mask;

static bool mask_is_empty(Mask mask)
{
    return mask.low == 0 && mask.high == 0;
}

static unsigned mask_count(Mask mask)
{
    return (unsigned)__builtin_popcountll(mask.low) +
           (unsigned)__builtin_popcountll(mask.high);
}

static bool mask_contains(Mask mask, unsigned value)
{
    if (value == 0 || value > MAX_VALUES) {
        return false;
    }
    if (value <= 64U) {
        return (mask.low & (UINT64_C(1) << (value - 1U))) != 0;
    }
    return (mask.high & (UINT64_C(1) << (value - 65U))) != 0;
}

static Mask mask_without(Mask mask, unsigned value)
{
    if (!mask_contains(mask, value)) {
        die("attempt to remove an absent search value");
    }
    if (value <= 64U) {
        mask.low &= ~(UINT64_C(1) << (value - 1U));
    } else {
        mask.high &= ~(UINT64_C(1) << (value - 65U));
    }
    return mask;
}

static Mask full_mask(unsigned values)
{
    if (values == 0 || values > MAX_VALUES) {
        die("invalid full search-mask size");
    }
    Mask mask = {0, 0};
    if (values >= 64U) {
        mask.low = UINT64_MAX;
        const unsigned high_values = values - 64U;
        mask.high = high_values == 64U ? UINT64_MAX :
                    (UINT64_C(1) << high_values) - 1U;
    } else {
        mask.low = (UINT64_C(1) << values) - 1U;
    }
    return mask;
}

static unsigned largest_value(Mask mask)
{
    if (mask.high != 0) {
        return 128U - (unsigned)__builtin_clzll(mask.high);
    }
    if (mask.low != 0) {
        return 64U - (unsigned)__builtin_clzll(mask.low);
    }
    die("internal search mask is empty");
    return 0;
}

static int sum_smallest(Mask mask, unsigned count, unsigned *last)
{
    int sum = 0;
    unsigned value = 0;
    for (unsigned index = 0; index < count; ++index) {
        if (mask.low != 0) {
            value = (unsigned)__builtin_ctzll(mask.low) + 1U;
            mask.low &= mask.low - 1U;
        } else if (mask.high != 0) {
            value = (unsigned)__builtin_ctzll(mask.high) + 65U;
            mask.high &= mask.high - 1U;
        } else {
            die("too few values while summing the smallest block");
        }
        sum += (int)value;
    }
    if (last != NULL) {
        *last = value;
    }
    return sum;
}

static int sum_largest(Mask mask, unsigned count)
{
    int sum = 0;
    for (unsigned index = 0; index < count; ++index) {
        unsigned value;
        if (mask.high != 0) {
            const unsigned bit = 63U - (unsigned)__builtin_clzll(mask.high);
            value = bit + 65U;
            mask.high &= ~(UINT64_C(1) << bit);
        } else if (mask.low != 0) {
            const unsigned bit = 63U - (unsigned)__builtin_clzll(mask.low);
            value = bit + 1U;
            mask.low &= ~(UINT64_C(1) << bit);
        } else {
            die("too few values while summing the largest block");
        }
        sum += (int)value;
    }
    return sum;
}

typedef struct {
    uint64_t states;
} SearchStats;

static U128 count_partitions(Mask mask, unsigned m, int total,
                             bool reduced, SearchStats *stats)
{
    if (mask_count(mask) != 3U * m) {
        die("recursion mask cardinality invariant failed");
    }
    if (stats->states == UINT64_MAX) {
        die("recursive-state counter overflowed");
    }
    ++stats->states;
    if (m == 0) {
        if (!mask_is_empty(mask) || total != 0) {
            die("nonempty terminal recursion state");
        }
        return 1;
    }
    if (total <= 0 || total % 5 != 0) {
        return 0;                       /* T=5*sum(z) is necessary. */
    }

    unsigned lower_last;
    const int lower_sum = sum_smallest(mask, m, &lower_last);
    if (5 * lower_sum > total) {        /* Inequality (1). */
        return 0;
    }
    const bool equality = 5 * lower_sum == total;
    if (reduced && !equality) {
        die("reduced-recursion equality invariant failed");
    }
    if (equality) {
        reduced = true;
    }

    const int upper_sum = sum_largest(mask, m);
    if (5 * upper_sum <= 2 * total) {   /* Strict inequality (2). */
        return 0;
    }

    const unsigned maximum = largest_value(mask);
    const unsigned first_z = maximum / 4U + 1U;
    unsigned last_z = (maximum - 1U) / 2U;
    if (reduced && last_z > lower_last) {
        last_z = lower_last;
    }

    U128 answer = 0;
    for (unsigned z = first_z; z <= last_z; ++z) {
        const unsigned q = 4U * z - maximum;
        if (q == z) {
            continue;                   /* A triple contains distinct values. */
        }
        if (reduced && q <= lower_last) {
            continue;                   /* Both addends belong to the upper block. */
        }
        if (!mask_contains(mask, z) || !mask_contains(mask, q)) {
            continue;
        }

        Mask child = mask_without(mask, maximum);
        child = mask_without(child, z);
        child = mask_without(child, q);
        const U128 addend = count_partitions(
            child, m - 1U, total - 5 * (int)z, reduced, stats);
        if (!add_u128(&answer, addend)) {
            die("A202954 answer overflowed unsigned __int128");
        }
    }
    return answer;
}

typedef struct {
    Mask mask;
    unsigned m;
    int total;
} RootTask;

typedef struct {
    _Atomic unsigned next;
    unsigned count;
    RootTask tasks[MAX_ROOT_TASKS];
} RootSchedule;

typedef struct {
    RootSchedule *schedule;
    U128 answer;
    SearchStats stats;
} Worker;

static unsigned build_root_tasks(int n, RootTask tasks[MAX_ROOT_TASKS])
{
    const unsigned m = (unsigned)n;
    const unsigned maximum = 3U * m;
    const Mask mask = full_mask(maximum);
    const int total = (int)(maximum * (maximum + 1U) / 2U);
    unsigned count = 0;

    for (unsigned z = maximum / 4U + 1U;
         z <= (maximum - 1U) / 2U; ++z) {
        const unsigned q = 4U * z - maximum;
        if (q == z) {
            continue;
        }
        if (count >= MAX_ROOT_TASKS) {
            die("root-task array is too small");
        }
        Mask child = mask_without(mask, maximum);
        child = mask_without(child, z);
        child = mask_without(child, q);
        tasks[count++] = (RootTask) {
            .mask = child,
            .m = m - 1U,
            .total = total - 5 * (int)z
        };
    }
    return count;
}

static void *worker_main(void *argument)
{
    Worker *worker = argument;
    for (;;) {
        const unsigned index = atomic_fetch_add_explicit(
            &worker->schedule->next, 1U, memory_order_relaxed);
        if (index >= worker->schedule->count) {
            break;
        }
        const RootTask *const task = &worker->schedule->tasks[index];
        const U128 addend = count_partitions(
            task->mask, task->m, task->total, false, &worker->stats);
        if (!add_u128(&worker->answer, addend)) {
            die("worker answer overflowed unsigned __int128");
        }
    }
    return NULL;
}

static U128 a202954(int n)
{
    if (n == 0) {
        return 1;
    }
    if (n % 5 != 0 && n % 5 != 3) {
        return 0;
    }

    RootSchedule schedule = {0};
    atomic_init(&schedule.next, 0U);
    schedule.count = build_root_tasks(n, schedule.tasks);
    if (schedule.count == 0) {
        return 0;
    }
    int threads = requested_threads;
    if ((unsigned)threads > schedule.count) {
        threads = (int)schedule.count;
    }

    Worker *workers = calloc((size_t)threads, sizeof(*workers));
    pthread_t *ids = calloc((size_t)threads, sizeof(*ids));
    if (workers == NULL || ids == NULL) {
        free(workers);
        free(ids);
        die("cannot allocate search workers");
    }

    const double started = now_seconds();
    for (int id = 0; id < threads; ++id) {
        workers[id].schedule = &schedule;
        const int error = pthread_create(&ids[id], NULL,
                                         worker_main, &workers[id]);
        if (error != 0) {
            fprintf(stderr, "error: pthread_create: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }

    U128 answer = 0;
    uint64_t states = 1;                /* Count the root state. */
    for (int id = 0; id < threads; ++id) {
        const int error = pthread_join(ids[id], NULL);
        if (error != 0) {
            fprintf(stderr, "error: pthread_join: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
        if (!add_u128(&answer, workers[id].answer)) {
            die("A202954 answer overflowed unsigned __int128");
        }
        if (UINT64_MAX - states < workers[id].stats.states) {
            die("recursive-state counter overflowed");
        }
        states += workers[id].stats.states;
    }
    free(workers);
    free(ids);

    fprintf(stderr,
            "202954_01: A202954 n=%d, maximum-first exact pruning, "
            "root-tasks=%u, states=%" PRIu64 ", threads=%d, %.3f s\n",
            n, schedule.count, states, threads, now_seconds() - started);
    return answer;
}

static void verify_internal_invariants(void)
{
    /* Exercise both words and their 64/65 boundary without a long search. */
    Mask mask = full_mask(MAX_VALUES);
    if (mask_count(mask) != MAX_VALUES || largest_value(mask) != MAX_VALUES ||
        sum_smallest(mask, MAX_N, NULL) != 300 ||
        sum_largest(mask, MAX_N) != 1452) {
        die("two-word search-mask self-check failed");
    }
    mask = mask_without(mask, 64);
    mask = mask_without(mask, 65);
    mask = mask_without(mask, 72);
    if (mask_count(mask) != MAX_VALUES - 3U || largest_value(mask) != 71U ||
        mask_contains(mask, 64) || mask_contains(mask, 65) ||
        mask_contains(mask, 72)) {
        die("two-word search-mask boundary self-check failed");
    }

    /* {3,4,5,6,7,10} has exactly the completion
       {3,5,7},{4,6,10}; its z-values are its two smallest values, so this
       also exercises entry into and persistence of reduced recursion. */
    mask = full_mask(10);
    mask = mask_without(mask, 1);
    mask = mask_without(mask, 2);
    mask = mask_without(mask, 8);
    mask = mask_without(mask, 9);
    SearchStats stats = {0};
    if (count_partitions(mask, 2, 35, false, &stats) != 1) {
        die("reduced-recursion self-check failed");
    }
    stats = (SearchStats){0};
    if (count_partitions(mask, 2, 35, true, &stats) != 1) {
        die("explicit reduced-recursion self-check failed");
    }
}

static void verify_known(int n, U128 value)
{
    const int count = (int)(sizeof(known) / sizeof(known[0]));
    if (n >= count) {
        return;
    }
    U128 expected;
    if (!parse_u128(known[n], &expected)) {
        die("invalid built-in known term");
    }
    if (value != expected) {
        fprintf(stderr, "error: A202954 mismatch at n=%d: got ", n);
        print_u128(stderr, value);
        fprintf(stderr, ", expected %s\n", known[n]);
        exit(EXIT_FAILURE);
    }
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--threads T] [--output FILE]\n"
            "       %s --term N [--threads T] [--output FILE]\n"
            "       %s --check [--threads T] [--no-bfile]\n"
            "N must be in 0..%d; T must be in 1..%d.\n",
            program, program, program, MAX_N, MAX_THREADS);
}

int main(int argc, char **argv)
{
    enum { MODE_RANGE, MODE_TERM, MODE_CHECK } mode = MODE_RANGE;
    int maximum = -1;
    bool have_mode = false;
    bool have_threads = false;
    bool have_output = false;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        if (!strcmp(argv[i], "--threads")) {
            if (have_threads || ++i >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            requested_threads = parse_threads(argv[i]);
            have_threads = true;
        } else if (!strcmp(argv[i], "--output")) {
            if (have_output || ++i >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            output_path = argv[i];
            write_bfile = true;
            have_output = true;
        } else if (!strcmp(argv[i], "--no-bfile")) {
            if (have_output) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            write_bfile = false;
            have_output = true;
        } else if (!strcmp(argv[i], "--term")) {
            if (have_mode || ++i >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = MODE_TERM;
            maximum = parse_n(argv[i]);
            have_mode = true;
        } else if (!strcmp(argv[i], "--check")) {
            if (have_mode) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = MODE_CHECK;
            have_mode = true;
        } else if (argv[i][0] != '-' && !have_mode) {
            maximum = parse_n(argv[i]);
            mode = MODE_RANGE;
            have_mode = true;
        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (maximum < 0) {
        maximum = DEFAULT_N;
    }
    if (mode == MODE_CHECK) {
        verify_internal_invariants();
        for (int n = 0; n <= DEFAULT_CHECK_N; ++n) {
            verify_known(n, a202954(n));
        }
        printf("ok: A202954 terms n=0..%d verified with exact "
               "maximum-first pruning\n", DEFAULT_CHECK_N);
        return EXIT_SUCCESS;
    }
    if (mode == MODE_TERM) {
        const U128 value = a202954(maximum);
        verify_known(maximum, value);
        if (write_bfile) {
            store_bfile_term(maximum, value);
        }
        print_term(maximum, value);
        return EXIT_SUCCESS;
    }

    for (int n = 0; n <= maximum; ++n) {
        const U128 value = a202954(n);
        verify_known(n, value);
        if (write_bfile) {
            store_bfile_term(n, value);
        }
        print_term(n, value);
    }
    return EXIT_SUCCESS;
}
