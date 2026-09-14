#!/usr/bin/env ruby
# frozen_string_literal: true

require "optparse"

# Count smooth necklaces, smooth Lyndon words, and cyclically smooth Lyndon
# words over the ordered alphabet {0, 1, ..., q - 1}.
#
# Usage:
#   ruby 215327_01.rb [OPTIONS] Q [MAX_N]
#
# The default MAX_N is 12.  Enumeration is exact but exponential, so large
# values of Q or MAX_N can take a long time.

DEFAULT_MAX_N = 12

def parse_positive_integer(text, name)
  value = Integer(text, 10)
  abort "#{name} must be a positive integer" unless value.positive?

  value
rescue ArgumentError
  abort "#{name} must be a positive integer"
end

def divisors(n)
  small = []
  large = []
  d = 1
  while d * d <= n
    if (n % d).zero?
      small << d
      large << n / d unless d * d == n
    end
    d += 1
  end
  small + large.reverse
end

def mobius(n)
  value = n
  prime_factors = 0
  prime = 2

  while prime * prime <= value
    if (value % prime).zero?
      value /= prime
      return 0 if (value % prime).zero?

      prime_factors += 1
      value /= prime while (value % prime).zero?
    end
    prime += prime == 2 ? 1 : 2
  end

  prime_factors += 1 if value > 1
  prime_factors.even? ? 1 : -1
end

def matrix_product(left, right)
  size = left.length
  product = Array.new(size) { Array.new(size, 0) }

  size.times do |row|
    size.times do |middle|
      coefficient = left[row][middle]
      next if coefficient.zero?

      size.times do |column|
        product[row][column] += coefficient * right[middle][column]
      end
    end
  end

  product
end

def adjacency_matrix(q)
  Array.new(q) do |row|
    Array.new(q) { |column| (row - column).abs <= 1 ? 1 : 0 }
  end
end

def trace_powers(q, max_n)
  matrix = adjacency_matrix(q)
  power = Array.new(q) { |row| Array.new(q) { |column| row == column ? 1 : 0 } }
  traces = Array.new(max_n + 1, 0)

  1.upto(max_n) do |n|
    power = matrix_product(power, matrix)
    traces[n] = q.times.sum { |index| power[index][index] }
  end

  traces
end

def cyclic_lyndon_count(n, traces)
  numerator = divisors(n).sum do |e|
    mobius(e) * traces[n / e]
  end
  abort "internal error: C numerator is not divisible by #{n}" unless (numerator % n).zero?

  numerator / n
end

# The FKM recursion generates exactly the lexicographically least
# representative of every q-ary necklace of length n. The period parameter is
# the least period at an emitted representative. Rejecting a newly appended
# letter whose difference from its predecessor exceeds 1 restricts the output
# to smooth representatives without changing the canonical-representative
# test performed by the recursion.
def count_smooth_objects(q, n)
  word = Array.new(n + 1, 0)
  smooth_necklaces = 0
  smooth_lyndon = 0
  cyclic_smooth_lyndon = 0

  generate = lambda do |position, period|
    if position > n
      return unless (n % period).zero?

      smooth_necklaces += 1
      if period == n
        smooth_lyndon += 1
        cyclic_smooth_lyndon += 1 if (word[n] - word[1]).abs <= 1
      end
      return
    end

    copied_letter = word[position - period]
    word[position] = copied_letter
    if position == 1 || (word[position] - word[position - 1]).abs <= 1
      generate.call(position + 1, period)
    end

    (copied_letter + 1).upto(q - 1) do |letter|
      word[position] = letter
      next if position > 1 && (word[position] - word[position - 1]).abs > 1

      generate.call(position + 1, position)
    end
  end

  generate.call(1, 1)
  [smooth_necklaces, smooth_lyndon, cyclic_smooth_lyndon]
end

options = { check: false, sequence: "S" }
parser = OptionParser.new do |opts|
  opts.banner = "Usage: ruby #{File.basename($PROGRAM_NAME)} [OPTIONS] Q [MAX_N]"
  opts.separator ""
  opts.separator "Default output is a two-column b-file-style table for S_q(n)."
  opts.separator "S_q and L_q start with the OEIS convention 0 1; C_q starts at n=1."
  opts.separator "MAX_N defaults to #{DEFAULT_MAX_N}. Enumeration is exact but exponential."
  opts.separator ""
  opts.on("-s", "--sequence NAME", "output S, L, or C (default: S)") do |name|
    options[:sequence] = name.upcase
  end
  opts.on("--check", "show S, L, C, the identity RHS, and check status") do
    options[:check] = true
  end
  opts.on("-h", "--help", "show this help") do
    puts opts
    exit
  end
end

begin
  parser.parse!(ARGV)
rescue OptionParser::ParseError => e
  warn e.message
  warn parser
  exit 1
end

unless (1..2).cover?(ARGV.length)
  warn parser
  exit 1
end
unless %w[S L C].include?(options[:sequence])
  abort "--sequence must be S, L, or C"
end

q = parse_positive_integer(ARGV[0], "Q")
max_n = parse_positive_integer(ARGV[1] || DEFAULT_MAX_N.to_s, "MAX_N")

traces = trace_powers(q, max_n)
counts = Array.new(max_n + 1)

1.upto(max_n) do |n|
  counts[n] = count_smooth_objects(q, n)
end

checked_rows = []

1.upto(max_n) do |n|
  smooth_necklaces, smooth_lyndon, cyclic_smooth_lyndon = counts[n]
  proper_divisor_sum = divisors(n).reject { |d| d == n }.sum do |d|
    counts[d][2]
  end
  right_hand_side = smooth_lyndon + proper_divisor_sum
  formula_c = cyclic_lyndon_count(n, traces)

  unless cyclic_smooth_lyndon == formula_c
    abort "C check failed at n=#{n}: enumeration=#{cyclic_smooth_lyndon}, formula=#{formula_c}"
  end
  unless smooth_necklaces == right_hand_side
    abort "identity failed at n=#{n}: S=#{smooth_necklaces}, RHS=#{right_hand_side}"
  end

  checked_rows << [n, smooth_necklaces, smooth_lyndon,
                   cyclic_smooth_lyndon, right_hand_side, "OK"]
end

if options[:check]
  puts "# q=#{q}"
  puts "# n S_q(n) L_q(n) C_q(n) RHS check"
  checked_rows.each { |row| puts row.join(" ") }
  warn "Verified C_q(n) independently and S_q(n) = L_q(n) + " \
       "sum_{d|n, d!=n} C_q(d) for q=#{q}, 1<=n<=#{max_n}."
else
  column = { "S" => 1, "L" => 2, "C" => 3 }.fetch(options[:sequence])
  puts "0 1" unless options[:sequence] == "C"
  checked_rows.each { |row| puts "#{row[0]} #{row[column]}" }
end
