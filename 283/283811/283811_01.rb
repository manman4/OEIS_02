n = 1300
(0..n).each{|i|
  j = 120 * 6 ** i - 240 * 5 ** i + 150 * 4 ** i - 30 * 3 ** i + 2 ** i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
