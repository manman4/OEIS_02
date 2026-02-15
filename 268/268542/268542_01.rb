def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..n} binomial(n,k) * binomial(n+k,k) * binomial(2*k,n).
def a(n)
  (0..n).inject(0){|s, k| s + ncr(n, k) * ncr(n+k, k) * ncr(2*k, n)}
end

n = 310
(0..n).each{|i|
  print i
  print ' '
  puts a(i)
}