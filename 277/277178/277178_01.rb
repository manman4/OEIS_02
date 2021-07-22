def A(k, n)
  ary = [0]
  a, b = 0, 1
  cnt = 0
  while cnt < n
    cnt += 1
    a, b = b, ((5 * cnt + 2 * k - 4) * b - (4 * cnt + 2 * k - 4) * a) / cnt
    ary << a
  end
  ary
end

n = 20
p ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
