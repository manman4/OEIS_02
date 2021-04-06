def A(l, m, n)
  a = Array.new(3, 1)
  ary = [1]
  while ary.size < n + 1
    i = a[2] ** l * a[1] ** m + 1
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

def A208210(n)
  A(2, 3, n)
end

n = 12
ary = A208210(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
