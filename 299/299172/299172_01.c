/*
 * Low-memory, disk-backed A299172 search.
 *
 * This file reuses the exact reduced-form enumerator and utility routines
 * from A299172_hurwitz.c.  Its own main program writes the packed H6 table
 * to disk in small independently restartable blocks, then reads larger
 * portions of that file while accumulating the congruence for every prime.
 */
#define main a299172_in_memory_main
#include "A299172_hurwitz.c"
#undef main

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
    char magic[8];
    uint64_t version;
    uint64_t nmax;
    uint64_t dmax;
    uint64_t block_size;
    uint64_t block_count;
    uint64_t table_bytes;
} DiskTableHeader;

typedef struct {
    int table_fd;
    int state_fd;
    uint64_t nmax;
    uint64_t dmax;
    uint64_t block_size;
    uint64_t block_count;
    uint8_t *completed;
    atomic_uint_fast64_t next_block;
    atomic_uint_fast64_t done_count;
    uint64_t printed_count;
    uint64_t report_step;
    pthread_mutex_t print_lock;
    double started;
} DiskBuildJob;

typedef struct {
    const uint32_t *primes;
    size_t sum_base;
    size_t work_first;
    size_t prime_last;
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
} DiskSearchJob;

static const char DISK_MAGIC[8] = {'A', '2', '9', '9', 'H', '6', 'D', '1'};

static void write_all_at(int fd, const void *data, size_t size, off_t offset)
{
    const uint8_t *p = (const uint8_t *)data;
    while (size > 0) {
        const ssize_t written = pwrite(fd, p, size, offset);
        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }
            die("pwrite failed");
        }
        if (written == 0) {
            die("pwrite returned zero");
        }
        p += (size_t)written;
        size -= (size_t)written;
        offset += written;
    }
}

static void read_all_at(int fd, void *data, size_t size, off_t offset)
{
    uint8_t *p = (uint8_t *)data;
    while (size > 0) {
        const ssize_t got = pread(fd, p, size, offset);
        if (got < 0) {
            if (errno == EINTR) {
                continue;
            }
            die("pread failed");
        }
        if (got == 0) {
            die("unexpected end of table file");
        }
        p += (size_t)got;
        size -= (size_t)got;
        offset += got;
    }
}

static bool path_exists(const char *path)
{
    struct stat st;
    return stat(path, &st) == 0;
}

static char *state_path_for(const char *table_path)
{
    const size_t n = strlen(table_path);
    char *path = (char *)malloc(n + 7);
    if (path == NULL) {
        die("cannot allocate state path");
    }
    memcpy(path, table_path, n);
    memcpy(path + n, ".state", 7);
    return path;
}

