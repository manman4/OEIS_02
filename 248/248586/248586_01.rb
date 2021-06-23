def A248586(n)
  a, b, c = 1, 5, 45
  ary = [1]
  i = 3
  while ary.size < n + 1
    j = -(-19 * i * i + 19 * i - 5) * c - 35 * (i - 1) * (i - 1) * b + 17 * (i - 1) * (i - 2) * a
    break if j % (i * i) > 0
    a, b, c = b, c, j / (i * i)
    ary << a
    i += 1
  end
  ary
end

n = 1000
ary = A248586(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
