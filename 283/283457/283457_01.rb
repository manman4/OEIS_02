def A283457(n)
  (0..n).map{|i| 1 + (45 * 2 ** (2 * i + 2) + 45 * 2 ** (i + 2) + 40 * 3 ** (i + 1) + 5 * 2 ** (i + 3) * 3 ** (i + 1) + 24 * 5 ** (i + 1)) / 120}
end

n = 21
ary = A283457(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
