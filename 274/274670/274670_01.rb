def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..n} binomial(n,k) * binomial(2*n-k,n) * Sum_{j=0..k} binomial(k,j) * binomial(2*n-j,n).
def a(n)
  (0..n).inject(0){|s, k|
    s + ncr(n, k) * ncr(2*n-k, n) * (0..k).inject(0){|t, j|
      t + ncr(k, j) * ncr(2*n-j, n)
    }
  }
end

n = 214
(0..n).each{|i|
  print i
  print ' '
  puts a(i)
}