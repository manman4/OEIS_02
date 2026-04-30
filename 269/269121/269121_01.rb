def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = 2^(3*n) - 3 * Sum_{k=0..n-1} binomial(3*n,k).
def a(n)
  return 1 if n == 0
  2 ** (3 * n) - 3 * (0..n - 1).inject(0){|s, k| s + ncr(3 * n, k)}
end

n = 1000
(0..n).each{|i|
  print i
  print ' '
  puts a(i)
}
