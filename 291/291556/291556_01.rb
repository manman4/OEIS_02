def A(k, n)
  return [0] if n == 0
  ary = [0, 1]
  (2..n).each{|i| ary << ((i - 1) ** k + i ** k) * ary[-1] - (i - 1) ** (2 * k) * ary[-2]}
  ary
end

n = 59
a = []
(0..n).each{|i| a << A(i, n - i)}
ary = []
(0..n).each{|i|
  (0..i).each{|j|
    ary << a[i - j][j]
  }
}
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
