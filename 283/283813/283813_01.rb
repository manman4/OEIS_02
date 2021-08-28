n = 1200
(0..n).each{|i|
  j = 5040 * 8 ** i - 15120 * 7 ** i + 16800 * 6 ** i - 8400 * 5 ** i + 1806 * 4 ** i - 126 * 3 ** i + 2 ** i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
