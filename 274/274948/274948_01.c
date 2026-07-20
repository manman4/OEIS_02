/*
 * A274948
 * T(n,k) is the maximum number of unattacked vacant squares when k queens
 * are placed on an n X n board, for 0 <= k <= n^2.
 *
 * Compile on macOS with the alias from the accompanying instructions:
 *   gcc-omp -std=c11 -Wall -Wextra -Wconversion 274948_01.c -o 274948_01
 *
 * Run:
 *   ./274948_01 6
 *   ./274948_01 --verify 5
 *
 * The optimized search uses only exact reductions:
 *   - attacked/occupied squares grow monotonically when queens are added;
 *   - one lexicographically least prefix is retained under the eight square
 *     symmetries;
 *   - if q queens leave s safe squares, queens placed on those s squares
 *     leave the original q squares safe (a witnessed dual upper bound).
 *
 * --verify independently enumerates every relevant subset without symmetry,
 * duality, or branch-and-bound for n <= 5.
 */

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_LIMIT 5
#define SYMMETRIES 8
#define DIRECT_VERIFY_MAX_N 5

typedef struct {
    int n;
    int size;
    int words;
    int zero_threshold;
    int maximum_searched_k;
    uint64_t *attack;
    int *transform;
    int *minimum_image;
} Problem;

typedef struct {
    int first;
    int second;
} Pair;

typedef struct {
    const Problem *problem;
    int *best_attacked;
    int *chosen;
    int *next_square;
    int *attacked_count;
    uint64_t *attacked_stack;
    uint64_t nodes;
    bool overflow;
} Search;

static bool checked_mul_size(size_t left, size_t right, size_t *result) {
    if (left != 0 && right > SIZE_MAX / left) return false;
    *result = left * right;
    return true;
}

static bool checked_add_u64(uint64_t *value, uint64_t addition) {
    if (addition > UINT64_MAX - *value) return false;
    *value += addition;
    return true;
}

static uint64_t *attack_mask(const Problem *problem, int square) {
    return problem->attack + (size_t)square * (size_t)problem->words;
}

static const uint64_t *const_attack_mask(const Problem *problem, int square) {
    return problem->attack + (size_t)square * (size_t)problem->words;
}

static int *transform_map(const Problem *problem, int symmetry) {
    return problem->transform + (size_t)symmetry * (size_t)problem->size;
}

static const int *const_transform_map(const Problem *problem, int symmetry) {
    return problem->transform + (size_t)symmetry * (size_t)problem->size;
}

static void bitset_set(uint64_t *set, int bit) {
    set[bit >> 6] |= UINT64_C(1) << (bit & 63);
}

static int transformed_square(int symmetry, int n, int square) {
    const int row = square / n;
    const int col = square % n;
    int rr;
    int cc;

    switch (symmetry) {
    case 0: rr = row;         cc = col;         break;
    case 1: rr = col;         cc = n - 1 - row; break;
    case 2: rr = n - 1 - row; cc = n - 1 - col; break;
    case 3: rr = n - 1 - col; cc = row;         break;
    case 4: rr = row;         cc = n - 1 - col; break;
    case 5: rr = n - 1 - col; cc = n - 1 - row; break;
    case 6: rr = n - 1 - row; cc = col;         break;
    default: rr = col;        cc = row;         break;
    }
    return rr * n + cc;
}

