/*
 * A291504 -- permutations whose prefix sums are never prime.
 *
 * Count permutations s_1,...,s_n of [n] such that
 *
 *     Sum_{i=1..j} s_i is not prime,   1 <= j <= n.
 *
 * For a subset S of [n], let dp[S] be the number of valid orders of S as
 * an initial segment.  Its sum depends only on S.  Consequently
 *
 *     dp[empty] = 1,
 *     dp[S] = 0                                      if sum(S) is prime,
 *     dp[S] = Sum_{x in S} dp[S \ {x}]               otherwise.
 *
 * The summands are disjoint according to the last element x, so the
 * recurrence is exact.  The answer is dp[[n]].
 *
 * Only two adjacent subset-cardinality layers are retained.  Subsets are
 * enumerated in colexicographic order.  If their zero-based bit positions
 * are b_0 < ... < b_{k-1}, their rank is
 *
 *     Sum_{i=0..k-1} binomial(b_i, i+1).
 *
 * Prefix and suffix rank sums give every predecessor index in O(1) after
 * O(k) preparation.  The algorithm uses O(n*2^n) arithmetic operations and
 * O(binomial(n,floor(n/2))) memory.  At n=30, the largest pair of adjacent
 * U128 layers occupies 4,808,643,120 bytes (about 4.48 GiB), below the
 * program's 6 GiB limit.
 *
 * Counts are exact unsigned 128-bit integers.  Since every count is at most
 * n! and 30! < 2^128, n<=30 is arithmetically safe.  Every addition is also
 * checked explicitly.
 *
 * --upto writes a(0),...,a(MAX_N) to b291504_01.txt via a temporary .part
 * file and prints the same b-file lines to standard output.  --check uses
 * an independent left-to-right exhaustive search for n<=10.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       291504_01.c -o 291504_01
 *
 * Usage:
 *   ./291504_01                 # save a(0)..a(23)
 *   ./291504_01 25
 *   ./291504_01 --upto 25
 *   ./291504_01 --term 25
 *   ./291504_01 --check
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(__SIZEOF_INT128__)
#error "291504_01.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define DEFAULT_MAX_N 23U
#define MAX_N 30U
#define MAX_PREFIX_SUM (MAX_N * (MAX_N + 1U) / 2U)
#define DIRECT_CHECK_MAX_N 10U
#define MEMORY_LIMIT_BYTES (UINT64_C(6) * 1024U * 1024U * 1024U)
#define BFILE_PATH "b291504_01.txt"
#define PART_PATH "b291504_01.txt.part"

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} Mode;

static _Noreturn void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
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

static void checked_add(U128 *target, U128 value)
{
    const U128 maximum = ~(U128)0;
    if (*target > maximum - value) {
        die("unsigned 128-bit count overflow");
    }
    *target += value;
}

static int print_u128(FILE *stream, U128 value)
{
    char reverse[40];
    char text[40];
    size_t length = 0U;

    do {
        reverse[length++] = (char)('0' + (unsigned)(value % 10U));
        value /= 10U;
    } while (value != 0U);
    for (size_t i = 0U; i < length; ++i) {
        text[i] = reverse[length - 1U - i];
    }
    text[length] = '\0';
    return fputs(text, stream) == EOF ? -1 : 0;
}

static uint64_t binomial[MAX_N + 1U][MAX_N + 1U];
static bool prime[MAX_PREFIX_SUM + 1U];
static bool tables_ready;

static void prepare_tables(void)
{
    if (tables_ready) {
        return;
    }

    binomial[0][0] = 1U;
    for (unsigned n = 1U; n <= MAX_N; ++n) {
        binomial[n][0] = 1U;
        binomial[n][n] = 1U;
        for (unsigned k = 1U; k < n; ++k) {
            binomial[n][k] =
                binomial[n - 1U][k - 1U] + binomial[n - 1U][k];
        }
    }

    for (unsigned value = 2U; value <= MAX_PREFIX_SUM; ++value) {
        prime[value] = true;
    }
    for (unsigned divisor = 2U;
         divisor <= MAX_PREFIX_SUM / divisor; ++divisor) {
        if (!prime[divisor]) {
            continue;
        }
        for (unsigned multiple = divisor * divisor;
             multiple <= MAX_PREFIX_SUM; multiple += divisor) {
            prime[multiple] = false;
        }
    }
    tables_ready = true;
}

static U128 *allocate_layer(uint64_t count, unsigned n, unsigned cardinality)
{
    if (count > SIZE_MAX / sizeof(U128)) {
        die("DP allocation size overflow");
    }
    const uint64_t bytes = count * sizeof(U128);
    if (bytes > MEMORY_LIMIT_BYTES) {
        die("one DP layer exceeds the 6 GiB memory limit");
    }
    U128 *layer = malloc((size_t)bytes);
    if (layer == NULL) {
        fprintf(stderr,
                "error: cannot allocate n=%u, cardinality=%u DP layer "
                "(%.1f MiB)\n",
                n, cardinality, (double)bytes / (1024.0 * 1024.0));
        exit(EXIT_FAILURE);
    }
    return layer;
}

/* Gosper's hack: the next larger mask with the same nonzero popcount. */
static uint32_t next_combination(uint32_t mask)
{
    const uint32_t low_bit = mask & (0U - mask);
    const uint32_t ripple = mask + low_bit;
    return ripple | (((mask ^ ripple) >> 2U) / low_bit);
}

