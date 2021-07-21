def A276453(n)
  a = [1, 1, 2, 6]
  ary = [1]
  while ary.size < n + 1
    i = a[1..-1].inject(1){|s, i| s * (i + 1)}
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

n = 20
p ary = A276453(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
