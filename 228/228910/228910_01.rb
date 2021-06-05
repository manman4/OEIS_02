def A(k, n)
  a = [1]
  i = 1
  i > k ? ary = [1] : ary = [0]
  while ary.size < n + 1
    a << 0
    b = [1] + (1..i).map{|j| (j + 1) * a[j] - j * a[j - 1]}
    a = b
    i += 1
    i > k ? ary << a[k] : ary << 0
  end
  ary[0..n]
end

n = 1500
ary = A(7, n)
(0..n).each{|i|
  j = -ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
