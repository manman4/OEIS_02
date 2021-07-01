def A227999(n)
  a = [1] * 4 + [2, 2]
  ary = a
  i = 5
  while i < n
    j = (a[1] * a[-1] + a[2] * a[-2] + a[3] * a[-3]) / a[0]
    a = *a[1..-1], j
    ary << j
    i += 1
  end
  ary[0..n]
end

n = 200
ary = A227999(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