static bool problem_init(Problem *problem, int n) {
    memset(problem, 0, sizeof(*problem));

    const int64_t size64 = (int64_t)n * (int64_t)n;
    const int64_t threshold64 = size64 - 3 * (int64_t)n + 3;
    if (size64 > INT_MAX || threshold64 < 1 || threshold64 > INT_MAX) {
        return false;
    }

    problem->n = n;
    problem->size = (int)size64;
    problem->words = (problem->size - 1) / 64 + 1;
    problem->zero_threshold = (int)threshold64;
    problem->maximum_searched_k = problem->zero_threshold - 1;

    size_t attack_elements;
    size_t attack_bytes;
    size_t transform_elements;
    size_t transform_bytes;
    size_t minimum_bytes;
    if (!checked_mul_size((size_t)problem->size, (size_t)problem->words,
                          &attack_elements) ||
        !checked_mul_size(attack_elements, sizeof(*problem->attack),
                          &attack_bytes) ||
        !checked_mul_size(SYMMETRIES, (size_t)problem->size,
                          &transform_elements) ||
        !checked_mul_size(transform_elements, sizeof(*problem->transform),
                          &transform_bytes) ||
        !checked_mul_size((size_t)problem->size,
                          sizeof(*problem->minimum_image), &minimum_bytes)) {
        return false;
    }

    problem->attack = calloc(1, attack_bytes);
    problem->transform = malloc(transform_bytes);
    problem->minimum_image = malloc(minimum_bytes);
    if (problem->attack == NULL || problem->transform == NULL ||
        problem->minimum_image == NULL) {
        free(problem->attack);
        free(problem->transform);
        free(problem->minimum_image);
        memset(problem, 0, sizeof(*problem));
        return false;
    }

    for (int queen = 0; queen < problem->size; ++queen) {
        uint64_t *mask = attack_mask(problem, queen);
        const int queen_row = queen / n;
        const int queen_col = queen % n;
        for (int square = 0; square < problem->size; ++square) {
            const int row = square / n;
            const int col = square % n;
            int row_difference = row - queen_row;
            int col_difference = col - queen_col;
            if (row_difference < 0) row_difference = -row_difference;
            if (col_difference < 0) col_difference = -col_difference;
            if (row == queen_row || col == queen_col ||
                row_difference == col_difference) {
                bitset_set(mask, square);
            }
        }
    }

    for (int symmetry = 0; symmetry < SYMMETRIES; ++symmetry) {
        int *map = transform_map(problem, symmetry);
        for (int square = 0; square < problem->size; ++square) {
            map[square] = transformed_square(symmetry, n, square);
        }
    }

    for (int square = 0; square < problem->size; ++square) {
        int minimum = square;
        for (int symmetry = 1; symmetry < SYMMETRIES; ++symmetry) {
            const int image = const_transform_map(problem, symmetry)[square];
            if (image < minimum) minimum = image;
        }
        problem->minimum_image[square] = minimum;
    }
    return true;
}

static void problem_destroy(Problem *problem) {
    free(problem->attack);
    free(problem->transform);
    free(problem->minimum_image);
    memset(problem, 0, sizeof(*problem));
}

static bool canonical_pair(const Problem *problem, int first, int second) {
    for (int symmetry = 0; symmetry < SYMMETRIES; ++symmetry) {
        const int *map = const_transform_map(problem, symmetry);
        int a = map[first];
        int b = map[second];
        if (a > b) {
            const int temporary = a;
            a = b;
            b = temporary;
        }
        if (a < first || (a == first && b < second)) return false;
    }
    return true;
}

static bool canonical_triple(const Problem *problem, int first, int second,
                             int third) {
    for (int symmetry = 0; symmetry < SYMMETRIES; ++symmetry) {
        const int *map = const_transform_map(problem, symmetry);
        int a = map[first];
        int b = map[second];
        int c = map[third];
        int temporary;
        if (a > b) { temporary = a; a = b; b = temporary; }
        if (b > c) { temporary = b; b = c; c = temporary; }
        if (a > b) { temporary = a; a = b; b = temporary; }
        if (a < first ||
            (a == first &&
             (b < second || (b == second && c < third)))) {
            return false;
        }
    }
    return true;
}

static int bitset_count(const uint64_t *set, int words) {
    int count = 0;
    for (int word = 0; word < words; ++word) {
        count += __builtin_popcountll(set[word]);
    }
    return count;
}

static void record_bounds(const Problem *problem, int *best_attacked,
                          int queens, int attacked_count) {
    if (attacked_count < best_attacked[queens]) {
        best_attacked[queens] = attacked_count;
    }

    const int safe = problem->size - attacked_count;
    int dual_k = safe < problem->maximum_searched_k
                     ? safe : problem->maximum_searched_k;
    const int dual_attacked = problem->size - queens;
    while (dual_k > 0 && best_attacked[dual_k] > dual_attacked) {
        best_attacked[dual_k] = dual_attacked;
        --dual_k;
    }
}

static bool best_is_nondecreasing(const Problem *problem, const int *best) {
    for (int k = 2; k <= problem->maximum_searched_k; ++k) {
        if (best[k - 1] > best[k]) return false;
    }
    return true;
}

