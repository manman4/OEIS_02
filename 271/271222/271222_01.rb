def A271222(n)
  ary = [0]
  a, mod = 2, 3
  n.times{
    b = a % mod
    ary << b
    a = 2 * b * b + b + 4
    mod *= 3
  }
  ary
end
p A271222(100)
n = 30
ary = A271222(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