static U128 count_dp(unsigned n)
{
    if (n == 0U) {
        return 1U;
    }

    prepare_tables();
    const unsigned total_sum = n * (n + 1U) / 2U;
    if (prime[total_sum]) {
        return 0U;
    }

    uint64_t previous_count = 1U;
    U128 *previous = allocate_layer(previous_count, n, 0U);
    previous[0] = 1U;

    for (unsigned cardinality = 1U; cardinality <= n; ++cardinality) {
        const uint64_t current_count = binomial[n][cardinality];
        if (previous_count > UINT64_MAX - current_count ||
            previous_count + current_count >
                MEMORY_LIMIT_BYTES / sizeof(U128)) {
            free(previous);
            die("adjacent DP layers exceed the 6 GiB memory limit");
        }
        U128 *current = allocate_layer(current_count, n, cardinality);
        uint32_t mask = (UINT32_C(1) << cardinality) - 1U;

        for (uint64_t index = 0U; index < current_count; ++index) {
            unsigned positions[MAX_N];
            uint32_t bits = mask;
            unsigned sum = 0U;

            for (unsigned i = 0U; i < cardinality; ++i) {
                const unsigned bit = (unsigned)__builtin_ctz(bits);
                bits &= bits - 1U;
                positions[i] = bit;
                sum += bit + 1U;
            }

            U128 count = 0U;
            if (!prime[sum]) {
                uint64_t prefix_rank[MAX_N + 1U];
                uint64_t shifted_suffix_rank[MAX_N + 1U];

                prefix_rank[0] = 0U;
                for (unsigned i = 0U; i < cardinality; ++i) {
                    prefix_rank[i + 1U] = prefix_rank[i] +
                        binomial[positions[i]][i + 1U];
                }
                shifted_suffix_rank[cardinality] = 0U;
                for (unsigned i = cardinality; i-- > 0U;) {
                    shifted_suffix_rank[i] = shifted_suffix_rank[i + 1U] +
                        binomial[positions[i]][i];
                }

                for (unsigned removed = 0U;
                     removed < cardinality; ++removed) {
                    const uint64_t predecessor = prefix_rank[removed] +
                        shifted_suffix_rank[removed + 1U];
                    if (predecessor >= previous_count) {
                        free(current);
                        free(previous);
                        die("internal combinatorial rank error");
                    }
                    checked_add(&count, previous[predecessor]);
                }
            }
            current[index] = count;
            if (index + 1U < current_count) {
                mask = next_combination(mask);
            }
        }

        free(previous);
        previous = current;
        previous_count = current_count;
    }

    if (previous_count != 1U) {
        free(previous);
        die("internal final-layer size error");
    }
    const U128 answer = previous[0];
    free(previous);
    return answer;
}

/* Independent definition-level DFS used only by --check. */
static U128 direct_search(unsigned n, unsigned position, uint32_t used,
                          unsigned prefix_sum)
{
    if (position == n) {
        return 1U;
    }

    U128 count = 0U;
    for (unsigned value = 1U; value <= n; ++value) {
        const uint32_t bit = UINT32_C(1) << (value - 1U);
        if ((used & bit) != 0U || prime[prefix_sum + value]) {
            continue;
        }
        checked_add(&count, direct_search(
            n, position + 1U, used | bit, prefix_sum + value));
    }
    return count;
}

