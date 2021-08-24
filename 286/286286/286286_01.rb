def A(n)
  a = 1
  s = 0
  t = 1
  ary = []
  1.step(2 * n - 1, 2){|i|
    a *= 1r / i
    s += a
    t *= i
    ary << (s * t).to_i
  }
  ary
end

n = 19
p ary = A(n)
(1..n).each{|i|
  j = ary[i - 1].numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
