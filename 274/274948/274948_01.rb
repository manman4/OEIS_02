#!/usr/bin/env ruby
# frozen_string_literal: true

# A274948: T(n, k) is the maximum number of unattacked vacant squares when k
# queens are placed on an n X n board.  This program prints complete rows
# n = 1..limit for 0 <= k <= n^2.  It avoids searching the trailing zero terms.

DEFAULT_LIMIT = 5

def popcount(value)
  count = 0
  until value.zero?
    word = value & 0xffff_ffff
    word -= (word >> 1) & 0x5555_5555
    word = (word & 0x3333_3333) + ((word >> 2) & 0x3333_3333)
    word = (word + (word >> 4)) & 0x0f0f_0f0f
    count += ((word * 0x0101_0101) & 0xffff_ffff) >> 24
    value >>= 32
  end
  count
end

def attack_masks(n)
  size = n * n
  Array.new(size) do |queen|
    queen_row, queen_col = queen.divmod(n)
    mask = 0

    size.times do |square|
      row, col = square.divmod(n)
      if row == queen_row || col == queen_col ||
         (row - queen_row).abs == (col - queen_col).abs
        mask |= 1 << square
      end
    end
    mask
  end
end

class SafeSquareSearch
  attr_reader :nodes

  def initialize(n)
    @n = n
    @size = n * n
    @masks = attack_masks(n)
    # With this many queens every placement has zero safe vacant squares.
    @zero_threshold = (n - 1) * (n - 2) + 1
    @maximum_searched_k = @zero_threshold - 1
    @best_attacked = Array.new(@zero_threshold, @size + 1)
    @transforms = build_transforms
    @minimum_image = Array.new(@size) do |square|
      @transforms.map { |transform| transform[square] }.min
    end
    @canonical_pair = build_canonical_pairs
    @chosen = Array.new(@maximum_searched_k)
    @nodes = 1
  end

  def solve
    search(0, 0, 0, 0) if @maximum_searched_k.positive?

    values = Array.new(@zero_threshold + 1, 0)
    1.upto(@maximum_searched_k) do |k|
      raise "no placement examined for k=#{k}" if @best_attacked[k] > @size

      values[k] = @size - @best_attacked[k]
    end
    unless @best_attacked.drop(1).each_cons(2).all? { |left, right| left <= right }
      raise "internal monotonicity check failed"
    end
    values[@zero_threshold] = 0
    values
  end

  private

  def build_transforms
    Array.new(8) { Array.new(@size) }.tap do |maps|
      @size.times do |square|
        row, col = square.divmod(@n)
        coordinates = [
          [row, col], [col, @n - 1 - row],
          [@n - 1 - row, @n - 1 - col], [@n - 1 - col, row],
          [row, @n - 1 - col], [@n - 1 - col, @n - 1 - row],
          [@n - 1 - row, col], [col, row]
        ]
        coordinates.each_with_index do |(rr, cc), symmetry|
          maps[symmetry][square] = rr * @n + cc
        end
      end
    end
  end

  def build_canonical_pairs
    pairs = Array.new(@size * @size, false)
    @size.times do |first|
      (first + 1...@size).each do |second|
        pairs[first * @size + second] = @transforms.all? do |transform|
          a = transform[first]
          b = transform[second]
          a, b = b, a if a > b
          first < a || (first == a && second <= b)
        end
      end
    end
    pairs
  end

  def canonical_triple?(first, second, third)
    @transforms.all? do |transform|
      a = transform[first]
      b = transform[second]
      c = transform[third]
      a, b = b, a if a > b
      b, c = c, b if b > c
      a, b = b, a if a > b

      first < a ||
        (first == a && (second < b || (second == b && third <= c)))
    end
  end

  def record_bounds(queens, attacked_count)
    if attacked_count < @best_attacked[queens]
      @best_attacked[queens] = attacked_count
    end

    # If Q leaves s safe squares, placing queens on any of those s squares
    # leaves every square of Q safe (queen attacks are symmetric).  This gives
    # a witnessed bound for all smaller queen counts as well.
    safe = @size - attacked_count
    dual_k = [safe, @maximum_searched_k].min
    dual_attacked = @size - queens
    while dual_k.positive? && @best_attacked[dual_k] > dual_attacked
      @best_attacked[dual_k] = dual_attacked
      dual_k -= 1
    end
  end

  def search(start, queens, attacked, attacked_count)
    record_bounds(queens, attacked_count) if queens.positive?
    return if queens == @maximum_searched_k

    maximum_descendant = [@maximum_searched_k, queens + @size - start].min
    return if maximum_descendant <= queens

    # Adding queens cannot remove attacked squares.  If the current attacked
    # count cannot improve any still-reachable k, the whole branch is hopeless.
    # @best_attacked is nondecreasing, so only the last reachable k is needed.
    return if attacked_count >= @best_attacked[maximum_descendant]

    first = queens.zero? ? nil : @chosen[0]
    pair_base = queens == 1 ? first * @size : nil
    not_attacked = ~attacked
    start.upto(@size - 1) do |square|
      # Every D4 orbit has a lexicographically least representative.  Prefixes
      # smaller under a rotation/reflection cannot become least by appending
      # larger square indices, so rejecting them preserves every optimum.
      next if @minimum_image[square] < (first || square)
      next if pair_base && !@canonical_pair[pair_base + square]
      if queens == 2 && !canonical_triple?(@chosen[0], @chosen[1], square)
        next
      end

      new_bits = @masks[square] & not_attacked
      next_attacked = attacked | new_bits
      next_count = attacked_count + popcount(new_bits)
      @chosen[queens] = square
      @nodes += 1
      search(square + 1, queens + 1, next_attacked, next_count)
    end
  end
