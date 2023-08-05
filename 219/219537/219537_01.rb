def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, m, n)
  return 1 if n == 0
  (0..n - 1).inject(0){|s, i| s + ncr(n, i) * ncr(k * n + m * i, n - 1 - i)} / n
end

n = 1000
(0..n).each{|i|
  j = A(2, 1, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}