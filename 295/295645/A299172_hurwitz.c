#define _DARWIN_C_SOURCE
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

/*
 * A299172: primes p for which p - 1 divides Ramanujan's tau(p).
 *
 * From the Eichler--Selberg/Hurwitz formula used by Lygeros--Rozier,
 *
 *   tau(p) == -s_10(p) - 168                         (mod p - 1),
 *
 * where
 *
 *   s_10(p) = sum_{0 < t < 2 sqrt(p)} t^10 H(4p-t^2).
 *
 * We store the integer H6(D) = 6 H(D).  The test becomes
 *
 *   sum t^10 H6(4p-t^2) + 1008 == 0                 (mod 6(p-1)).
 *
 * H6 is generated exactly by enumerating reduced positive binary quadratic
 * forms.  No GRH assumption and no PARI library are used.
 */

typedef struct {
    uint64_t nmax;
    uint64_t dmax;
    uint64_t block_size;
    uint32_t *h6;
    uint64_t block_count;
    atomic_uint_fast64_t next_block;
    atomic_uint_fast64_t blocks_done;
    uint64_t printed_blocks;
    pthread_mutex_t print_lock;
    double started;
} HurwitzJob;

typedef struct {
    const uint32_t *primes;
    size_t first;
    size_t last;
    const uint32_t *h6;
    uint8_t *matches;
    atomic_size_t next;
    atomic_size_t done;
    unsigned printed_percent;
    size_t batch;
    pthread_mutex_t print_lock;
    double started;
} SearchJob;

static const uint64_t DELTA_T10_AT_1[11] = {
    1ULL,
    1023ULL,
    57002ULL,
    874500ULL,
    5921520ULL,
    21538440ULL,
    46070640ULL,
    59875200ULL,
    46569600ULL,
    19958400ULL,
    3628800ULL
};

static void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static double wall_seconds(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        die("clock_gettime failed");
    }
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static uint64_t parse_u64(const char *s, const char *name)
{
    char *end = NULL;
    errno = 0;
    unsigned long long value = strtoull(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') {
        fprintf(stderr, "error: invalid %s: %s\n", name, s);
        exit(EXIT_FAILURE);
    }
    return (uint64_t)value;
}

static unsigned default_thread_count(void)
{
#ifdef __APPLE__
    int count = 0;
    size_t size = sizeof(count);
    if (sysctlbyname("hw.logicalcpu", &count, &size, NULL, 0) == 0 &&
        count > 0) {
        return (unsigned)count;
    }
#elif defined(_SC_NPROCESSORS_ONLN)
    const long count = sysconf(_SC_NPROCESSORS_ONLN);
    if (count > 0) {
        return (unsigned)count;
    }
#endif
    return 1;
}

static uint64_t isqrt_u64(uint64_t n)
{
    uint64_t r = (uint64_t)sqrtl((long double)n);
    while ((r + 1) <= n / (r + 1)) {
        ++r;
    }
    while (r > 0 && r > n / r) {
        --r;
    }
    return r;
}

static inline uint64_t ceil_div_u64(uint64_t a, uint64_t b)
{
    return a / b + (a % b != 0);
}

/*
 * Relevant discriminants are D == 0 or 3 (mod 4).  For both residue
 * classes, D >> 1 packs them without gaps:
 *
 *   D:       3  4  7  8  11 12 ...
 *   D >> 1:  1  2  3  4   5  6 ...
 */
static inline size_t h6_index(uint64_t d)
{
    return (size_t)(d >> 1);
}

static void generate_hurwitz_range(
    uint64_t lo, uint64_t hi, uint32_t *h6, uint64_t base_index)
{
    const uint64_t amax = isqrt_u64(hi / 3);

    for (uint64_t a = 1; a <= amax; ++a) {
        const uint64_t step = 4 * a;

        /*
         * The reduced-form boundary rule excludes b = -a.  Thus
         * 1-a <= b <= a.
         */
        for (int64_t b = 1 - (int64_t)a; b <= (int64_t)a; ++b) {
            const uint64_t b2 = (uint64_t)(b * b);
            uint64_t c0 = ceil_div_u64(lo + b2, step);
            const uint64_t c1 = (hi + b2) / step;

            if (c0 < a) {
                c0 = a;
            }

            /* If a = c, the reduced-form boundary rule requires b >= 0. */
            if (b < 0 && c0 == a) {
                ++c0;
            }
            if (c0 > c1) {
                continue;
            }

            uint64_t d = step * c0 - b2;

            /*
             * Automorphism weights:
             *   (a,0,a) contributes 1/2, hence 3 to H6;
             *   (a,a,a) contributes 1/3, hence 2 to H6.
             * Every other reduced form contributes 6.
             */
            if (c0 == a && b == 0) {
                h6[h6_index(d) - base_index] += 3;
                ++c0;
                d += step;
            } else if (c0 == a && b == (int64_t)a) {
                h6[h6_index(d) - base_index] += 2;
                ++c0;
                d += step;
            }

            for (uint64_t c = c0; c <= c1; ++c, d += step) {
                h6[h6_index(d) - base_index] += 6;
            }
        }
    }
}

