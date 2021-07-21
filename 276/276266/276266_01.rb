def A(m, n)
  a = Array.new(m, 1)
  ary = [1]
  while ary.size < n + 1
    i = a[1..-1].inject(:*) + 1
    i *= i
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end
def A276266(n)
  A(3, n)
end

n = 15
ary = A276266(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
