def A268924(n)
  ary = [0]
  a, mod = 1, 3
  n.times{
    b = a % mod
    ary << b
    a = b * b + b + 2
    mod *= 3
  }
  ary
end
p A268924(10)
n = 3
ary = A268924(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
