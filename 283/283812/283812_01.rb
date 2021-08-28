n = 1300
(0..n).each{|i|
  j = 720 * 7 ** i - 1800 * 6 ** i + 1560 * 5 ** i - 540 * 4 ** i + 62 * 3 ** i - 2 ** i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
