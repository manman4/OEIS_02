def A225409(n)
  ary = [1]
  a = 1
  n.times{|i|
    b = (a + 3 * (a ** 3 + 9)) % (10 ** (i + 2))
    ary << (b - a) / (10 ** (i + 1))
    a = b
  }
  ary
end

n = 100
p ary = A225409(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