static void generate_hurwitz_block(HurwitzJob *job, uint64_t block_number)
{
    const uint64_t lo = block_number * job->block_size + 1;
    uint64_t hi = (block_number + 1) * job->block_size;
    if (hi > job->dmax) {
        hi = job->dmax;
    }
    generate_hurwitz_range(lo, hi, job->h6, 0);
}

static void *hurwitz_worker(void *arg)
{
    HurwitzJob *job = (HurwitzJob *)arg;

    for (;;) {
        const uint64_t block =
            atomic_fetch_add_explicit(&job->next_block, 1, memory_order_relaxed);
        if (block >= job->block_count) {
            break;
        }

        generate_hurwitz_block(job, block);

        atomic_fetch_add_explicit(&job->blocks_done, 1, memory_order_relaxed);
        pthread_mutex_lock(&job->print_lock);
        const uint64_t done =
            atomic_load_explicit(&job->blocks_done, memory_order_relaxed);
        if (done > job->printed_blocks) {
            job->printed_blocks = done;
            fprintf(stderr,
                    "\rHurwitz table: %" PRIu64 "/%" PRIu64
                    " blocks (%.1f s)",
                    done, job->block_count, wall_seconds() - job->started);
            fflush(stderr);
        }
        pthread_mutex_unlock(&job->print_lock);
    }

    return NULL;
}

static uint32_t *build_hurwitz_table(
    uint64_t nmax, uint64_t block_size, unsigned threads, uint64_t *dmax_out)
{
    if (nmax > UINT64_MAX / 4) {
        die("N is too large");
    }

    const uint64_t dmax = 4 * nmax;
    const uint64_t entries64 = (dmax >> 1) + 1;
    if (entries64 > SIZE_MAX / sizeof(uint32_t)) {
        die("Hurwitz table is too large for this machine");
    }

    const size_t entries = (size_t)entries64;
    uint32_t *h6 = (uint32_t *)calloc(entries, sizeof(*h6));
    if (h6 == NULL) {
        die("cannot allocate Hurwitz table");
    }

    HurwitzJob job;
    memset(&job, 0, sizeof(job));
    job.nmax = nmax;
    job.dmax = dmax;
    job.block_size = block_size;
    job.h6 = h6;
    job.block_count = (dmax + block_size - 1) / block_size;
    atomic_init(&job.next_block, 0);
    atomic_init(&job.blocks_done, 0);
    pthread_mutex_init(&job.print_lock, NULL);
    job.started = wall_seconds();

    pthread_t *ids = (pthread_t *)malloc(threads * sizeof(*ids));
    if (ids == NULL) {
        die("cannot allocate thread handles");
    }

    for (unsigned i = 0; i < threads; ++i) {
        if (pthread_create(&ids[i], NULL, hurwitz_worker, &job) != 0) {
            die("pthread_create failed while building Hurwitz table");
        }
    }
    for (unsigned i = 0; i < threads; ++i) {
        pthread_join(ids[i], NULL);
    }

    fprintf(stderr, "\n");
    pthread_mutex_destroy(&job.print_lock);
    free(ids);
    *dmax_out = dmax;
    return h6;
}

static void check_small_hurwitz_values(const uint32_t *h6, uint64_t dmax)
{
    static const uint32_t expected[17] = {
        0, 0, 0, 2, 3, 0, 0, 6, 6, 0, 0, 6, 8, 0, 0, 12, 9
    };
    const uint64_t last = dmax < 16 ? dmax : 16;

    for (uint64_t d = 1; d <= last; ++d) {
        uint32_t got = 0;
        if ((d & 3U) == 0 || (d & 3U) == 3) {
            got = h6[h6_index(d)];
        }
        if (got != expected[d]) {
            fprintf(stderr,
                    "error: H6 self-test failed at D=%" PRIu64
                    ": got %" PRIu32 ", expected %" PRIu32 "\n",
                    d, got, expected[d]);
            exit(EXIT_FAILURE);
        }
    }
}

