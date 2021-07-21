def A(k, m, n)
  a = Array.new(2 * k, 1)
  ary = [1]
  while ary.size < n + 1
    i = a[-1] * a[1] + a[k] ** m
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

def A276530(n)
  A(3, 3, n)
end

n = 35
ary = A276530(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
