def s(i)
  s = 0
  (1..i).each{|j| s += j ** (j + 1) if i % j == 0}
  s
end

n = 19
(1..n).each{|i|
  j = s(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
