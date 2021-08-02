def A(n)
  a = [1]
  ary = [1]
  (1..n).each{|i|
    a += [0]
    b = [1] + (1..i).map{|j| (2 * i - 1 - j) * a[j - 1] + (j + 1) * a[j]}
    a = b
    ary << a.reverse
  }
  ary.flatten
end

n = 139
ary = A(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
