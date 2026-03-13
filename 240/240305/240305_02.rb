def count_exact_d_with_max_mult_leq_up_to(n, d, m)
  # dp[j][s]: using processed sizes, select exactly j distinct sizes,
  # sum to s, where each selected size has multiplicity in 1..m.
  dp = Array.new(d + 1) { Array.new(n + 1, 0) }
  dp[0][0] = 1

  # Upper bound for the largest possible distinct part size when choosing d parts.
  max_size = n - (d - 1) * d / 2
  max_size = n if max_size > n

  size = 1
  while size <= max_size
    dp2 = dp.map(&:dup)

    j = 0
    while j < d
      # Restrict s-range for this j to reduce work.
      min_s = (j + 1) * (j + 2) / 2
      max_s = n - (d - (j + 1)) * (d - (j + 1) + 1) / 2
      min_s = 0 if min_s < 0
      max_s = n if max_s > n
      if min_s <= max_s
        # temp[s] = sum_{r=1..m} dp[j][s - r*size], computed by sliding window
        mod = 0
        while mod < size
          window_sum = 0
          s = mod
          while s <= n
            s1 = s - size
            if s1 >= 0
              window_sum += dp[j][s1]
              s2 = s - (m + 1) * size
              if s2 >= 0
                window_sum -= dp[j][s2]
              end
            end
            if s >= min_s && s <= max_s && window_sum != 0
              dp2[j + 1][s] += window_sum
            end
            s += size
          end
          mod += 1
        end
      end
      j += 1
    end

    dp = dp2
    size += 1
  end

  dp[d]
end

# Number of partitions p of n such that (maximal multiplicity of the parts of p) < (number of distinct parts of p).
def A240305(n)
  counts = Array.new(n + 1, 0)
  counts[0] = 1

  d = 1
  while d * (d + 1) / 2 <= n
    le_d_1 = count_exact_d_with_max_mult_leq_up_to(n, d, d - 1)

    s = 0
    while s <= n
      counts[s] += le_d_1[s]
      s += 1
    end
    d += 1
  end

  counts
end

n = 1000
ary = A240305(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
