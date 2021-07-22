def A278706(n)
  a = [1, 1, 1, 2, 3, 5]
  ary = [1]
  while ary.size < n + 1
    a = *a[1..-1], a[-1] + a[-3] + a[-5] - a[-6]
    ary << a[0]
  end
  ary
end

n = 6000
ary = A278706(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
