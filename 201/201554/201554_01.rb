n = 10000
(0..n).each{|i|
  j = 1 + 7 * i * (i + 1) * (841 * i ** 4 + 1682 * i ** 3 + 1568 * i * i + 727 * i + 222) / 180
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
