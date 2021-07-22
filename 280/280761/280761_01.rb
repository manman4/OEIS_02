def A280761(n)
  a, b = 8, 280
  ary = [8]
  while ary.size < n + 1
    a, b = b, 34 * b - a
    ary << a
  end
  ary
end

n = 1000
ary = A280761(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
