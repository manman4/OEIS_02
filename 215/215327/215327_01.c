#include <errno.h>
#include <getopt.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmp.h>

#define DEFAULT_MAX_N 12UL

typedef struct {
    mpz_t s;
    mpz_t l;
    mpz_t c;
} Counts;

typedef struct {
    size_t q;
    size_t n;
    size_t *word;
    Counts *counts;
} Generator;

static void
usage(FILE *stream, const char *program)
{
    fprintf(stream,
            "Usage: %s [OPTIONS] Q [MAX_N]\n"
            "\n"
            "Default output is a two-column b-file-style table for S_q(n).\n"
            "S_q and L_q start with the OEIS convention 0 1; C_q starts at n=1.\n"
            "MAX_N defaults to %lu. Enumeration is exact but exponential.\n"
            "\n"
            "Options:\n"
            "  -s, --sequence NAME  output S, L, or C (default: S)\n"
            "      --check          show S, L, C, the identity RHS, and check status\n"
            "  -h, --help           show this help\n",
            program, DEFAULT_MAX_N);
}

static size_t
parse_positive_size(const char *text, const char *name)
{
    char *end = NULL;
    uintmax_t value;

    if (text[0] == '\0' || text[0] == '-') {
        fprintf(stderr, "%s must be a positive integer\n", name);
        exit(EXIT_FAILURE);
    }
    errno = 0;
    value = strtoumax(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' || value == 0 ||
        value > SIZE_MAX || value > ULONG_MAX) {
        fprintf(stderr, "%s must be a positive integer in the supported range\n",
                name);
        exit(EXIT_FAILURE);
    }
    return (size_t)value;
}