static uint32_t *sieve_primes(uint32_t n, size_t *count_out)
{
    uint8_t *composite = (uint8_t *)calloc((size_t)n + 1, 1);
    if (composite == NULL) {
        die("cannot allocate prime sieve");
    }

    const uint32_t root = (uint32_t)isqrt_u64(n);
    for (uint32_t p = 2; p <= root; ++p) {
        if (composite[p]) {
            continue;
        }
        for (uint64_t k = (uint64_t)p * p; k <= n; k += p) {
            composite[k] = 1;
        }
    }

    size_t count = 0;
    for (uint32_t p = 2; p <= n; ++p) {
        if (!composite[p]) {
            ++count;
        }
    }

    uint32_t *primes = (uint32_t *)malloc(count * sizeof(*primes));
    if (primes == NULL) {
        die("cannot allocate prime list");
    }

    size_t j = 0;
    for (uint32_t p = 2; p <= n; ++p) {
        if (!composite[p]) {
            primes[j++] = p;
        }
    }

    free(composite);
    *count_out = count;
    return primes;
}

static size_t lower_bound_u32(const uint32_t *a, size_t n, uint32_t value)
{
    size_t lo = 0;
    size_t hi = n;
    while (lo < hi) {
        const size_t mid = lo + (hi - lo) / 2;
        if (a[mid] < value) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }
    return lo;
}

static bool is_a299172_prime(uint32_t p, const uint32_t *h6)
{
    const uint64_t modulus = 6ULL * ((uint64_t)p - 1);
    uint64_t sum = 1008 % modulus;
    uint64_t delta[11];

    for (unsigned k = 0; k <= 10; ++k) {
        delta[k] = DELTA_T10_AT_1[k] % modulus;
    }

    const uint64_t tmax = isqrt_u64(4ULL * p);
    for (uint64_t t = 1; t <= tmax; ++t) {
        const uint64_t d = 4ULL * p - t * t;
        const uint64_t h = h6[h6_index(d)];
        uint64_t term;
        if (h != 0 && delta[0] > UINT64_MAX / h) {
            term = (uint64_t)(((__uint128_t)delta[0] * h) % modulus);
        } else {
            term = (delta[0] * h) % modulus;
        }

        sum += term;
        if (sum >= modulus) {
            sum -= modulus;
        }

        /*
         * Advance t^10 modulo the modulus using forward differences.
         * This replaces modular exponentiation in the innermost loop
         * by ten additions.
         */
        for (unsigned k = 0; k < 10; ++k) {
            delta[k] += delta[k + 1];
            if (delta[k] >= modulus) {
                delta[k] -= modulus;
            }
        }
    }

    return sum == 0;
}

static void report_search_progress(SearchJob *job, size_t amount)
{
    const size_t done =
        atomic_fetch_add_explicit(&job->done, amount, memory_order_relaxed) + amount;
    const size_t total = job->last - job->first;
    unsigned percent = (unsigned)((100 * done) / total);
    if (percent > 100) {
        percent = 100;
    }

    pthread_mutex_lock(&job->print_lock);
    if (percent > job->printed_percent) {
        job->printed_percent = percent;
        fprintf(stderr, "\rPrime search: %3u%% (%.1f s)",
                percent, wall_seconds() - job->started);
        fflush(stderr);
    }
    pthread_mutex_unlock(&job->print_lock);
}

static void *search_worker(void *arg)
{
    SearchJob *job = (SearchJob *)arg;

    for (;;) {
        const size_t begin =
            atomic_fetch_add_explicit(&job->next, job->batch, memory_order_relaxed);
        if (begin >= job->last) {
            break;
        }
        size_t end = begin + job->batch;
        if (end > job->last) {
            end = job->last;
        }

        for (size_t i = begin; i < end; ++i) {
            if (is_a299172_prime(job->primes[i], job->h6)) {
                job->matches[i] = 1;
            }
        }
        report_search_progress(job, end - begin);
    }

    return NULL;
}

static void search_primes(
    const uint32_t *primes, size_t prime_count, uint32_t start,
    const uint32_t *h6, uint8_t *matches, unsigned threads)
{
    SearchJob job;
    memset(&job, 0, sizeof(job));
    job.primes = primes;
    job.first = lower_bound_u32(primes, prime_count, start);
    job.last = prime_count;
    job.h6 = h6;
    job.matches = matches;
    job.batch = 8;
    atomic_init(&job.next, job.first);
    atomic_init(&job.done, 0);
    job.printed_percent = 0;
    pthread_mutex_init(&job.print_lock, NULL);
    job.started = wall_seconds();

    pthread_t *ids = (pthread_t *)malloc(threads * sizeof(*ids));
    if (ids == NULL) {
        die("cannot allocate thread handles");
    }

    for (unsigned i = 0; i < threads; ++i) {
        if (pthread_create(&ids[i], NULL, search_worker, &job) != 0) {
            die("pthread_create failed during prime search");
        }
    }
    for (unsigned i = 0; i < threads; ++i) {
        pthread_join(ids[i], NULL);
    }

    if (job.printed_percent < 100) {
        fprintf(stderr, "\rPrime search: 100%% (%.1f s)\n",
                wall_seconds() - job.started);
    } else {
        fprintf(stderr, "\n");
    }
    pthread_mutex_destroy(&job.print_lock);
    free(ids);
}