static void initialize_disk_table(
    const char *table_path, const char *state_path,
    uint64_t nmax, uint64_t dmax, uint64_t block_size,
    int *table_fd_out, int *state_fd_out,
    uint8_t **completed_out, uint64_t *block_count_out)
{
    const uint64_t block_count = (dmax + 1 + block_size - 1) / block_size;
    const uint64_t table_entries = (dmax >> 1) + 1;
    const uint64_t table_bytes = table_entries * sizeof(uint32_t);
    const bool have_table = path_exists(table_path);
    const bool have_state = path_exists(state_path);
    DiskTableHeader header;
    uint8_t *completed = (uint8_t *)calloc((size_t)block_count, 1);
    if (completed == NULL) {
        die("cannot allocate table-state bitmap");
    }

    if (have_table != have_state) {
        die("table and .state must either both exist or both be absent");
    }

    int table_fd;
    int state_fd;
    if (!have_table) {
        table_fd = open(table_path, O_CREAT | O_EXCL | O_RDWR, 0644);
        if (table_fd < 0) {
            die("cannot create table file");
        }
        state_fd = open(state_path, O_CREAT | O_EXCL | O_RDWR, 0644);
        if (state_fd < 0) {
            die("cannot create state file");
        }
        if (ftruncate(table_fd, (off_t)table_bytes) != 0) {
            die("cannot size table file");
        }

        memset(&header, 0, sizeof(header));
        memcpy(header.magic, DISK_MAGIC, sizeof(header.magic));
        header.version = 1;
        header.nmax = nmax;
        header.dmax = dmax;
        header.block_size = block_size;
        header.block_count = block_count;
        header.table_bytes = table_bytes;
        write_all_at(state_fd, &header, sizeof(header), 0);
        write_all_at(state_fd, completed, (size_t)block_count,
                     (off_t)sizeof(header));
    } else {
        struct stat table_stat;
        table_fd = open(table_path, O_RDWR);
        state_fd = open(state_path, O_RDWR);
        if (table_fd < 0 || state_fd < 0) {
            die("cannot open existing table/state file");
        }
        read_all_at(state_fd, &header, sizeof(header), 0);
        if (memcmp(header.magic, DISK_MAGIC, sizeof(header.magic)) != 0 ||
            header.version != 1 ||
            header.nmax != nmax ||
            header.dmax != dmax ||
            header.block_size != block_size ||
            header.block_count != block_count ||
            header.table_bytes != table_bytes) {
            die("existing table metadata does not match this run");
        }
        if (fstat(table_fd, &table_stat) != 0 ||
            (uint64_t)table_stat.st_size != table_bytes) {
            die("existing table file has the wrong size");
        }
        read_all_at(state_fd, completed, (size_t)block_count,
                    (off_t)sizeof(header));
    }

    *table_fd_out = table_fd;
    *state_fd_out = state_fd;
    *completed_out = completed;
    *block_count_out = block_count;
}

static void *disk_build_worker(void *arg)
{
    DiskBuildJob *job = (DiskBuildJob *)arg;
    const size_t max_entries = (size_t)(job->block_size / 2);
    uint32_t *buffer = (uint32_t *)malloc(max_entries * sizeof(*buffer));
    if (buffer == NULL) {
        die("cannot allocate Hurwitz block buffer");
    }

    for (;;) {
        const uint64_t block =
            atomic_fetch_add_explicit(&job->next_block, 1, memory_order_relaxed);
        if (block >= job->block_count) {
            break;
        }
        if (job->completed[block]) {
            continue;
        }

        const uint64_t lo = block * job->block_size;
        uint64_t hi = lo + job->block_size - 1;
        if (hi > job->dmax) {
            hi = job->dmax;
        }
        const uint64_t base = lo >> 1;
        const size_t entries = (size_t)((hi >> 1) - base + 1);
        const size_t bytes = entries * sizeof(*buffer);

        memset(buffer, 0, bytes);
        generate_hurwitz_range(lo, hi, buffer, base);
        write_all_at(job->table_fd, buffer, bytes,
                     (off_t)(base * sizeof(*buffer)));

        const uint8_t one = 1;
        write_all_at(job->state_fd, &one, 1,
                     (off_t)(sizeof(DiskTableHeader) + block));
        job->completed[block] = 1;

        atomic_fetch_add_explicit(&job->done_count, 1, memory_order_relaxed);
        pthread_mutex_lock(&job->print_lock);
        const uint64_t done =
            atomic_load_explicit(&job->done_count, memory_order_relaxed);
        if (done > job->printed_count &&
            (done == job->block_count ||
             done >= job->printed_count + job->report_step)) {
            job->printed_count = done;
            fprintf(stderr,
                    "\rDisk Hurwitz table: %" PRIu64 "/%" PRIu64
                    " blocks (%.1f s)",
                    done, job->block_count, wall_seconds() - job->started);
            fflush(stderr);
        }
        pthread_mutex_unlock(&job->print_lock);
    }

    free(buffer);
    return NULL;
}

