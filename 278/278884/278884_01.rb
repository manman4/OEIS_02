def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

n = 1000
(0..n).each{|i|
  j = ncr(3 * i - 1, i) * ncr(3 * i, i) / (2 * i + 1)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
