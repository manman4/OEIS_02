def A271223(n)
  d_ary = []
  ary = [0]
  a, mod = 1, 3
  (n + 1).times{|i|
    b = a % mod
    d_ary << (b - ary[-1]) / 3 ** i
    ary << b
    a = b * b + b + 2
    mod *= 3
  }
  d_ary
end
n = 10000
ary = A271223(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
