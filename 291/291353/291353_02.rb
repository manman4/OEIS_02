n = 100
(1..n).each{|i|
  j = 9 * (10 ** (44 * i) - 1) / 89
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
