def A(k, n)
  return [0] if n == 0
  ary = [0, 1]
  (2..n).each{|i| ary << ((2 * i - 3) ** k + (2 * i - 1) ** k) * ary[-1] - (2 * i - 3) ** (2 * k) * ary[-2]}
  ary
end

n = 50
(0..n).each{|i|
  j = A(i, i)[-1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