static void build_or_resume_disk_table(
    int table_fd, int state_fd, uint8_t *completed,
    uint64_t nmax, uint64_t dmax, uint64_t block_size,
    uint64_t block_count, unsigned threads)
{
    uint64_t already_done = 0;
    for (uint64_t i = 0; i < block_count; ++i) {
        already_done += completed[i] != 0;
    }
    if (already_done == block_count) {
        fprintf(stderr, "disk Hurwitz table is already complete\n");
        return;
    }

    DiskBuildJob job;
    memset(&job, 0, sizeof(job));
    job.table_fd = table_fd;
    job.state_fd = state_fd;
    job.nmax = nmax;
    job.dmax = dmax;
    job.block_size = block_size;
    job.block_count = block_count;
    job.completed = completed;
    atomic_init(&job.next_block, 0);
    atomic_init(&job.done_count, already_done);
    job.printed_count = already_done;
    job.report_step = block_count / 100;
    if (job.report_step == 0) {
        job.report_step = 1;
    }
    pthread_mutex_init(&job.print_lock, NULL);
    job.started = wall_seconds();

    pthread_t *ids = (pthread_t *)malloc(threads * sizeof(*ids));
    if (ids == NULL) {
        die("cannot allocate build thread handles");
    }
    for (unsigned i = 0; i < threads; ++i) {
        if (pthread_create(&ids[i], NULL, disk_build_worker, &job) != 0) {
            die("cannot create disk-table worker");
        }
    }
    for (unsigned i = 0; i < threads; ++i) {
        pthread_join(ids[i], NULL);
    }
    fprintf(stderr, "\n");
    fsync(table_fd);
    fsync(state_fd);
    pthread_mutex_destroy(&job.print_lock);
    free(ids);
}

static inline uint64_t mul_mod_disk(uint64_t a, uint64_t b, uint64_t m)
{
    if (a == 0 || b <= UINT64_MAX / a) {
        return (a * b) % m;
    }
    return (uint64_t)(((__uint128_t)a * b) % m);
}

static uint64_t pow10_mod_disk(uint64_t a, uint64_t m)
{
    a %= m;
    const uint64_t a2 = mul_mod_disk(a, a, m);
    const uint64_t a4 = mul_mod_disk(a2, a2, m);
    const uint64_t a8 = mul_mod_disk(a4, a4, m);
    return mul_mod_disk(a8, a2, m);
}

static void deltas_at(uint64_t t, uint64_t modulus, uint64_t delta[11])
{
    uint64_t values[11];
    for (unsigned j = 0; j <= 10; ++j) {
        values[j] = pow10_mod_disk(t + j, modulus);
    }
    for (unsigned order = 0; order <= 10; ++order) {
        delta[order] = values[0];
        for (unsigned j = 0; j < 10 - order; ++j) {
            if (values[j + 1] >= values[j]) {
                values[j] = values[j + 1] - values[j];
            } else {
                values[j] = modulus - (values[j] - values[j + 1]);
            }
        }
    }
}

static uint64_t ceil_sqrt_u64(uint64_t n)
{
    uint64_t r = isqrt_u64(n);
    if (r * r < n) {
        ++r;
    }
    return r;
}

static void report_disk_search_progress(DiskSearchJob *job, size_t amount)
{
    const size_t done =
        atomic_fetch_add_explicit(&job->done, amount, memory_order_relaxed) + amount;
    const size_t total = job->prime_last - job->work_first;
    unsigned percent = total ? (unsigned)(100 * done / total) : 100;
    if (percent > 100) {
        percent = 100;
    }

    pthread_mutex_lock(&job->print_lock);
    if (percent == 100 || percent >= job->printed_percent + 5) {
        job->printed_percent = percent;
        fprintf(stderr,
                "\rSearch chunk %" PRIu64 "/%" PRIu64 ": %3u%% (%.1f s)",
                job->chunk_number, job->chunk_count, percent,
                wall_seconds() - job->started);
        fflush(stderr);
    }
    pthread_mutex_unlock(&job->print_lock);
}

