def A225984(n)
  a, b, c, d, e = 5, -1, 3, -7, 11
  ary = [5]
  n.times{
    a, b, c, d, e = b, c, d, e, -e + d - c + a
    ary << a
  }
  ary
end

n = 10000
ary = A225984(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
