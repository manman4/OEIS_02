def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A288470(n)
  (0..n).inject(0){|s, i| s + ncr(n, i) * ncr(2 * n, 2 * i)}
end

n = 24
(0..n).each{|i|
  j = A288470(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
