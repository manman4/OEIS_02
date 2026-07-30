/*
 * A299171
 *
 * Primes p such that p + 1 divides Ramanujan's tau(p).
 *
 * This program reuses the completed disk-backed Hurwitz table
 * A299172_H6_disk_1e9.bin without modifying it.
 *
 * Put
 *
 *     s10(p) = sum_{0<t<2sqrt(p)} t^10 H(4p-t^2).
 *
 * The Eichler--Selberg/Hurwitz formula in Lygeros--Rozier gives, for
 * every prime p,
 *
 *     tau(p) =
 *       (p+1)*(42p^5 - 42p^4 - 48p^3 - 27p^2 - 8p - 1) - s10(p).
 *
 * Reducing this exact identity modulo p+1 gives
 *
 *     tau(p) == -s10(p)  (mod p+1).
 *
 * The table stores the integer H6(D) = 6 H(D), so define
 *
 *     S6(p) = sum_{0<t<2sqrt(p)} t^10 H6(4p-t^2)
 *           = 6 s10(p).
 *
 * Therefore the required divisibility condition is exactly
 *
 *     p+1 divides tau(p)
 *       iff p+1 divides s10(p)
 *       iff S6(p) == 0  (mod 6*(p+1)).
 *
 * Testing modulo 6*(p+1), rather than dividing S6 by 6 modulo p+1,
 * also works when p+1 is not coprime to 6.
 */

#define A296580_NO_MAIN
#include "296580_01.c"

typedef struct {
    const uint32_t *primes;
    size_t prime_count;
    uint64_t *sums;
    const uint32_t *h6;
    uint64_t h6_base;
    uint64_t dlo;
    uint64_t dhi;
    atomic_size_t next;
    atomic_size_t done;
    size_t batch;
    unsigned printed_percent;
    pthread_mutex_t print_lock;
    double started;
    uint64_t chunk_number;
    uint64_t chunk_count;
} A299171SearchJob;

static void report_a299171_progress(
    A299171SearchJob *job, size_t amount)
{
    const size_t done =
        atomic_fetch_add_explicit(&job->done, amount, memory_order_relaxed) +
        amount;
    unsigned percent =
        job->prime_count ? (unsigned)(100 * done / job->prime_count) : 100;
    if (percent > 100) {
        percent = 100;
    }

    pthread_mutex_lock(&job->print_lock);
    if (percent == 100 || percent >= job->printed_percent + 5) {
        job->printed_percent = percent;
        fprintf(stderr,
                "\rA299171 chunk %" PRIu64 "/%" PRIu64
                ": %3u%% (%.1f s)",
                job->chunk_number, job->chunk_count, percent,
                wall_seconds() - job->started);
        fflush(stderr);
    }
    pthread_mutex_unlock(&job->print_lock);
}

static void *a299171_worker(void *arg)
{
    A299171SearchJob *job = (A299171SearchJob *)arg;

    for (;;) {
        const size_t begin =
            atomic_fetch_add_explicit(&job->next, job->batch,
                                      memory_order_relaxed);
        if (begin >= job->prime_count) {
            break;
        }
        size_t end = begin + job->batch;
        if (end > job->prime_count) {
            end = job->prime_count;
        }

        for (size_t i = begin; i < end; ++i) {
            const uint64_t p = job->primes[i];
            const uint64_t fourp = 4 * p;
            if (fourp <= job->dlo) {
                continue;
            }

            const uint64_t lower_square =
                fourp > job->dhi ? fourp - job->dhi : 1;
            const uint64_t upper_square = fourp - job->dlo;
            const uint64_t t0 = ceil_sqrt_u64(lower_square);
            const uint64_t t1 = isqrt_u64(upper_square);
            if (t0 > t1) {
                continue;
            }

            const uint64_t modulus = 6 * (p + 1);
            uint64_t delta[11];
            deltas_at(t0, modulus, delta);
            uint64_t sum = job->sums[i];

            for (uint64_t t = t0; t <= t1; ++t) {
                const uint64_t d = fourp - t * t;
                const uint64_t h =
                    job->h6[(d >> 1) - job->h6_base];
                const uint64_t term =
                    mul_mod_disk(delta[0], h, modulus);

                sum += term;
                if (sum >= modulus) {
                    sum -= modulus;
                }

                for (unsigned k = 0; k < 10; ++k) {
                    delta[k] += delta[k + 1];
                    if (delta[k] >= modulus) {
                        delta[k] -= modulus;
                    }
                }
            }
            job->sums[i] = sum;
        }

        report_a299171_progress(job, end - begin);
    }

    return NULL;
}

