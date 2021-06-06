puts '5 2'
puts '6 4'
puts '7 3'
(8..10000).each{|i|
  print i
  print ' '
  puts (i / 2 + 5) / 2
}
