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

n = 1500
ary = A(3, n)
sum = 0
s = 1
print 0
print ' '
puts 0
(1..n).each{|i|
  sum += s * ary[i - 1]
  break if sum.to_s.size > 1000
  print i
  print ' '
  puts sum
  s *= 5
}
