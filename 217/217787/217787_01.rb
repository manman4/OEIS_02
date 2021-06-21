def A(m, n)
  a = Array.new(4, 1)
  ary = [1]
  while ary.size < n + 1
    i = a[1] * a[3] + a[2] ** m
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

def A217787(n)
  A(0, n)
end

n = 5000
ary = A217787(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
