/*
 * A296580
 *
 * Odd primes p such that
 *
 *     tau(p) == (p - 1)/2  (mod p).
 *
 * This program reuses the completed disk-backed table
 * A299172_H6_disk_1e9.bin.  The table contains
 *
 *     H6(D) = 6 H(D)
 *
 * at packed index D >> 1 for D == 0 or 3 (mod 4).
 *
 * For a prime p, the Eichler--Selberg/Hurwitz formula gives
 *
 *     tau(p) == -s_10(p) - 1  (mod p),
 *
 *     6 s_10(p) = sum_{0<t<2sqrt(p)} t^10 H6(4p-t^2).
 *
 * For p >= 5, the A296580 condition is therefore equivalent to
 *
 *     sum t^10 H6(4p-t^2) + 3 == 0  (mod p).
 *
 * The prime p=3 is checked separately and is not a solution.
 */

#define A299172_DISK_NO_MAIN
#include "A299172_hurwitz_disk.c"

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
    const char *label;
} A296580SearchJob;

static int open_complete_h6_table(
    const char *table_path, uint64_t required_dmax,
    DiskTableHeader *header_out)
{
    char *state_path = state_path_for(table_path);
    if (!path_exists(table_path) || !path_exists(state_path)) {
        free(state_path);
        die("the H6 table and its .state file must already exist");
    }

    const int table_fd = open(table_path, O_RDONLY);
    const int state_fd = open(state_path, O_RDONLY);
    if (table_fd < 0 || state_fd < 0) {
        free(state_path);
        die("cannot open the H6 table or its .state file");
    }

    DiskTableHeader header;
    read_all_at(state_fd, &header, sizeof(header), 0);
    if (memcmp(header.magic, DISK_MAGIC, sizeof(header.magic)) != 0 ||
        header.version != 1) {
        die("unrecognized H6 table metadata");
    }
    if (header.dmax < required_dmax) {
        die("the H6 table is too short for the requested search limit");
    }

    struct stat st;
    if (fstat(table_fd, &st) != 0 ||
        (uint64_t)st.st_size != header.table_bytes) {
        die("the H6 table file has the wrong size");
    }

    uint8_t *completed =
        (uint8_t *)malloc((size_t)header.block_count);
    if (completed == NULL) {
        die("cannot allocate the H6 completion bitmap");
    }
    read_all_at(state_fd, completed, (size_t)header.block_count,
                (off_t)sizeof(header));

    uint64_t done = 0;
    for (uint64_t i = 0; i < header.block_count; ++i) {
        done += completed[i] != 0;
    }
    free(completed);
    close(state_fd);
    free(state_path);

    if (done != header.block_count) {
        fprintf(stderr,
                "error: H6 table is incomplete: %" PRIu64 "/%" PRIu64
                " blocks\n",
                done, header.block_count);
        close(table_fd);
        exit(EXIT_FAILURE);
    }

    *header_out = header;
    return table_fd;
}

static void report_a296580_progress(
    A296580SearchJob *job, size_t amount)
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
                "\r%s chunk %" PRIu64 "/%" PRIu64
                ": %3u%% (%.1f s)",
                job->label, job->chunk_number, job->chunk_count, percent,
                wall_seconds() - job->started);
        fflush(stderr);
    }
    pthread_mutex_unlock(&job->print_lock);
}

static void *a296580_worker(void *arg)
{
    A296580SearchJob *job = (A296580SearchJob *)arg;

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

            uint64_t delta[11];
            deltas_at(t0, p, delta);
            uint64_t sum = job->sums[i];

            for (uint64_t t = t0; t <= t1; ++t) {
                const uint64_t d = fourp - t * t;
                const uint64_t h =
                    job->h6[(d >> 1) - job->h6_base];
                const uint64_t term = mul_mod_disk(delta[0], h, p);

                sum += term;
                if (sum >= p) {
                    sum -= p;
                }

                for (unsigned k = 0; k < 10; ++k) {
                    delta[k] += delta[k + 1];
                    if (delta[k] >= p) {
                        delta[k] -= p;
                    }
                }
            }
            job->sums[i] = sum;
        }

        report_a296580_progress(job, end - begin);
    }

    return NULL;
}