static void process_a299171_chunk(
    const uint32_t *primes, size_t prime_count, uint64_t *sums,
    const uint32_t *h6, uint64_t h6_base,
    uint64_t dlo, uint64_t dhi, unsigned threads,
    uint64_t chunk_number, uint64_t chunk_count)
{
    const uint64_t minimum_p64 = (dlo + 4) / 4;
    const uint32_t minimum_p =
        minimum_p64 > UINT32_MAX ? UINT32_MAX : (uint32_t)minimum_p64;
    const size_t first =
        lower_bound_u32(primes, prime_count, minimum_p);

    A299171SearchJob job;
    memset(&job, 0, sizeof(job));
    job.primes = primes + first;
    job.prime_count = prime_count - first;
    job.sums = sums + first;
    job.h6 = h6;
    job.h6_base = h6_base;
    job.dlo = dlo;
    job.dhi = dhi;
    job.batch = 16;
    atomic_init(&job.next, 0);
    atomic_init(&job.done, 0);
    pthread_mutex_init(&job.print_lock, NULL);
    job.started = wall_seconds();
    job.chunk_number = chunk_number;
    job.chunk_count = chunk_count;

    pthread_t *ids = (pthread_t *)malloc(threads * sizeof(*ids));
    if (ids == NULL) {
        die("cannot allocate A299171 worker handles");
    }
    for (unsigned i = 0; i < threads; ++i) {
        if (pthread_create(&ids[i], NULL, a299171_worker, &job) != 0) {
            die("cannot create A299171 worker");
        }
    }
    for (unsigned i = 0; i < threads; ++i) {
        pthread_join(ids[i], NULL);
    }
    fprintf(stderr, "\n");

    pthread_mutex_destroy(&job.print_lock);
    free(ids);
}

static void a299171_usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s [options]\n"
            "  -n, --limit N       search primes p <= N"
            " (default: 1000000000)\n"
            "  -s, --start N       first p to search (default: 2)\n"
            "  -t, --threads N     worker threads (default: CPU count)\n"
            "  -r, --read-block N  discriminants per search chunk"
            " (default: 512000000)\n"
            "  -d, --table FILE    completed disk-backed H6 table\n"
            "  -o, --output FILE   write an indexed b-file to FILE\n"
            "  -h, --help          show this help\n",
            program);
}