static bool seed_best_bounds(const Problem *problem, int *best_attacked) {
    for (int k = 0; k <= problem->maximum_searched_k; ++k) {
        best_attacked[k] = problem->size;
    }
    if (problem->maximum_searched_k == 0) return true;

    size_t union_bytes;
    if (!checked_mul_size((size_t)problem->words, sizeof(uint64_t),
                          &union_bytes)) {
        return false;
    }
    uint64_t *attacked = calloc(1, union_bytes);
    if (attacked == NULL) return false;

    int attacked_count = 0;
    for (int k = 1; k <= problem->maximum_searched_k; ++k) {
        const uint64_t *mask = const_attack_mask(problem, k - 1);
        for (int word = 0; word < problem->words; ++word) {
            const uint64_t new_bits = mask[word] & ~attacked[word];
            attacked[word] |= new_bits;
            attacked_count += __builtin_popcountll(new_bits);
        }
        record_bounds(problem, best_attacked, k, attacked_count);
    }
    free(attacked);
    return best_is_nondecreasing(problem, best_attacked);
}

static bool search_allocate(Search *search, const Problem *problem,
                            const int *seed_best) {
    memset(search, 0, sizeof(*search));
    search->problem = problem;

    const size_t levels = (size_t)problem->maximum_searched_k + 1;
    size_t level_int_bytes;
    size_t stack_elements;
    size_t stack_bytes;
    if (!checked_mul_size(levels, sizeof(int), &level_int_bytes) ||
        !checked_mul_size(levels, (size_t)problem->words, &stack_elements) ||
        !checked_mul_size(stack_elements, sizeof(uint64_t), &stack_bytes)) {
        return false;
    }

    search->best_attacked = malloc(level_int_bytes);
    search->chosen = malloc(level_int_bytes);
    search->next_square = malloc(level_int_bytes);
    search->attacked_count = malloc(level_int_bytes);
    search->attacked_stack = malloc(stack_bytes);
    if (search->best_attacked == NULL || search->chosen == NULL ||
        search->next_square == NULL || search->attacked_count == NULL ||
        search->attacked_stack == NULL) {
        free(search->best_attacked);
        free(search->chosen);
        free(search->next_square);
        free(search->attacked_count);
        free(search->attacked_stack);
        memset(search, 0, sizeof(*search));
        return false;
    }
    memcpy(search->best_attacked, seed_best, level_int_bytes);
    return true;
}

static void search_destroy(Search *search) {
    free(search->best_attacked);
    free(search->chosen);
    free(search->next_square);
    free(search->attacked_count);
    free(search->attacked_stack);
    memset(search, 0, sizeof(*search));
}

static void initialize_pair(Search *search, int first, int second) {
    const Problem *problem = search->problem;
    search->chosen[0] = first;
    search->chosen[1] = second;

    uint64_t *attacked = search->attacked_stack +
                         (size_t)2 * (size_t)problem->words;
    const uint64_t *first_mask = const_attack_mask(problem, first);
    const uint64_t *second_mask = const_attack_mask(problem, second);
    for (int word = 0; word < problem->words; ++word) {
        attacked[word] = first_mask[word] | second_mask[word];
    }
    search->attacked_count[2] = bitset_count(attacked, problem->words);
    search->next_square[2] = second + 1;
    search->nodes = 1;
    record_bounds(problem, search->best_attacked, 2,
                  search->attacked_count[2]);
}

