def A(a0, a1,a2, n)
  a, b, c = a0, a1, a2
  ary = [a0]
  n.times{
    a, b, c = b, c, a + b + c
    ary << a
  }
  ary
end

n = 10000
ary = A(151646890045, 836564809606, 942785024683, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts ary[i]
}
