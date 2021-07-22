def A(m, n)
  a, b, i = 1, m + 1, 1
  ary = [1]
  while i < n
    a, b = b, b * m ** (i + 1) + a
    i += 1
    ary << a
  end
  ary
end

n = 100
ary = A(3, n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
