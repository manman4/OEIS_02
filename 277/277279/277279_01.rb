def A277279(n)
  a = [1, 1, 2, -1]
  ary = [1]
  while ary.size < n + 1
    i = a[1] * a[3] - a[2] ** 2
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

n = 200
ary = A277279(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
