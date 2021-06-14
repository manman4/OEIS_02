def A214733(n)
  a, b = 0, 1
  ary = [0]
  while ary.size < n + 1
    a, b = b, -b - 3 * a
    ary << a
  end
  ary
end

n = 10000
ary = A214733(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