static void *disk_search_worker(void *arg)
{
    DiskSearchJob *job = (DiskSearchJob *)arg;

    for (;;) {
        const size_t begin =
            atomic_fetch_add_explicit(&job->next, job->batch, memory_order_relaxed);
        if (begin >= job->prime_last) {
            break;
        }
        size_t end = begin + job->batch;
        if (end > job->prime_last) {
            end = job->prime_last;
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

            const uint64_t modulus = 6 * (p - 1);
            uint64_t delta[11];
            deltas_at(t0, modulus, delta);
            uint64_t sum = job->sums[i - job->sum_base];

            for (uint64_t t = t0; t <= t1; ++t) {
                const uint64_t d = fourp - t * t;
                const uint64_t h =
                    job->h6[(d >> 1) - job->h6_base];
                const uint64_t term = mul_mod_disk(delta[0], h, modulus);
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
            job->sums[i - job->sum_base] = sum;
        }
        report_disk_search_progress(job, end - begin);
    }
    return NULL;
}

static void process_search_chunk(
    const uint32_t *primes, size_t prime_first, size_t prime_count,
    uint64_t *sums, const uint32_t *h6, uint64_t h6_base,
    uint64_t dlo, uint64_t dhi, unsigned threads,
    uint64_t chunk_number, uint64_t chunk_count)
{
    const uint64_t minimum_p64 = (dlo + 4) / 4;
    const uint32_t minimum_p =
        minimum_p64 > UINT32_MAX ? UINT32_MAX : (uint32_t)minimum_p64;
    size_t first = lower_bound_u32(primes, prime_count, minimum_p);
    if (first < prime_first) {
        first = prime_first;
    }

    DiskSearchJob job;
    memset(&job, 0, sizeof(job));
    job.primes = primes;
    job.sum_base = prime_first;
    job.work_first = first;
    job.prime_last = prime_count;
    job.sums = sums;
    job.h6 = h6;
    job.h6_base = h6_base;
    job.dlo = dlo;
    job.dhi = dhi;
    job.batch = 16;
    atomic_init(&job.next, first);
    atomic_init(&job.done, 0);
    job.printed_percent = 0;
    pthread_mutex_init(&job.print_lock, NULL);
    job.started = wall_seconds();
    job.chunk_number = chunk_number;
    job.chunk_count = chunk_count;

    pthread_t *ids = (pthread_t *)malloc(threads * sizeof(*ids));
    if (ids == NULL) {
        die("cannot allocate search thread handles");
    }
    for (unsigned i = 0; i < threads; ++i) {
        if (pthread_create(&ids[i], NULL, disk_search_worker, &job) != 0) {
            die("cannot create disk-search worker");
        }
    }
    for (unsigned i = 0; i < threads; ++i) {
        pthread_join(ids[i], NULL);
    }
    fprintf(stderr, "\n");

    pthread_mutex_destroy(&job.print_lock);
    free(ids);
}

static void disk_usage(const char *program)
{
    fprintf(stderr,
            "Usage: %s [options]\n"
            "  -n, --limit N       search primes p <= N (default: 1000000000)\n"
            "  -s, --start N       first p to search (default: 2)\n"
            "  -t, --threads N     worker threads (default: CPU count)\n"
            "  -b, --build-block N discriminants per build block"
            " (default: 1000000)\n"
            "  -r, --read-block N  discriminants per search chunk"
            " (default: 512000000)\n"
            "  -d, --table FILE    disk-backed H6 table file\n"
            "  -o, --output FILE   append hits to FILE\n"
            "  -h, --help          show this help\n",
            program);
}

int a299172_disk_program_main(int argc, char **argv)
{
    uint64_t nmax64 = 1000000000;
    uint64_t start64 = 2;
    uint64_t build_block = 1000000;
    uint64_t read_block = 512000000;
    unsigned threads = default_thread_count();
    const char *table_path = "A299172_H6_disk.bin";
    const char *output_path = "A299172_hurwitz_disk_hits.txt";

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
        } else if ((!strcmp(argv[i], "-b") ||
                    !strcmp(argv[i], "--build-block")) &&
                   i + 1 < argc) {
            build_block = parse_u64(argv[++i], "build block");
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
            disk_usage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            disk_usage(argv[0]);
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
    if (build_block == 0 || build_block % 4 != 0 ||
        read_block == 0 || read_block % 4 != 0) {
        die("build/read block sizes must be positive multiples of 4");
    }

    const uint32_t nmax = (uint32_t)nmax64;
    const uint32_t start = (uint32_t)start64;
    const uint64_t dmax = 4 * nmax64 - 1;
    const double all_started = wall_seconds();
    char *state_path = state_path_for(table_path);

    fprintf(stderr,
            "A299172 disk-backed Hurwitz search\n"
            "limit=%" PRIu32 ", start=%" PRIu32 ", threads=%u\n"
            "build block=%" PRIu64 ", read block=%" PRIu64 "\n"
            "table=%s (%.3f GiB)\n",
            nmax, start, threads, build_block, read_block, table_path,
            ((double)(((dmax >> 1) + 1) * sizeof(uint32_t))) /
                (1024.0 * 1024.0 * 1024.0));

    int table_fd;
    int state_fd;
    uint8_t *completed;
    uint64_t block_count;
    initialize_disk_table(
        table_path, state_path, nmax, dmax, build_block,
        &table_fd, &state_fd, &completed, &block_count);
    build_or_resume_disk_table(
        table_fd, state_fd, completed, nmax, dmax,
        build_block, block_count, threads);
    free(completed);
    close(state_fd);

    size_t prime_count = 0;
    uint32_t *primes = sieve_primes(nmax, &prime_count);
    const size_t prime_first = lower_bound_u32(primes, prime_count, start);
    const size_t search_count = prime_count - prime_first;
    uint64_t *sums = (uint64_t *)malloc(search_count * sizeof(*sums));
    if (sums == NULL) {
        die("cannot allocate residue array");
    }
    for (size_t j = 0; j < search_count; ++j) {
        const uint64_t p = primes[prime_first + j];
        sums[j] = 1008 % (6 * (p - 1));
    }
    fprintf(stderr, "primes in search range: %zu\n", search_count);

    const uint64_t chunk_count =
        (dmax + 1 + read_block - 1) / read_block;
    const uint64_t max_span =
        read_block < dmax + 1 ? read_block : dmax + 1;
    const size_t max_entries = (size_t)(max_span / 2);
    uint32_t *h6 = (uint32_t *)malloc(max_entries * sizeof(*h6));
    if (h6 == NULL) {
        die("cannot allocate search table chunk");
    }
    fprintf(stderr, "search-table RAM chunk: %.1f MiB\n",
            ((double)max_entries * sizeof(*h6)) / (1024.0 * 1024.0));

    for (uint64_t chunk = 0; chunk < chunk_count; ++chunk) {
        const uint64_t dlo = chunk * read_block;
        uint64_t dhi = dlo + read_block - 1;
        if (dhi > dmax) {
            dhi = dmax;
        }
        const uint64_t base = dlo >> 1;
        const size_t entries = (size_t)((dhi >> 1) - base + 1);
        read_all_at(table_fd, h6, entries * sizeof(*h6),
                    (off_t)(base * sizeof(*h6)));

        /*
         * Give workers a primes pointer and sums pointer with the same local
         * origin, so later chunks can skip small primes safely.
         */
        process_search_chunk(
            primes + prime_first, 0, search_count, sums,
            h6, base, dlo, dhi, threads, chunk + 1, chunk_count);
    }
    close(table_fd);

    FILE *out = fopen(output_path, "a");
    if (out == NULL) {
        die("cannot open output file");
    }
    fprintf(out, "# disk limit=%" PRIu32 " start=%" PRIu32 "\n",
            nmax, start);
    printf("hits:");
    size_t hit_count = 0;
    for (size_t j = 0; j < search_count; ++j) {
        if (sums[j] == 0) {
            const uint32_t p = primes[prime_first + j];
            printf(" %" PRIu32, p);
            fprintf(out, "%" PRIu32 "\n", p);
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
    free(primes);
    free(state_path);
    return EXIT_SUCCESS;
}

#ifndef A299172_DISK_NO_MAIN
int main(int argc, char **argv)
{
    return a299172_disk_program_main(argc, argv);
}
#endif