static void search_pair(Search *search) {
    const Problem *problem = search->problem;
    const int first = search->chosen[0];
    int depth = 2;

    while (depth >= 2 && !search->overflow) {
        if (depth == problem->maximum_searched_k) {
            --depth;
            continue;
        }

        const int start = search->next_square[depth];
        const int available = problem->size - start;
        const int wanted = problem->maximum_searched_k - depth;
        const int maximum_descendant =
            available < wanted ? depth + available
                               : problem->maximum_searched_k;
        if (maximum_descendant <= depth ||
            search->attacked_count[depth] >=
                search->best_attacked[maximum_descendant]) {
            --depth;
            continue;
        }

        const uint64_t *attacked = search->attacked_stack +
                                   (size_t)depth * (size_t)problem->words;
        bool descended = false;
        while (search->next_square[depth] < problem->size) {
            const int square = search->next_square[depth]++;
            if (problem->minimum_image[square] < first) continue;
            if (depth == 2 &&
                !canonical_triple(problem, first, search->chosen[1], square)) {
                continue;
            }

            uint64_t *next = search->attacked_stack +
                             (size_t)(depth + 1) * (size_t)problem->words;
            const uint64_t *mask = const_attack_mask(problem, square);
            int added = 0;
            for (int word = 0; word < problem->words; ++word) {
                const uint64_t new_bits = mask[word] & ~attacked[word];
                next[word] = attacked[word] | new_bits;
                added += __builtin_popcountll(new_bits);
            }
            const int next_depth = depth + 1;
            const int next_count = search->attacked_count[depth] + added;
            const int next_available = problem->size - (square + 1);
            const int next_wanted =
                problem->maximum_searched_k - next_depth;
            const int next_maximum =
                next_available < next_wanted ? next_depth + next_available
                                             : problem->maximum_searched_k;
            if (next_count >= search->best_attacked[next_maximum]) continue;

            search->chosen[depth] = square;
            search->attacked_count[next_depth] = next_count;
            search->next_square[next_depth] = square + 1;
            depth = next_depth;
            if (!checked_add_u64(&search->nodes, 1)) {
                search->overflow = true;
                break;
            }
            record_bounds(problem, search->best_attacked, depth, next_count);
            descended = true;
            break;
        }
        if (!descended) --depth;
    }
}

static bool build_pairs(const Problem *problem, Pair **pairs_out,
                        size_t *count_out) {
    size_t count = 0;
    for (int first = 0; first < problem->size - 1; ++first) {
        if (problem->minimum_image[first] != first) continue;
        for (int second = first + 1; second < problem->size; ++second) {
            if (problem->minimum_image[second] < first) continue;
            if (!canonical_pair(problem, first, second)) continue;
            if (count == SIZE_MAX) return false;
            ++count;
        }
    }

    size_t bytes;
    if (!checked_mul_size(count, sizeof(Pair), &bytes)) return false;
    Pair *pairs = count == 0 ? NULL : malloc(bytes);
    if (count != 0 && pairs == NULL) return false;

    size_t index = 0;
    for (int first = 0; first < problem->size - 1; ++first) {
        if (problem->minimum_image[first] != first) continue;
        for (int second = first + 1; second < problem->size; ++second) {
            if (problem->minimum_image[second] < first) continue;
            if (!canonical_pair(problem, first, second)) continue;
            pairs[index].first = first;
            pairs[index].second = second;
            ++index;
        }
    }
    if (index != count) {
        free(pairs);
        return false;
    }
    *pairs_out = pairs;
    *count_out = count;
    return true;
}

static void direct_search(const Problem *problem, int start, int queens,
                          uint64_t attacked, int *best_attacked) {
    if (queens > 0) {
        const int count = __builtin_popcountll(attacked);
        if (count < best_attacked[queens]) best_attacked[queens] = count;
    }
    if (queens == problem->maximum_searched_k) return;

    for (int square = start; square < problem->size; ++square) {
        const uint64_t mask = const_attack_mask(problem, square)[0];
        direct_search(problem, square + 1, queens + 1,
                      attacked | mask, best_attacked);
    }
}

static bool direct_verify(const Problem *problem, const int *best_attacked) {
    if (problem->n > DIRECT_VERIFY_MAX_N) return true;

    const size_t levels = (size_t)problem->maximum_searched_k + 1;
    size_t bytes;
    if (!checked_mul_size(levels, sizeof(int), &bytes)) return false;
    int *direct_best = malloc(bytes);
    if (direct_best == NULL) return false;
    for (int k = 0; k <= problem->maximum_searched_k; ++k) {
        direct_best[k] = problem->size;
    }
    if (problem->maximum_searched_k > 0) {
        direct_search(problem, 0, 0, UINT64_C(0), direct_best);
    }

    bool matches = true;
    for (int k = 1; k <= problem->maximum_searched_k; ++k) {
        if (direct_best[k] != best_attacked[k]) {
            matches = false;
            break;
        }
    }
    free(direct_best);
    return matches;
}

