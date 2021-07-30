def A(n)
  a, b, c, d = 1, 1, 2, 3
  ary = [1]
  n.times{
    a, b, c, d = b, c, d, a * b * c * d
    ary << a
  }
  ary
end

n = 20
ary = A(n)
(0..n).each{|i|
  j = ary[i].to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
