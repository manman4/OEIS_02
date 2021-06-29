def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  (0..n).inject(0){|s, i| s + (-1) ** (i % 2) * ncr(2 * i, i) ** 2 * ncr(n + i, n - i)}
end

n = 500
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
