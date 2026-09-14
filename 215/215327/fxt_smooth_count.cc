/*
 * This program is an adaptation of Joerg Arndt's FXT example
 * fxt/demo/comb/lyndon-words-prefix-cond-demo.cc and uses FXT's
 * lyndon_words_prefix_cond class.  The adaptation adds an N/Q-oriented
 * command-line interface, S/L/N/C counting, and incremental b-file output.
 *
 * FXT: https://www.jjj.de/fxt/
 * Original FXT code: Copyright (C) Joerg Arndt
 * License: GNU General Public License version 3 or later; see fxt/COPYING.txt.
 */

#include "comb/lyndon-words-prefix-cond.h"

#include "fxttypes.h"

#include <cerrno>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

namespace {

ulong word_length;

struct Counts {
    std::uint64_t s = 0;
    std::uint64_t l = 0;
    std::uint64_t n = 0;
    std::uint64_t c = 0;
};

bool
smooth_prefix(const ulong *word, ulong position)
{
    if (position <= 1) {
        return true;
    }
    const ulong left = word[position - 1];
    const ulong right = word[position];
    return left >= right ? left - right <= 1 : right - left <= 1;
}

bool
cyclically_smooth_prefix(const ulong *word, ulong position)
{
    if (!smooth_prefix(word, position)) {
        return false;
    }
    if (position != word_length) {
        return true;
    }
    const ulong first = word[1];
    const ulong last = word[word_length];
    return first >= last ? first - last <= 1 : last - first <= 1;
}

ulong
parse_positive_ulong(const char *text, const char *name)
{
    char *end = nullptr;

    if (text[0] == '\0' || text[0] == '-') {
        std::cerr << name << " must be a positive integer\n";
        std::exit(EXIT_FAILURE);
    }
    errno = 0;
    const unsigned long value = std::strtoul(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' || value == 0) {
        std::cerr << name << " must be a positive integer in the supported range\n";
        std::exit(EXIT_FAILURE);
    }
    return value;
}

bool
increment(std::uint64_t &value)
{
    if (value == std::numeric_limits<std::uint64_t>::max()) {
        std::cerr << "count exceeds the 64-bit counter range\n";
        return false;
    }
    ++value;
    return true;
}

bool
count_pair(ulong length, ulong colors, bool cyclic,
           std::uint64_t &necklace_count,
           std::uint64_t &lyndon_count)
{
    word_length = length;
    lyndon_words_prefix_cond words(length, 0, colors - 1);
    ulong position = words.first(cyclic ? cyclically_smooth_prefix
                                        : smooth_prefix);

    while (position != 0) {
        if (words.is_necklace()) {
            if (!increment(necklace_count)) {
                return false;
            }
            if (words.is_lyndon_word() && !increment(lyndon_count)) {
                return false;
            }
        }
        position = words.next_necklace();
    }
    return true;
}

bool
count_all(ulong length, ulong colors, Counts &counts)
{
    return count_pair(length, colors, false, counts.s, counts.l) &&
           count_pair(length, colors, true, counts.n, counts.c);
}

std::string
bfile_name(char kind, ulong colors)
{
    return "b-" + std::string(1, kind) + "_q" +
           std::to_string(colors) + ".txt";
}

bool
write_bfiles(ulong max_length, ulong colors)
{
    const std::string s_name = bfile_name('S', colors);
    const std::string l_name = bfile_name('L', colors);
    const std::string n_name = bfile_name('N', colors);
    const std::string c_name = bfile_name('C', colors);
    std::ofstream s_file(s_name);
    std::ofstream l_file(l_name);
    std::ofstream n_file(n_name);
    std::ofstream c_file(c_name);

    if (!s_file || !l_file || !n_file || !c_file) {
        std::cerr << "could not create all four b-files\n";
        return false;
    }

    s_file << "0 1\n";
    l_file << "0 1\n";
    s_file.flush();
    l_file.flush();

    for (ulong length = 1;; ++length) {
        Counts counts;
        if (!count_all(length, colors, counts)) {
            return false;
        }

        s_file << length << ' ' << counts.s << '\n';
        l_file << length << ' ' << counts.l << '\n';
        n_file << length << ' ' << counts.n << '\n';
        c_file << length << ' ' << counts.c << '\n';
        s_file.flush();
        l_file.flush();
        n_file.flush();
        c_file.flush();
        if (!s_file || !l_file || !n_file || !c_file) {
            std::cerr << "error while writing b-files\n";
            return false;
        }

        std::cout << length << " S=" << counts.s << " L=" << counts.l
                  << " N=" << counts.n << " C=" << counts.c << '\n'
                  << std::flush;
        if (length == max_length) {
            break;
        }
    }

    std::cout << "saved " << s_name << ", " << l_name << ", "
              << n_name << ", " << c_name << '\n';
    return true;
}

void
usage(const char *program)
{
    std::cerr
        << "Usage: " << program << " N Q [S|L|N|C]\n"
        << "       " << program << " --bfiles MAX_N Q\n"
        << "  N: word/necklace length\n"
        << "  Q: number of colors (the alphabet is 0,...,Q-1)\n"
        << "  S: smooth necklaces (default)\n"
        << "  L: smooth Lyndon words\n"
        << "  N: cyclically smooth necklaces\n"
        << "  C: cyclically smooth Lyndon words\n"
        << "  --bfiles: save all four sequences through MAX_N\n"
        << "Adapted from Joerg Arndt's FXT prefix-condition demo.\n";
}

}  // namespace

int
main(int argc, char **argv)
{
    if (argc == 4 && std::string(argv[1]) == "--bfiles") {
        const ulong max_length = parse_positive_ulong(argv[2], "MAX_N");
        const ulong colors = parse_positive_ulong(argv[3], "Q");
        return write_bfiles(max_length, colors) ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    if (argc < 3 || argc > 4) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    word_length = parse_positive_ulong(argv[1], "N");
    const ulong colors = parse_positive_ulong(argv[2], "Q");
    char kind = 'S';
    if (argc == 4) {
        const std::string argument(argv[3]);
        if (argument.size() != 1) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        kind = argument[0];
        if (kind >= 'a' && kind <= 'z') {
            kind = static_cast<char>(kind - 'a' + 'A');
        }
    }
    if (kind != 'S' && kind != 'L' && kind != 'N' && kind != 'C') {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    const bool necklace = kind == 'S' || kind == 'N';
    const bool cyclic = kind == 'N' || kind == 'C';
    lyndon_words_prefix_cond words(word_length, 0, colors - 1);
    ulong position = words.first(cyclic ? cyclically_smooth_prefix
                                        : smooth_prefix);
    std::uint64_t count = 0;

    while (position != 0) {
        const bool selected = necklace ? words.is_necklace()
                                       : words.is_lyndon_word();
        if (selected) {
            if (count == std::numeric_limits<std::uint64_t>::max()) {
                std::cerr << "count exceeds the 64-bit counter range\n";
                return EXIT_FAILURE;
            }
            ++count;
        }
        position = necklace ? words.next_necklace()
                            : words.next_lyndon_word();
    }

    std::cout << kind << '_' << colors << '(' << word_length << ") = "
              << count << '\n';
    return EXIT_SUCCESS;
}
