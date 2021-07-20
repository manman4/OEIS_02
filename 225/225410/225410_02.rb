def A225410(n)
  ary = [7]
  a = 7
  n.times{|i|
    b = (a + 3 * (9 * a ** 3 - 7)) % (10 ** (i + 2))
    ary << (b - a) / (10 ** (i + 1))
    a = b
  }
  ary
end

n = 100
p ary = A225410(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