static bool solve(const Problem *problem, bool verify, int **best_out,
                  uint64_t *nodes_out, bool *direct_verified) {
    const size_t levels = (size_t)problem->maximum_searched_k + 1;
    size_t best_bytes;
    if (!checked_mul_size(levels, sizeof(int), &best_bytes)) return false;
    int *seed_best = malloc(best_bytes);
    int *global_best = malloc(best_bytes);
    if (seed_best == NULL || global_best == NULL) {
        free(seed_best);
        free(global_best);
        return false;
    }
    if (!seed_best_bounds(problem, seed_best)) {
        free(seed_best);
        free(global_best);
        return false;
    }
    memcpy(global_best, seed_best, best_bytes);

    uint64_t nodes = 1;
    for (int first = 0; first < problem->size; ++first) {
        if (problem->minimum_image[first] == first &&
            !checked_add_u64(&nodes, 1)) {
            free(seed_best);
            free(global_best);
            return false;
        }
    }

    if (problem->maximum_searched_k >= 2) {
        Pair *pairs = NULL;
        size_t pair_count = 0;
        if (!build_pairs(problem, &pairs, &pair_count) ||
            pair_count > (size_t)LLONG_MAX) {
            free(pairs);
            free(seed_best);
            free(global_best);
            return false;
        }

        bool worker_failed = false;
        const long long task_count = (long long)pair_count;
#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1)
#endif
        for (long long task = 0; task < task_count; ++task) {
            Search search;
            const Pair pair = pairs[(size_t)task];
            const bool allocated = search_allocate(&search, problem, seed_best);
            if (allocated) {
                initialize_pair(&search, pair.first, pair.second);
                search_pair(&search);
            }

#ifdef _OPENMP
#pragma omp critical(a274948_merge)
#endif
            {
                if (!allocated || search.overflow ||
                    (allocated &&
                     !best_is_nondecreasing(problem, search.best_attacked))) {
                    worker_failed = true;
                } else {
                    for (int k = 1; k <= problem->maximum_searched_k; ++k) {
                        if (search.best_attacked[k] < global_best[k]) {
                            global_best[k] = search.best_attacked[k];
                        }
                    }
                    if (!checked_add_u64(&nodes, search.nodes)) {
                        worker_failed = true;
                    }
                }
            }
            if (allocated) search_destroy(&search);
        }
        free(pairs);
        if (worker_failed) {
            free(seed_best);
            free(global_best);
            return false;
        }
    }

    free(seed_best);
    if (!best_is_nondecreasing(problem, global_best)) {
        free(global_best);
        return false;
    }

    *direct_verified = false;
    if (verify && problem->n <= DIRECT_VERIFY_MAX_N) {
        if (!direct_verify(problem, global_best)) {
            free(global_best);
            return false;
        }
        *direct_verified = true;
    }
    *best_out = global_best;
    *nodes_out = nodes;
    return true;
}

static int parse_limit(const char *text) {
    errno = 0;
    char *end = NULL;
    const long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value <= 0 ||
        value > 46340) {
        return -1;
    }
    return (int)value;
}

static void usage(const char *program) {
    fprintf(stderr, "Usage: %s [--verify] [positive_limit]\n", program);
}

