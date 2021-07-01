def A(n)
  a, b, c, d, e = 0, 1, 3, 6, 10
  ary = [0, 0]
  while ary.size <= n
    a, b, c, d, e = b, c, d, e, 6 * e - 15 * d + 20 * c - 15 * b + 6 * a
    ary << a
  end
  ary
end

n = 4000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
