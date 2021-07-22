def A278314(n)
  a, b, c, d, e, f, g, h = 1, -3, -5, -14, -8, 69, -435, 2065
  ary = [0, 0, 1]
  while ary.size < n
    a, b, c, d, e, f, g, h = b, c, d, e, f, g, h, (b * h + 3 * c * g - 3 * d * f + 6 * e * e) / a
    ary << a
  end
  ary[0..n - 1]
end

n = 200
ary = A278314(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