int main(int argc, char **argv) {
    bool verify = false;
    int limit = DEFAULT_LIMIT;
    bool have_limit = false;

    for (int argument = 1; argument < argc; ++argument) {
        if (strcmp(argv[argument], "--verify") == 0) {
            verify = true;
        } else if (!have_limit) {
            limit = parse_limit(argv[argument]);
            if (limit < 0) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            have_limit = true;
        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    for (int n = 1; n <= limit; ++n) {
        Problem problem;
        if (!problem_init(&problem, n)) {
            fprintf(stderr, "n=%d: size overflow or memory allocation failed\n", n);
            return EXIT_FAILURE;
        }

        const int full_maximum_k = problem.maximum_searched_k;
        const size_t value_count = (size_t)full_maximum_k + 1;
        size_t value_bytes;
        if (!checked_mul_size(value_count, sizeof(int), &value_bytes)) {
            fprintf(stderr, "n=%d: result-size overflow\n", n);
            problem_destroy(&problem);
            return EXIT_FAILURE;
        }
        int *safe_values = calloc(value_count, sizeof(int));
        if (safe_values == NULL) {
            fprintf(stderr, "n=%d: result allocation failed\n", n);
            problem_destroy(&problem);
            return EXIT_FAILURE;
        }
        safe_values[0] = problem.size;

        uint64_t nodes = 0;
        int crossing = 0;
        if (full_maximum_k == 0) {
            nodes = 1;
        } else {
            for (int cutoff = 1; cutoff <= full_maximum_k; ++cutoff) {
                problem.maximum_searched_k = cutoff;
                int *partial_best = NULL;
                uint64_t partial_nodes = 0;
                bool unused_direct_verified = false;
                if (!solve(&problem, false, &partial_best, &partial_nodes,
                           &unused_direct_verified) ||
                    !checked_add_u64(&nodes, partial_nodes)) {
                    fprintf(stderr,
                            "n=%d: search, counter, or allocation failure\n", n);
                    free(partial_best);
                    free(safe_values);
                    problem_destroy(&problem);
                    return EXIT_FAILURE;
                }
                for (int k = 1; k <= cutoff; ++k) {
                    safe_values[k] = problem.size - partial_best[k];
                }
                free(partial_best);

                if (safe_values[cutoff] <= cutoff) {
                    crossing = cutoff;
                    break;
                }
            }
        }

        if (full_maximum_k > 0 && crossing == 0) {
            fprintf(stderr, "n=%d: failed to find dual crossing\n", n);
            free(safe_values);
            problem_destroy(&problem);
            return EXIT_FAILURE;
        }

        /*
         * Let f(k)=T(n,k).  Queen attacks are symmetric, so
         * f(k) >= j iff f(j) >= k.  Hence
         * f(k) = #{j >= 1 : f(j) >= k}.
         * Once f(crossing) <= crossing, no later j can contribute, and every
         * remaining entry is therefore determined by the known prefix.
         */
        for (int k = crossing + 1; k <= full_maximum_k; ++k) {
            int count = 0;
            for (int j = 1; j <= crossing; ++j) {
                if (safe_values[j] >= k) ++count;
            }
            safe_values[k] = count;
        }

        bool valid = true;
        for (int k = 2; k <= full_maximum_k; ++k) {
            if (safe_values[k - 1] < safe_values[k]) valid = false;
        }
        for (int k = 1; k <= full_maximum_k && valid; ++k) {
            int conjugate = 0;
            for (int j = 1; j <= full_maximum_k; ++j) {
                if (safe_values[j] >= k) ++conjugate;
            }
            if (safe_values[k] != conjugate) valid = false;
        }
        if (!valid) {
            fprintf(stderr, "n=%d: dual-result verification failed\n", n);
            free(safe_values);
            problem_destroy(&problem);
            return EXIT_FAILURE;
        }

        bool direct_verified = false;
        problem.maximum_searched_k = full_maximum_k;
        if (verify && n <= DIRECT_VERIFY_MAX_N) {
            int *expected_best = malloc(value_bytes);
            if (expected_best == NULL) {
                fprintf(stderr, "n=%d: verification allocation failed\n", n);
                free(safe_values);
                problem_destroy(&problem);
                return EXIT_FAILURE;
            }
            expected_best[0] = problem.size;
            for (int k = 1; k <= full_maximum_k; ++k) {
                expected_best[k] = problem.size - safe_values[k];
            }
            if (!direct_verify(&problem, expected_best)) {
                fprintf(stderr, "n=%d: direct verification failed\n", n);
                free(expected_best);
                free(safe_values);
                problem_destroy(&problem);
                return EXIT_FAILURE;
            }
            free(expected_best);
            direct_verified = true;
        }

        printf("n=%d k=0..%d:", n, problem.size);
        for (int k = 0; k <= problem.size; ++k) {
            int value = 0;
            if (k <= full_maximum_k) value = safe_values[k];
            printf("%s%d", k == 0 ? " " : ", ", value);
        }
        printf("  crossing=%d nodes=%" PRIu64, crossing, nodes);
        if (direct_verified) {
            printf(" direct_verified");
        } else if (verify && n > DIRECT_VERIFY_MAX_N) {
            printf(" direct_verify_skipped(n>%d)", DIRECT_VERIFY_MAX_N);
        }
        putchar('\n');

        free(safe_values);
        problem_destroy(&problem);
    }
    return EXIT_SUCCESS;
}
