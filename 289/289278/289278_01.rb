def A005259(n)
  i = 0
  a, b = 1, 5
  ary = [1]
  while i < n
    i += 1
    a, b = b, ((((34 * i + 51) * i + 27) * i + 5) * b - i ** 3 * a) / (i + 1) ** 3
    ary << a
  end
  ary
end

n = 10000
ary = A005259(2 * n + 1)
(0..n).each{|i|
  j = ary[i] % ((2 * i + 1) ** 2)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