static int write_bfile_line(FILE *stream, unsigned n, U128 value)
{
    if (fprintf(stream, "%u ", n) < 0 || print_u128(stream, value) != 0 ||
        fputc('\n', stream) == EOF) {
        return -1;
    }
    return 0;
}

static void write_sequence(unsigned maximum)
{
    FILE *output = fopen(PART_PATH, "w");
    if (output == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                PART_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    }

    for (unsigned n = 0U; n <= maximum; ++n) {
        const U128 value = count_dp(n);
        if (write_bfile_line(stdout, n, value) != 0 ||
            write_bfile_line(output, n, value) != 0 ||
            fflush(stdout) != 0 || fflush(output) != 0) {
            (void)fclose(output);
            fprintf(stderr, "error: cannot write output: %s\n",
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    if (fclose(output) != 0) {
        fprintf(stderr, "error: cannot close %s: %s\n",
                PART_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (rename(PART_PATH, BFILE_PATH) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                PART_PATH, BFILE_PATH, strerror(errno));
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "saved: %s; n=0..%u\n", BFILE_PATH, maximum);
}

static void check_implementation(unsigned maximum)
{
    prepare_tables();
    for (unsigned n = 0U; n <= maximum; ++n) {
        const U128 dynamic = count_dp(n);
        const U128 direct = direct_search(n, 0U, 0U, 0U);
        if (dynamic != direct) {
            fprintf(stderr, "error: DP/direct mismatch at n=%u: ", n);
            (void)print_u128(stderr, dynamic);
            fputs(" != ", stderr);
            (void)print_u128(stderr, direct);
            fputc('\n', stderr);
            exit(EXIT_FAILURE);
        }
    }
    printf("ok: subset DP agrees with direct enumeration for n=0..%u\n",
           maximum);
}

static void usage(const char *program, FILE *stream)
{
    fprintf(stream,
            "Usage:\n"
            "  %s [MAX_N]\n"
            "  %s --upto MAX_N\n"
            "  %s --term N\n"
            "  %s --check [CHECK_N]\n"
            "\n"
            "MAX_N and N may be 0..%u; the default MAX_N is %u.\n"
            "CHECK_N may be 0..%u and defaults to %u.\n"
            "--upto saves a(0)..a(MAX_N) as %s.\n"
            "The maximum DP allocation is limited to 6 GiB.\n",
            program, program, program, program,
            MAX_N, DEFAULT_MAX_N,
            DIRECT_CHECK_MAX_N, DIRECT_CHECK_MAX_N, BFILE_PATH);
}

int main(int argc, char **argv)
{
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        usage(argv[0], stdout);
        return EXIT_SUCCESS;
    }

    Mode mode = MODE_UPTO;
    unsigned n = DEFAULT_MAX_N;
    if (argc == 1) {
        write_sequence(n);
        return EXIT_SUCCESS;
    }

    if (strcmp(argv[1], "--term") == 0 ||
        strcmp(argv[1], "--upto") == 0) {
        if (argc != 3) {
            usage(argv[0], stderr);
            return EXIT_FAILURE;
        }
        mode = strcmp(argv[1], "--term") == 0 ? MODE_TERM : MODE_UPTO;
        n = parse_unsigned(argv[2], MAX_N,
                           mode == MODE_TERM ? "N" : "MAX_N");
    } else if (strcmp(argv[1], "--check") == 0) {
        if (argc > 3) {
            usage(argv[0], stderr);
            return EXIT_FAILURE;
        }
        mode = MODE_CHECK;
        n = argc == 3 ?
            parse_unsigned(argv[2], DIRECT_CHECK_MAX_N, "CHECK_N") :
            DIRECT_CHECK_MAX_N;
    } else {
        if (argc != 2 || argv[1][0] == '-') {
            usage(argv[0], stderr);
            return EXIT_FAILURE;
        }
        n = parse_unsigned(argv[1], MAX_N, "MAX_N");
    }

    if (mode == MODE_CHECK) {
        check_implementation(n);
    } else if (mode == MODE_TERM) {
        if (print_u128(stdout, count_dp(n)) != 0 || putchar('\n') == EOF) {
            die("cannot write standard output");
        }
    } else {
        write_sequence(n);
    }
    return EXIT_SUCCESS;
}
