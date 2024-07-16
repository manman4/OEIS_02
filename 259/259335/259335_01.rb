def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = ( Sum_{k=0..n} binomial(2*n, k)^2 * (binomial(2*n, k+1) - binomial(2*n, k-1)) )/(n*binomial(2*n, n)).
def A(n)
  (1..n).inject(2 * n){|s, i| s + ncr(2 * n, i) ** 2 * (ncr(2 * n, i + 1) - ncr(2 * n, i - 1))} / (n * ncr(2 * n, n))
end

n = 1000
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
