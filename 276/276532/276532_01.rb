def A(k, n)
  a = Array.new(k, 1)
  ary = [1]
  while ary.size < n + 1
    i = a[-1] * a[1] + a[2..-2].inject(:*)
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end
def A276532(n)
  A(7, n)
end

n = 30
ary = A276532(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
