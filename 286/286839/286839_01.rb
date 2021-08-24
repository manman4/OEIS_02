def A(k, m, n)
  d_ary = []
  ary = [0]
  a, mod = k, m
  (n + 1).times{|i|
    b = a % mod
    d_ary << (b - ary[-1]) / m ** i
    ary << b
    a = b ** m
    mod *= m
  }
  d_ary
end
def A286839(n)
  A(8, 13, n)
end
n = 10000
ary = A286839(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
