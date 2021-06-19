def A253832(n)
  a, b, c, d = 1, 1, 1, 1
  ary = [1]
  while ary.size < n + 1
    a, b, c, d = b, c, d, d * (1 + c / a)
    ary << a
  end
  ary
end

n = 30
ary = A253832(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
