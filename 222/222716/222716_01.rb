n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts (i - 1) * i * (i + 1) * (3 * i ** 2 - 2) / 6
}
