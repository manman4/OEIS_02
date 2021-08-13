n = 1200
(0..n).each{|i|
  j = 40320 * 9 ** i - 141120 * 8 ** i + 191520 * 7 ** i - 126000 * 6 ** i + 40824 * 5 ** i - 5796 * 4 ** i + 254 * 3 ** i - 2 ** i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
