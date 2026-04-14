# T(0,0) = 1
# T(n,k) = 0 if k < 0 or k > n
# T(n,k) = 2*(k+1)*(2*k+1) * T(n-1,k+1)
#        + (4*(4*k + 1)*n + 8*k^2 - 8*k - 3) * T(n-1,k)
#        + 2*(4*n + 2*k - 5)*(2*n + k - 1) * T(n-1,k-1)

def triangle_rows(limit)
  rows = []
  a = [1] # n = 0
  rows << a.dup

  (1..limit).each{|n|
    next_a = Array.new(n + 1, 0)
    (0..n).each{|k|
      alpha = (k + 1 <= a.size - 1) ? 2 * (k + 1) * (2 * k + 1) * a[k + 1] : 0
      beta = (k <= a.size - 1) ? (4 * (4 * k + 1) * n + 8 * k**2 - 8 * k - 3) * a[k] : 0
      gamma = (k > 0) ? 2 * (4 * n + 2 * k - 5) * (2 * n + k - 1) * a[k - 1] : 0
      next_a[k] = alpha + beta + gamma
    }
    a = next_a
    rows << a.dup
  }
  rows
end

n = 250
rows = triangle_rows(n)
ary = (0..n).map{|i| rows[i][0]}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print " "
  puts j
}
