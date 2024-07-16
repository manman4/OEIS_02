def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = (1/(n+1)) * Sum_{k=0..n} (k/(n+k)) * binomial(n+k,k)^2
def A(n)
  (0..n).inject(0){|s, i| s + i.to_r / (n + i) * ncr(n + i, i) ** 2} / (n + 1)
end

n = 1000
(1..n).each{|i|
  j = A(i).to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
