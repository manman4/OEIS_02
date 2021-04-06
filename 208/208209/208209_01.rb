def A(m, n)
  a = Array.new(m, 1)
  ary = [1]
  while ary.size < n + 1
    i = (a[1..-1].inject(:*)) ** 2 + 1
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

def A208209(n)
  A(3, n)
end

n = 12
ary = A208209(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
