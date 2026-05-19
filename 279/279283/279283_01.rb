def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=1..n} binomial(k+2,3) * binomial(n+3*k-1,4*k-1).
def a(n)
  (1..n).inject(0){|s, i| s + ncr(i + 2, 3) * ncr(n + 3 * i - 1, 4 * i - 1)}
end

n = 1000
(0..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}