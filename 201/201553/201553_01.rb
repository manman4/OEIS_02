n = 10000
(0..n).each{|i|
  j = (2 * i + 1) * (44 * i ** 4 + 88 * i ** 3 + 71 * i ** 2 + 27 * i + 5) / 5
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
