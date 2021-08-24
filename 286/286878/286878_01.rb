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
def A286878(n)
  A(13, 17, n)
end
p A286878(100)
n = 10
ary = A286878(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