static void process_a296580_chunk(
    const uint32_t *primes, size_t prime_count, uint64_t *sums,
    const uint32_t *h6, uint64_t h6_base,
    uint64_t dlo, uint64_t dhi, unsigned threads,
    uint64_t chunk_number, uint64_t chunk_count, const char *label)
{
    const uint64_t minimum_p64 = (dlo + 4) / 4;
    const uint32_t minimum_p =
        minimum_p64 > UINT32_MAX ? UINT32_MAX : (uint32_t)minimum_p64;
    const size_t first =
        lower_bound_u32(primes, prime_count, minimum_p);

    A296580SearchJob job;
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
    job.label = label;

    pthread_t *ids = (pthread_t *)malloc(threads * sizeof(*ids));
    if (ids == NULL) {
        die("cannot allocate A296580 worker handles");
    }
    for (unsigned i = 0; i < threads; ++i) {
        if (pthread_create(&ids[i], NULL, a296580_worker, &job) != 0) {
            die("cannot create A296580 worker");
        }
    }
    for (unsigned i = 0; i < threads; ++i) {
        pthread_join(ids[i], NULL);
    }
    fprintf(stderr, "\n");

    pthread_mutex_destroy(&job.print_lock);
    free(ids);
}

static void a296580_usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s [options]\n"
            "  -n, --limit N       search odd primes p <= N"
            " (default: 1000000000)\n"
            "  -s, --start N       first p to search (default: 5)\n"
            "  -t, --threads N     worker threads (default: CPU count)\n"
            "  -r, --read-block N  discriminants per search chunk"
            " (default: 512000000)\n"
            "  -d, --table FILE    completed disk-backed H6 table\n"
            "  -o, --output FILE   append hits to FILE\n"
            "  -h, --help          show this help\n",
            program);
}

int a296580_program_main(int argc, char **argv)
{
    uint64_t limit64 = 1000000000;
    uint64_t start64 = 5;
    uint64_t read_block = 512000000;
    unsigned threads = default_thread_count();
    const char *table_path = "A299172_H6_disk_1e9.bin";
    const char *output_path = "b296580_1e9.txt";

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
            a296580_usage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            a296580_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (limit64 < 5 || limit64 > UINT32_MAX) {
        die("limit must be between 5 and 4294967295");
    }
    if (start64 < 5) {
        start64 = 5;
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
            "A296580 Hurwitz-table search\n"
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

    uint64_t *sums = (uint64_t *)malloc(prime_count * sizeof(*sums));
    if (sums == NULL) {
        die("cannot allocate A296580 residue array");
    }
    for (size_t i = 0; i < prime_count; ++i) {
        sums[i] = 3 % primes[i];
    }
    fprintf(stderr, "odd primes in search range: %zu\n", prime_count);

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
        process_a296580_chunk(
            primes, prime_count, sums, h6, base, dlo, dhi,
            threads, chunk + 1, chunk_count, "A296580");
    }
    close(table_fd);

    FILE *out = fopen(output_path, "a");
    if (out == NULL) {
        die("cannot open A296580 output file");
    }
    fprintf(out, "# A296580 limit=%" PRIu32 " start=%" PRIu32 "\n",
            limit, start);

    printf("hits:");
    size_t hit_count = 0;
    for (size_t i = 0; i < prime_count; ++i) {
        if (sums[i] == 0) {
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

    free(h6);
    free(sums);
    free(all_primes);
    return EXIT_SUCCESS;
}

#ifndef A296580_NO_MAIN
int main(int argc, char **argv)
{
    return a296580_program_main(argc, argv);
}
#endif
