def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..floor(n/2)} binomial(3*n-6*k+1,k) * binomial(3*n-6*k+1,n-2*k)/(3*n-6*k+1).
def A(n)
  (0..n / 2).inject(0){|s, i| s + ncr(3 * n - 6 * i + 1, i) * ncr(3 * n - 6 * i + 1, n - 2 * i) / (3 * n - 6 * i + 1r)}.to_i
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}