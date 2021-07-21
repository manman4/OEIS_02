def A276402(n)
  a, b, c, d, e, f = 1, 1, -1, 1, -3, -3
  ary = [1]
  while ary.size < n + 1
    i = b * f + 2 * c * e - 2 * d * d
    break if i % a > 0
    a, b, c, d, e, f = b, c, d, e, f, i / a
    ary << a
  end
  ary
end

n = 30
p ary = A276402(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
