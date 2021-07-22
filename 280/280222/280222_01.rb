def A(m, n)
  i, a, b = 0, 0, 1
  ary = [0]
  while i < n
    i += 1
    a, b = b, b * m ** i + a
    ary << a
  end
  ary
end
def A280222(n)
  A(-3, n)
end

n = 100
ary = A280222(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
