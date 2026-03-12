def count_exact_d_distinct_with_min_mult_up_to(n, d, min_mult)
  # dp[j][s]: using processed sizes, select exactly j distinct sizes,
  # sum to s, where each selected size has multiplicity >= min_mult.
  dp = Array.new(d + 1){Array.new(n + 1, 0)}
  dp[0][0] = 1

  (1..n).each{|size|
    dp2 = dp.map(&:dup)

    (0..d - 1).each{|j|
      temp = Array.new(n + 1, 0)

      base = min_mult * size
      (base..n).each{|s|
        temp[s] = dp[j][s - base]
      }
      (base + size..n).each{|s|
        temp[s] += temp[s - size]
      }

      (0..n).each{|s|
        dp2[j + 1][s] += temp[s]
      }
    }

    dp = dp2
  }

  dp[d]
end

def count_partitions_with_constraint_eq_up_to(n_max)
  counts = Array.new(n_max + 1, 0)
  counts[0] = 0

  k = 1
  loop do
    min_sum = k * k * (k + 1) / 2
    break if min_sum > n_max

    at_least_k = count_exact_d_distinct_with_min_mult_up_to(n_max, k, k)
    at_least_k1 = count_exact_d_distinct_with_min_mult_up_to(n_max, k, k + 1)

    (0..n_max).each{|s|
      counts[s] += at_least_k[s] - at_least_k1[s]
    }
    k += 1
  end

  counts
end

n = 60
counts = count_partitions_with_constraint_eq_up_to(n)
p (0..n).map{|s| counts[s]}
# counts.each_with_index{|val, n|
#   print n
#   print ' '
#   puts val
# }
