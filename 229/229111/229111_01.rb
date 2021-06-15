def A229111(n)
  a, b = 1, -5
  ary = [1]
  i = 1
  while i < n
    a, b = b, (-(2 * i + 1) * (11 * (i + 1) * i + 5) * b - 125 * i ** 3 * a) / (i + 1) ** 3
    ary << a
    i += 1
  end
  ary
end

n = 21
ary = A229111(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
