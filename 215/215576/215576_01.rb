def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{i=0..n/2} C(2*n-4*i+1,i)*C(2*n-4*i+1,n-2*i)/(2*n-4*i+1).
def A(n)
  (0..n / 2).inject(0){|s, i| s + ncr(2 * n - 4 * i + 1, i) * ncr(2 * n - 4 * i + 1, n - 2 * i) / (2 * n - 4 * i + 1).to_r}.to_i
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
