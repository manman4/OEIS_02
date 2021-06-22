def A(k, l, n)
  a = Array.new(k * 2, 1)
  ary = [1]
  while ary.size < n + 1
    break if (a[1] * a[-1] + a[k] * l) % a[0] > 0
    a = *a[1..-1], (a[1] * a[-1] + a[k] * l) / a[0]
    ary << a[0]
  end
  ary
end

def A275174(n)
  A(4, 1, n)
end

n = 1000
ary = A275174(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
