def s(k, i)
  s = 0
  (1..i).each{|j| s += j ** (k * j + 1) if i % j == 0}
  s
end

n = 13
(1..n).each{|i|
  j = s(2, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
