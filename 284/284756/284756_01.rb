def A284756(n)
  a, b, c, i = 0, 0, 1, -1
  ary = [0, 0]
  while i < n
    i += 1
    a, b, c = b, c, (2 * (2 * i + 1) * (5 * i * i + 5 * i + 2) * c - 8 * i * (7 * i * i + 1) * b + 22 * i * (i - 1) * (2 * i - 1) * a) / ((i + 1) ** 3)
    ary << b
  end
  ary[2..-1]
end

n = 900
ary = A284756(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
