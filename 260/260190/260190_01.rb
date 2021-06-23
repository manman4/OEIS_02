def A(m, n)
  i, a, b = 0, 1, 0
  ary = [1]
  while i < n
    i += 1
    a, b = b, b * m ** i + a
    ary << a
  end
  ary
end
def A260190(n)
  A(-1, n)
end

n = 77
ary = A260190(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
