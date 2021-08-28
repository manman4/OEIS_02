def A283456(n)
  (0..n).map{|i| 1 + (9 * 2 ** (2 * i + 2) + 9 * 2 ** (i + 2) + 8 * 3 ** (i + 1)) / 24}
end

n = 24
ary = A283456(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