int main(int argc, char **argv)
{
    uint64_t limit64 = 1000000000;
    uint64_t start64 = 2;
    uint64_t read_block = 512000000;
    unsigned threads = default_thread_count();
    const char *table_path = "A299172_H6_disk_1e9.bin";
    const char *output_path = "b299171_1e9.txt";

    for (int i = 1; i < argc; ++i) {
        if ((!strcmp(argv[i], "-n") || !strcmp(argv[i], "--limit")) &&
            i + 1 < argc) {
            limit64 = parse_u64(argv[++i], "limit");
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
        } else if ((!strcmp(argv[i], "-r") ||
                    !strcmp(argv[i], "--read-block")) &&
                   i + 1 < argc) {
            read_block = parse_u64(argv[++i], "read block");
        } else if ((!strcmp(argv[i], "-d") || !strcmp(argv[i], "--table")) &&
                   i + 1 < argc) {
            table_path = argv[++i];
        } else if ((!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) &&
                   i + 1 < argc) {
            output_path = argv[++i];
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            a299171_usage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            a299171_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (limit64 < 2 || limit64 > UINT32_MAX) {
        die("limit must be between 2 and 4294967295");
    }
    if (start64 < 2) {
        start64 = 2;
    }
    if (start64 > limit64) {
        die("start is greater than limit");
    }
    if (read_block == 0 || read_block % 4 != 0) {
        die("read block must be a positive multiple of 4");
    }

    const uint32_t limit = (uint32_t)limit64;
    const uint32_t start = (uint32_t)start64;
    const uint64_t required_dmax = 4 * limit64 - 1;
    const double all_started = wall_seconds();

    DiskTableHeader table_header;
    const int table_fd =
        open_complete_h6_table(table_path, required_dmax, &table_header);

    fprintf(stderr,
            "A299171 Hurwitz-table search\n"
            "condition: tau(p) == 0 (mod p+1)\n"
            "equivalent test: S6(p) == 0 (mod 6*(p+1))\n"
            "limit=%" PRIu32 ", start=%" PRIu32 ", threads=%u\n"
            "read block=%" PRIu64 "\n"
            "table=%s, table limit=%" PRIu64 ", complete=%" PRIu64
            "/%" PRIu64 "\n",
            limit, start, threads, read_block, table_path,
            table_header.nmax, table_header.block_count,
            table_header.block_count);

    size_t all_prime_count = 0;
    uint32_t *all_primes = sieve_primes(limit, &all_prime_count);
    const size_t first =
        lower_bound_u32(all_primes, all_prime_count, start);
    const size_t prime_count = all_prime_count - first;
    uint32_t *primes = all_primes + first;

    uint64_t *sums =
        (uint64_t *)calloc(prime_count, sizeof(*sums));
    if (sums == NULL) {
        die("cannot allocate A299171 residue array");
    }
    fprintf(stderr, "primes in search range: %zu\n", prime_count);

    const uint64_t chunk_count =
        (required_dmax + 1 + read_block - 1) / read_block;
    const uint64_t max_span =
        read_block < required_dmax + 1 ? read_block : required_dmax + 1;
    const size_t max_entries = (size_t)(max_span / 2);
    uint32_t *h6 = (uint32_t *)malloc(max_entries * sizeof(*h6));
    if (h6 == NULL) {
        die("cannot allocate H6 search chunk");
    }
    fprintf(stderr, "search-table RAM chunk: %.1f MiB\n",
            ((double)max_entries * sizeof(*h6)) / (1024.0 * 1024.0));

    for (uint64_t chunk = 0; chunk < chunk_count; ++chunk) {
        const uint64_t dlo = chunk * read_block;
        uint64_t dhi = dlo + read_block - 1;
        if (dhi > required_dmax) {
            dhi = required_dmax;
        }
        const uint64_t base = dlo >> 1;
        const size_t entries = (size_t)((dhi >> 1) - base + 1);

        read_all_at(table_fd, h6, entries * sizeof(*h6),
                    (off_t)(base * sizeof(*h6)));
        process_a299171_chunk(
            primes, prime_count, sums, h6, base, dlo, dhi,
            threads, chunk + 1, chunk_count);
    }
    close(table_fd);

    FILE *out = fopen(output_path, "w");
    if (out == NULL) {
        die("cannot open A299171 output file");
    }

    printf("hits:");
    size_t hit_count = 0;
    for (size_t i = 0; i < prime_count; ++i) {
        if (sums[i] == 0) {
            printf(" %" PRIu32, primes[i]);
            ++hit_count;
            fprintf(out, "%zu %" PRIu32 "\n", hit_count, primes[i]);
        }
    }
    printf("\n");
    fclose(out);

    fprintf(stderr, "finished in %.1f s; %zu hits; output: %s\n",
            wall_seconds() - all_started, hit_count, output_path);

    free(h6);
    free(sums);
    free(all_primes);
    return EXIT_SUCCESS;
}
