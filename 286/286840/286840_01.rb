def A(k, m, n)
  ary = [0]
  a, mod = k, m
  n.times{
    b = a % mod
    ary << b
    a = b ** m
    mod *= m
  }
  ary
end
def A286840(n)
  A(5, 13, n)
end
p A286840(100)
n = 10
ary = A286840(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
