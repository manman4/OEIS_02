s = 1
puts '1 1'
(2..10000).each{|i|
  i % 3 == 1? s -= i : s += i
  print i
  print ' '
  puts s
}
