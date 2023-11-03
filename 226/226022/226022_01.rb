def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=0..floor(n/3)}(-1)^k*binomial(n-3*k+1,k)*binomial(2*(n-3*k),n-3*k)/(n-3*k+1).
def A(n)
  (0..n / 3).inject(0){|s, i| s + (-1) ** i * ncr(n - 3 * i + 1, i) * ncr(2 * (n - 3 * i), n - 3 * i) / (n - 3 * i + 1)}
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}