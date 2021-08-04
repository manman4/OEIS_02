def A(k, n)
  return [0] if n == 0
  ary = [0, 1]
  (2..n).each{|i| ary << ((i - 1) ** k + i ** k) * ary[-1] - (i - 1) ** (2 * k) * ary[-2]}
  ary
end

n = 200
ary = A(9, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
