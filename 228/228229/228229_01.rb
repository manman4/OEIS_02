n = 500
s = 0
(0..n).each{|i|
  s *= i * (i + 1)
  s += 1
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
}
