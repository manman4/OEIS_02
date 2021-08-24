def A(k, n)
  a = (1..k).to_a
  ary = [1]
  while ary.size < n
    a = *a[1..-1], (1..k).inject(0){|s, i| s + i * a[-i]}
    ary << a[0]
  end
  ary
end

n = 31
ary = A(5, n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
