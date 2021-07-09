def A(k, m, n)
  a = Array.new(4, 1)
  ary = [1]
  while ary.size < n + 1
    j = a[1] ** m * a[-1] ** k + a[2]
    break if j % a[0] > 0
    a = *a[1..-1], j / a[0]
    ary << a[0]
  end
  ary
end

n = 15
ary = A(2, 3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