end

# Independent exhaustive verifier.  It deliberately uses no branch-and-bound
# test; use --verify when changing the optimized search.
class DirectSafeSquareSearch
  def initialize(n)
    @size = n * n
    @masks = attack_masks(n)
    @zero_threshold = (n - 1) * (n - 2) + 1
    @maximum_searched_k = @zero_threshold - 1
    @best_attacked = Array.new(@zero_threshold, @size + 1)
  end

  def solve
    search(0, 0, 0) if @maximum_searched_k.positive?

    values = Array.new(@zero_threshold + 1, 0)
    1.upto(@maximum_searched_k) do |k|
      values[k] = @size - @best_attacked[k]
    end
    values
  end

  private

  def search(start, queens, attacked)
    if queens.positive?
      attacked_count = popcount(attacked)
      if attacked_count < @best_attacked[queens]
        @best_attacked[queens] = attacked_count
      end
    end
    return if queens == @maximum_searched_k

    start.upto(@size - 1) do |square|
      search(square + 1, queens + 1, attacked | @masks[square])
    end
  end
end

verify = !ARGV.delete("--verify").nil?
limit = Integer(ARGV.fetch(0, DEFAULT_LIMIT.to_s), 10)
raise ArgumentError, "limit must be positive" unless limit.positive?

known_a001366 = [nil, 0, 0, 0, 1, 3].freeze

1.upto(limit) do |n|
  search = SafeSquareSearch.new(n)
  values = search.solve
  zero_threshold = values.length - 1

  if verify
    direct_values = DirectSafeSquareSearch.new(n).solve
    raise "direct verification failed for n=#{n}" unless values == direct_values
  end

  full_values = Array.new(n * n + 1, 0)
  full_values[0] = n * n
  1.upto(zero_threshold) { |k| full_values[k] = values[k] }

  unless full_values.each_cons(2).all? { |left, right| left >= right }
    raise "monotonicity check failed for n=#{n}"
  end
  if n < known_a001366.length && full_values[n] != known_a001366[n]
    raise "A001366 check failed for n=#{n}"
  end

  row = full_values.join(", ")
  verification = verify ? " direct_verified" : ""
  puts "n=#{n} k=0..#{n * n}: #{row}  nodes=#{search.nodes}#{verification}"
end
