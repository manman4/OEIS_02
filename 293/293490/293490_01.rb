def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..n} binomial(2*k, k)*binomial(2*n-k, n).
def A293490(n)
  (0..n).inject(0){|s, k| s + ncr(2 * k, k) * ncr(2 * n - k, n)}
end

n = 1000
(0..n).each{|i|
  j = A293490(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}