def A225458(n)
  ary = [9]
  a = 9
  n.times{|i|
    b = (a + a ** 9 - 9) % (10 ** (i + 2))
    ary << (b - a) / (10 ** (i + 1))
    a = b
  }
  ary
end

n = 10100
ary = A225458(n)
(0..10000).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
