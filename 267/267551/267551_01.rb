n = 1000
s, t = 2, 1
(0..n).each{|i|
  j = s.to_s.reverse
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
  s, t = t, s + t
}
