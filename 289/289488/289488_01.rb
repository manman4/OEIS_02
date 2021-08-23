def A(n)
  a = 1
  s = 0
  ary = []
  1.step(2 * n - 1, 2){|i|
    a *= 1r / i
    s += a
    ary << s
  }
  ary
end

n = 500
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1].denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
