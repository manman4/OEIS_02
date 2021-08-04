def A(m, n)
  ary = [m]
  a, b = m, 1
  (1..n).each{|i|
    a, b = b, i * a + b
    ary << a
  }
  ary
end

n = 1000
ary = A(-1, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
