(0..10000).each{|i|
  j = i * i + (i + 1) * (i + 1)
  print i
  print ' '
  puts j * (j + 2 * i * (i + 1))
}
