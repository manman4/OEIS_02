def A(n)
  ary = [1, -1]
  (2..n).each{|i|
    ary << (-(91 * i ** 3 - 221 * i * i + 160 * i - 36) * ary[-1] - 16 * (i -1 ) * (2 * i - 3) * (7 * i -3 ) * ary[-2]) / (2 * i * (2 * i - 1) * (7 * i - 10))
  }
  ary
end

n = 2000
ary = A(n)
(0..1000).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
