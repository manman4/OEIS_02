def A276130(n)
  a = Array.new(5, 1)
  ary = [1]
  while ary.size < n + 1
    b, c, d, e = a[1], a[2], a[3], a[4]
    i = (b + d) * (c + e)
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

n = 30
ary = A276130(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
