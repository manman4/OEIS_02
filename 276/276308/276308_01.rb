def A(m, n)
  a = Array.new(m, 1)
  ary = [1]
  while ary.size < n + 1
    i = (a[1] + 1) * (a[-1] + 1)
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

def A276308(n)
  A(4, n)
end

n = 2000
ary = A276308(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
