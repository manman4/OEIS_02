def A(m, n)
  a, b, i = 1, 1, 0
  ary = [1]
  while i < n
    a, b = b, b + a * m ** (i + 2)
    i += 1
    ary << a
  end
  ary
end

n = 15
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
