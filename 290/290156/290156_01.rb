def A(a0, a1,a2, n)
  a, b, c = a0, a1, a2
  ary = [a0]
  n.times{
    a, b, c = b, c, a + b + c
    ary << a
  }
  ary
end

n = 10000
ary = A(99202581681909167232, 67600144946390082339, 139344212815127987596, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts ary[i]
}
