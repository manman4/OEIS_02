def A(k, n)
  ary = []
  a, mod = k, 5
  (n + 1).times{|i|
    b = a % mod
    ary << b
    a = b ** 5
    mod *= 5
  }
  str = (2 * ary[-1]).to_s(5).reverse
  (0..n).map{|i| str[i].to_i}
end

n = 65
ary = A(2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
