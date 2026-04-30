def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{i=0..3*n-2} binomial(3*n-4,i) * Sum_{j=0..floor((3*n-i-2)/4)} (-1)^j * binomial(n,j) * binomial(4*n-i-4-4*j,n-2). 
def a(n)
  (0..3 * n - 2).inject(0){|s, i| s + ncr(3 * n - 4, i) * (0..(3 * n - i - 2) / 4).inject(0){|t, j| t + (-1) ** j * ncr(n, j) * ncr(4 * n - i - 4 - 4 * j, n - 2)}}
end

n = 101
(2..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
