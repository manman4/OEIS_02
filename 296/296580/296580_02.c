/*
 * Auxiliary search for the second comment in A296580.
 *
 * Find odd primes p such that
 *
 *     tau(p) == (p + 1)/2  (mod p).
 *
 * This is not the defining congruence of A296580; it checks the related
 * nonexistence statement in the COMMENTS section.  The completed
 * A299172_H6_disk_1e9.bin table is reused without modification.
 *
 * If
 *
 *     S6(p) = sum_{0<t<2sqrt(p)} t^10 * (6 H(4p-t^2)),
 *
 * then, for every prime p >= 5, the Hurwitz class-number formula gives
 *
 *     tau(p) == -S6(p)/6 - 1  (mod p).
 *
 * Since 6 is invertible modulo p, this is equivalently
 *
 *     6 tau(p) == -S6(p) - 6  (mod p).
 *
 * On the other hand, the congruence being tested implies
 *
 *     6 tau(p) == 6*(p + 1)/2
 *              == 3*(p + 1)
 *              == 3                         (mod p).
 *
 * Comparing the two expressions for 6 tau(p) gives
 *
 *     -S6(p) - 6 == 3                       (mod p),
 *
 * and hence
 *
 *     S6(p) + 9 == 0  (mod p).
 *
 * The odd prime p=3 is not a solution:
 * tau(3) == 0 (mod 3), whereas (3+1)/2 == 2 (mod 3).
 */

#define A296580_NO_MAIN
#include "296580_01.c"

static void a296580_plus_usage(const char *program)
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

int main(int argc, char **argv)
{
    uint64_t limit64 = 1000000000;
    uint64_t start64 = 5;
    uint64_t read_block = 512000000;
    unsigned threads = default_thread_count();
    const char *table_path = "A299172_H6_disk_1e9.bin";
    const char *output_path = "b296580_plus_1e9.txt";

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
            a296580_plus_usage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            a296580_plus_usage(argv[0]);
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
            "A296580 complementary-congruence search\n"
            "condition: tau(p) == (p+1)/2 (mod p)\n"
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

    /*
     * The shared engine adds S6(p) modulo p.  Starting at 9 therefore
     * leaves S6(p)+9 modulo p, which is the required test.
     */
    uint64_t *sums = (uint64_t *)malloc(prime_count * sizeof(*sums));
    if (sums == NULL) {
        die("cannot allocate A296580 complementary residue array");
    }
    for (size_t i = 0; i < prime_count; ++i) {
        sums[i] = 9 % primes[i];
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
            threads, chunk + 1, chunk_count, "A296580-plus");
    }
    close(table_fd);

    FILE *out = fopen(output_path, "a");
    if (out == NULL) {
        die("cannot open A296580 complementary output file");
    }
    fprintf(out,
            "# tau(p) == (p+1)/2 (mod p), limit=%" PRIu32
            " start=%" PRIu32 "\n",
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