static void usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s [options]\n"
            "  -n, --limit N       search primes p <= N (default: 10000000)\n"
            "  -s, --start N       first p to search (default: 2)\n"
            "  -t, --threads N     worker threads (default: CPU count)\n"
            "  -b, --block N       Hurwitz discriminants per block"
            " (default: 1000000)\n"
            "  -o, --output FILE   append hits to FILE\n"
            "  -h, --help          show this help\n",
            program);
}

int main(int argc, char **argv)
{
    uint64_t nmax64 = 10000000;
    uint64_t start64 = 2;
    uint64_t block_size = 1000000;
    unsigned threads = default_thread_count();
    const char *output_path = "A299172_hurwitz_c_hits.txt";

    for (int i = 1; i < argc; ++i) {
        if ((!strcmp(argv[i], "-n") || !strcmp(argv[i], "--limit")) &&
            i + 1 < argc) {
            nmax64 = parse_u64(argv[++i], "limit");
        } else if ((!strcmp(argv[i], "-s") || !strcmp(argv[i], "--start")) &&
                   i + 1 < argc) {
            start64 = parse_u64(argv[++i], "start");
        } else if ((!strcmp(argv[i], "-t") || !strcmp(argv[i], "--threads")) &&
                   i + 1 < argc) {
            const uint64_t value = parse_u64(argv[++i], "threads");
            if (value == 0 || value > 1024) {
                die("threads must be between 1 and 1024");
            }
            threads = (unsigned)value;
        } else if ((!strcmp(argv[i], "-b") || !strcmp(argv[i], "--block")) &&
                   i + 1 < argc) {
            block_size = parse_u64(argv[++i], "block");
        } else if ((!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) &&
                   i + 1 < argc) {
            output_path = argv[++i];
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (nmax64 < 2 || nmax64 > UINT32_MAX) {
        die("limit must be between 2 and 4294967295");
    }
    if (start64 < 2) {
        start64 = 2;
    }
    if (start64 > nmax64) {
        die("start is greater than limit");
    }
    if (block_size == 0) {
        die("block size must be positive");
    }

    const uint32_t nmax = (uint32_t)nmax64;
    const uint32_t start = (uint32_t)start64;
    const double all_started = wall_seconds();

    fprintf(stderr,
            "A299172 Hurwitz search\n"
            "limit=%" PRIu32 ", start=%" PRIu32 ", threads=%u,"
            " block=%" PRIu64 "\n",
            nmax, start, threads, block_size);

    uint64_t dmax = 0;
    uint32_t *h6 =
        build_hurwitz_table(nmax, block_size, threads, &dmax);
    check_small_hurwitz_values(h6, dmax);
    fprintf(stderr, "Hurwitz self-test passed; table memory %.1f MiB\n",
            ((double)((dmax >> 1) + 1) * sizeof(*h6)) / (1024.0 * 1024.0));

    size_t prime_count = 0;
    uint32_t *primes = sieve_primes(nmax, &prime_count);
    uint8_t *matches = (uint8_t *)calloc(prime_count, 1);
    if (matches == NULL) {
        die("cannot allocate result array");
    }
    fprintf(stderr, "primes <= %" PRIu32 ": %zu\n", nmax, prime_count);

    search_primes(primes, prime_count, start, h6, matches, threads);

    FILE *out = fopen(output_path, "a");
    if (out == NULL) {
        fprintf(stderr, "error: cannot open output file %s: %s\n",
                output_path, strerror(errno));
        return EXIT_FAILURE;
    }

    fprintf(out, "# limit=%" PRIu32 " start=%" PRIu32 "\n", nmax, start);
    printf("hits:");
    size_t hit_count = 0;
    const size_t first = lower_bound_u32(primes, prime_count, start);
    for (size_t i = first; i < prime_count; ++i) {
        if (matches[i]) {
            printf(" %" PRIu32, primes[i]);
            fprintf(out, "%" PRIu32 "\n", primes[i]);
            ++hit_count;
        }
    }
    printf("\n");
    fprintf(out, "# hits=%zu elapsed=%.3f seconds\n",
            hit_count, wall_seconds() - all_started);
    fclose(out);

    fprintf(stderr, "finished in %.1f s; %zu hits; output: %s\n",
            wall_seconds() - all_started, hit_count, output_path);

    free(matches);
    free(primes);
    free(h6);
    return EXIT_SUCCESS;
}
