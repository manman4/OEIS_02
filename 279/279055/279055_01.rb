def A(k, n)
  a = [1]
  ary = [1]
  cnt = 0
  while cnt < n
    cnt += 1
    a << a[-1] * cnt
    ary << (0..cnt).inject(0){|s, i| s + (a[i] * a[-1 - i]) ** k}
  end
  ary
end

n = 16
ary = A(2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
