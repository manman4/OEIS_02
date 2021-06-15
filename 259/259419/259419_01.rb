def A259419(n)
  a = [1, 2, 1, -3]
  i = 3
  while i < n
    i += 1
    if i % 3 == 2
      a << (a[i - 1] * a[i - 3] - 3 * a[i - 2] * a[i - 2]) / a[i - 4]
    else
      a << (a[i - 1] * a[i - 3] -     a[i - 2] * a[i - 2]) / a[i - 4]
    end
  end
  a[0..n]
end

n = 200
ary = A259419(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
