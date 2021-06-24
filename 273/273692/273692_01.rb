(0..10000).each{|i|
  j = ((3 * i + 2r) / 2 ** (i + 2)).denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
