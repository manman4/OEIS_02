def A272853(n)
  a, b, c = 9, 791, 65601
  ary = [a]
  while ary.size < n + 1
    a, b, c = b, c, 82 * c + 82 * b - a
    ary << a
  end
  ary
end

n = 600
ary = A272853(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
