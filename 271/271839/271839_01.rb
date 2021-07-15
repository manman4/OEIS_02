def A(k, n)
  a = Array.new(2 * k, 1)
  ary = [1]
  while ary.size < n + 1
    i = a[-1] * a[1] + a[k] ** 2
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

k = 10
n = 600
ary = A(k, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