static void *
xcalloc(size_t count, size_t size)
{
    void *pointer;

    if (size != 0 && count > SIZE_MAX / size) {
        fprintf(stderr, "allocation size overflow\n");
        exit(EXIT_FAILURE);
    }
    pointer = calloc(count, size);
    if (pointer == NULL) {
        fprintf(stderr, "memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    return pointer;
}

static int
letters_are_smooth(size_t a, size_t b)
{
    return a >= b ? a - b <= 1 : b - a <= 1;
}

/*
 * The FKM recursion emits exactly the lexicographically least representative
 * of every q-ary necklace of length n.  At an emitted representative, period
 * is its least period.  The local rejection below imposes smoothness on the
 * linear representative without changing the FKM canonicality test.
 */
static void
generate_necklaces(Generator *generator, size_t position, size_t period)
{
    size_t copied_letter;
    size_t letter;

    if (position > generator->n) {
        if (generator->n % period != 0) {
            return;
        }
        mpz_add_ui(generator->counts->s, generator->counts->s, 1);
        if (period == generator->n) {
            mpz_add_ui(generator->counts->l, generator->counts->l, 1);
            if (letters_are_smooth(generator->word[generator->n],
                                   generator->word[1])) {
                mpz_add_ui(generator->counts->c, generator->counts->c, 1);
            }
        }
        return;
    }

    copied_letter = generator->word[position - period];
    generator->word[position] = copied_letter;
    if (position == 1 ||
        letters_are_smooth(generator->word[position],
                           generator->word[position - 1])) {
        generate_necklaces(generator, position + 1, period);
    }

    if (copied_letter == generator->q - 1) {
        return;
    }
    letter = copied_letter + 1;
    for (;;) {
        generator->word[position] = letter;
        if (position == 1 ||
            letters_are_smooth(generator->word[position],
                               generator->word[position - 1])) {
            generate_necklaces(generator, position + 1, position);
        }
        if (letter == generator->q - 1) {
            break;
        }
        ++letter;
    }
}

static void
count_smooth_objects(size_t q, size_t n, Counts *counts)
{
    Generator generator;

    if (n == SIZE_MAX) {
        fprintf(stderr, "MAX_N is too large\n");
        exit(EXIT_FAILURE);
    }
    generator.q = q;
    generator.n = n;
    generator.word = xcalloc(n + 1, sizeof(*generator.word));
    generator.counts = counts;
    generate_necklaces(&generator, 1, 1);
    free(generator.word);
}

static int
mobius(size_t n)
{
    size_t value = n;
    size_t prime = 2;
    unsigned int prime_factors = 0;

    while (prime <= value / prime) {
        if (value % prime == 0) {
            value /= prime;
            if (value % prime == 0) {
                return 0;
            }
            ++prime_factors;
            while (value % prime == 0) {
                value /= prime;
            }
        }
        prime += prime == 2 ? 1 : 2;
    }
    if (value > 1) {
        ++prime_factors;
    }
    return prime_factors % 2 == 0 ? 1 : -1;
}

/*
 * If P is the current power of the tridiagonal adjacency matrix A, then
 * (P A)[row,column] is the sum of at most three neighboring entries of P.
 * This computes all required traces exactly in O(max_n q^2) GMP additions.
 */
static void
trace_powers(size_t q, size_t max_n, mpz_t *traces)
{
    size_t cells;
    mpz_t *power;
    mpz_t *next;
    size_t index;
    size_t n;
    size_t row;
    size_t column;

    if (q > SIZE_MAX / q) {
        fprintf(stderr, "Q is too large for the trace matrix\n");
        exit(EXIT_FAILURE);
    }
    cells = q * q;
    power = xcalloc(cells, sizeof(*power));
    next = xcalloc(cells, sizeof(*next));
    for (index = 0; index < cells; ++index) {
        mpz_init(power[index]);
        mpz_init(next[index]);
    }
    for (row = 0; row < q; ++row) {
        mpz_set_ui(power[row * q + row], 1);
    }

    for (n = 1; n <= max_n; ++n) {
        for (row = 0; row < q; ++row) {
            for (column = 0; column < q; ++column) {
                mpz_set(next[row * q + column], power[row * q + column]);
                if (column > 0) {
                    mpz_add(next[row * q + column],
                            next[row * q + column],
                            power[row * q + column - 1]);
                }
                if (column + 1 < q) {
                    mpz_add(next[row * q + column],
                            next[row * q + column],
                            power[row * q + column + 1]);
                }
            }
        }
        for (row = 0; row < q; ++row) {
            mpz_add(traces[n], traces[n], next[row * q + row]);
        }
        {
            mpz_t *temporary = power;
            power = next;
            next = temporary;
        }
    }

    for (index = 0; index < cells; ++index) {
        mpz_clear(power[index]);
        mpz_clear(next[index]);
    }
    free(power);
    free(next);
}

static void
cyclic_lyndon_formula(size_t n, mpz_t *traces, mpz_t result)
{
    mpz_t numerator;
    size_t e;

    mpz_init(numerator);
    for (e = 1; e <= n; ++e) {
        int mu;

        if (n % e != 0) {
            continue;
        }
        mu = mobius(e);
        if (mu > 0) {
            mpz_add(numerator, numerator, traces[n / e]);
        } else if (mu < 0) {
            mpz_sub(numerator, numerator, traces[n / e]);
        }
    }
    if (!mpz_divisible_ui_p(numerator, (unsigned long)n)) {
        fprintf(stderr, "internal error: C numerator is not divisible by %zu\n",
                n);
        mpz_clear(numerator);
        exit(EXIT_FAILURE);
    }
    mpz_divexact_ui(result, numerator, (unsigned long)n);
    mpz_clear(numerator);
}

static void
print_value(size_t n, const mpz_t value)
{
    printf("%zu ", n);
    mpz_out_str(stdout, 10, value);
    putchar('\n');
}

static void
flush_stdout(void)
{
    if (fflush(stdout) == EOF) {
        perror("stdout");
        exit(EXIT_FAILURE);
    }
}

int
main(int argc, char **argv)
{
    static const struct option long_options[] = {
        {"sequence", required_argument, NULL, 's'},
        {"check", no_argument, NULL, 1000},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    const char *program = strrchr(argv[0], '/');
    char sequence = 'S';
    int check = 0;
    int option;
    size_t q;
    size_t max_n;
    Counts *counts;
    mpz_t *traces;
    mpz_t *formula_c;
    mpz_t *right_hand_sides;
    size_t n;

    program = program == NULL ? argv[0] : program + 1;
    while ((option = getopt_long(argc, argv, "s:h", long_options, NULL)) != -1) {
        switch (option) {
        case 's':
            if (strlen(optarg) != 1 ||
                (optarg[0] != 'S' && optarg[0] != 's' &&
                 optarg[0] != 'L' && optarg[0] != 'l' &&
                 optarg[0] != 'C' && optarg[0] != 'c')) {
                fprintf(stderr, "--sequence must be S, L, or C\n");
                return EXIT_FAILURE;
            }
            sequence = optarg[0];
            if (sequence >= 'a' && sequence <= 'z') {
                sequence = (char)(sequence - 'a' + 'A');
            }
            break;
        case 1000:
            check = 1;
            break;
        case 'h':
            usage(stdout, program);
            return EXIT_SUCCESS;
        default:
            usage(stderr, program);
            return EXIT_FAILURE;
        }
    }
    if (argc - optind < 1 || argc - optind > 2) {
        usage(stderr, program);
        return EXIT_FAILURE;
    }

    q = parse_positive_size(argv[optind], "Q");
    max_n = argc - optind == 2
                ? parse_positive_size(argv[optind + 1], "MAX_N")
                : DEFAULT_MAX_N;
    if (max_n == SIZE_MAX) {
        fprintf(stderr, "MAX_N is too large\n");
        return EXIT_FAILURE;
    }

    counts = xcalloc(max_n + 1, sizeof(*counts));
    traces = xcalloc(max_n + 1, sizeof(*traces));
    formula_c = xcalloc(max_n + 1, sizeof(*formula_c));
    right_hand_sides = xcalloc(max_n + 1, sizeof(*right_hand_sides));
    for (n = 0; n <= max_n; ++n) {
        mpz_init(counts[n].s);
        mpz_init(counts[n].l);
        mpz_init(counts[n].c);
        mpz_init(traces[n]);
        mpz_init(formula_c[n]);
        mpz_init(right_hand_sides[n]);
    }

    if (check) {
        printf("# q=%zu\n", q);
        puts("# n S_q(n) L_q(n) C_q(n) RHS check");
    } else if (sequence != 'C') {
        puts("0 1");
    }
    flush_stdout();

    trace_powers(q, max_n, traces);
    for (n = 1; n <= max_n; ++n) {
        size_t d;

        count_smooth_objects(q, n, &counts[n]);
        cyclic_lyndon_formula(n, traces, formula_c[n]);
        for (d = 1; d < n; ++d) {
            if (n % d == 0) {
                mpz_add(right_hand_sides[n], right_hand_sides[n], counts[d].c);
            }
        }
        mpz_add(right_hand_sides[n], right_hand_sides[n], counts[n].l);

        if (mpz_cmp(counts[n].c, formula_c[n]) != 0) {
            gmp_fprintf(stderr,
                        "C check failed at n=%zu: enumeration=%Zd, formula=%Zd\n",
                        n, counts[n].c, formula_c[n]);
            return EXIT_FAILURE;
        }
        if (mpz_cmp(counts[n].s, right_hand_sides[n]) != 0) {
            gmp_fprintf(stderr,
                        "identity failed at n=%zu: S=%Zd, RHS=%Zd\n",
                        n, counts[n].s, right_hand_sides[n]);
            return EXIT_FAILURE;
        }

        if (check) {
            gmp_printf("%zu %Zd %Zd %Zd %Zd OK\n", n, counts[n].s,
                       counts[n].l, counts[n].c, right_hand_sides[n]);
        } else if (sequence == 'S') {
            print_value(n, counts[n].s);
        } else if (sequence == 'L') {
            print_value(n, counts[n].l);
        } else {
            print_value(n, counts[n].c);
        }
        flush_stdout();
    }

    if (check) {
        fprintf(stderr,
                "Verified C_q(n) independently and S_q(n) = L_q(n) + "
                "sum_{d|n, d!=n} C_q(d) for q=%zu, 1<=n<=%zu.\n",
                q, max_n);
    }

    for (n = 0; n <= max_n; ++n) {
        mpz_clear(counts[n].s);
        mpz_clear(counts[n].l);
        mpz_clear(counts[n].c);
        mpz_clear(traces[n]);
        mpz_clear(formula_c[n]);
        mpz_clear(right_hand_sides[n]);
    }
    free(counts);
    free(traces);
    free(formula_c);
    free(right_hand_sides);
    return EXIT_SUCCESS;
}
