def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

k = 4
n = 1000
(0..n).each{|i|
  j = ncr(2 * k * i + k + 2, k * i + 1) / (2 * k * i + k + 2)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
