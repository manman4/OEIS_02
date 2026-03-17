# Fast DP for: Number of partitions of 2n such that
# (sum of parts having multiplicity 1) = sum of all other parts.
# Compute a(0..N) in one DP pass (no need to recompute for each n).

def all_a(n_max)
  size = n_max + 1
  dp = Array.new(size){Array.new(size, 0)}
  dp[0][0] = 1
  new_dp = Array.new(size){Array.new(size, 0)}
  nz = Array.new(size){[]}
  nz[0] = [0]
  nz_rows = [0]
  new_nz = Array.new(size){[]}
  new_rows = []
  contrib_a = Array.new(size)
  contrib_b = Array.new(size)

  # For valid partitions with s1 = s2 = n, all parts are <= n.
  p = 1
  while p <= n_max
    if contrib_a[p].nil?
      ca = [0] # not using part p
      cb = [0]
      ca << p   # use part p exactly once
      cb << 0
      m = 2
      while m * p <= n_max
        ca << 0
        cb << m * p # use part p with multiplicity m (>=2)
        m += 1
      end
      contrib_a[p] = ca
      contrib_b[p] = cb
    end
    ca = contrib_a[p]
    cb = contrib_b[p]

    # s1 is bounded by using each part 1..p once; s2 can still reach n_max via repeated 1s.
    max_s1 = p * (p + 1) / 2
    max_s1 = n_max if max_s1 > n_max
    # clear only cells touched in previous iteration
    r = 0
    while r < new_rows.length
      row_idx = new_rows[r]
      cols = new_nz[row_idx]
      c = 0
      while c < cols.length
        new_dp[row_idx][cols[c]] = 0
        c += 1
      end
      cols.clear
      r += 1
    end
    new_rows.clear

    r = 0
    while r < nz_rows.length
      s1 = nz_rows[r]
      if s1 <= max_s1
        row = dp[s1]
        cols = nz[s1]
        c = 0
        while c < cols.length
          s2 = cols[c]
          count = row[s2]
          i = 0
          len = ca.length
          # index 0 and 1 are always within bounds
          ns1 = s1
          ns2 = s2
          if new_dp[ns1][ns2] == 0
            new_nz[ns1] << ns2
            new_rows << ns1 if new_nz[ns1].length == 1
          end
          new_dp[ns1][ns2] += count
          ns1 = s1 + p
          if ns1 < size
            if new_dp[ns1][ns2] == 0
              new_nz[ns1] << ns2
              new_rows << ns1 if new_nz[ns1].length == 1
            end
            new_dp[ns1][ns2] += count
          end
          i = 2
          while i < len
            ns2 = s2 + cb[i]
            if ns2 < size
              if new_dp[s1][ns2] == 0
                new_nz[s1] << ns2
                new_rows << s1 if new_nz[s1].length == 1
              end
              new_dp[s1][ns2] += count
            end
            i += 1
          end
          c += 1
        end
      end
      r += 1
    end

    dp, new_dp = new_dp, dp
    nz, new_nz = new_nz, nz
    nz_rows, new_rows = new_rows, nz_rows
    p += 1
  end

  (0..size - 1).map{|n| dp[n][n]}
end

n = 200
ary = all_a(n)
(0..n).each{|i| 
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
