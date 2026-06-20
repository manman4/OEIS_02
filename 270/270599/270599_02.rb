require "etc"
require "tmpdir"

def reduce_fraction(num, den)
  g = num.gcd(den)
  [num / g, den / g]
end

def subtract_multiple(p, q, m, d)
  reduce_fraction(p * d - q * m, q * d)
end

class A270599Counter
  def initialize
    @cache = {}
  end

  def count(n)
    return 0 if n <= 0
    return 0 if n.even?
    return 1 if n == 1
    dfs(3, 1, 1, n)
  end

  private

  def dfs(min_d, p, q, sum_left)
    return 1 if p == 0 && sum_left == 0
    return 0 if p == 0 || sum_left <= 0 || min_d > sum_left

    # If p/q is representable by odd unit fractions whose denominators sum to sum_left,
    # then the parity of the number of remaining terms forces p == sum_left (mod 2).
    return 0 if ((p ^ sum_left) & 1) != 0

    # Easy upper/lower bounds for the remaining reciprocal sum.
    return 0 if p * min_d * min_d > q * sum_left
    return 0 if p * sum_left < q

    key = [min_d, p, q, sum_left]
    cached = @cache[key]
    return cached unless cached.nil?

    ans = 0
    next_d = min_d + 2
    max_m = [sum_left / min_d, (p * min_d) / q].min
    m = 0
    while m <= max_m
      rest_sum = sum_left - m * min_d
      if m == 0
        np = p
        nq = q
      else
        np, nq = subtract_multiple(p, q, m, min_d)
      end

      if np == 0
        ans += 1 if rest_sum == 0
      elsif next_d <= rest_sum &&
            ((np ^ rest_sum) & 1) == 0 &&
            np * next_d * next_d <= nq * rest_sum &&
            np * rest_sum >= nq
        ans += dfs(next_d, np, nq, rest_sum)
      end

      m += 1
    end

    @cache[key] = ans
  end
end

def count_a270599(n)
  A270599Counter.new.count(n)
end

def sequence_a270599(limit)
  counter = A270599Counter.new
  (0..limit).map { |n| counter.count(n) }
end

def parallel_sequence_a270599(limit, jobs = Etc.nprocessors)
  jobs = [[jobs.to_i, 1].max, limit + 1].min
  indices = (0..limit).select(&:odd?)
  slices = Array.new(jobs) { [] }
  indices.each_with_index { |n, i| slices[i % jobs] << n }

  paths = []
  pids = []
  slices.each do |slice|
    next if slice.empty?

    path = File.join(Dir.tmpdir, "a270599_#{Process.pid}_#{paths.length}.dump")
    paths << path
    pid = fork do
      counter = A270599Counter.new
      data = {}
      slice.each { |n| data[n] = counter.count(n) }
      File.binwrite(path, Marshal.dump(data))
      exit! 0
    end
    pids << pid
  end

  pids.each { |pid| Process.wait(pid) }

  ary = Array.new(limit + 1, 0)
  paths.each do |path|
    Marshal.load(File.binread(path)).each do |n, v|
      ary[n] = v
    end
    File.delete(path)
  end
  ary
end

if __FILE__ == $PROGRAM_NAME
  limit = (ARGV[0] || 100).to_i
  jobs = (ARGV[1] || 1).to_i
  ary = jobs > 1 ? parallel_sequence_a270599(limit, jobs) : sequence_a270599(limit)
  puts ary.join(", ")
end
