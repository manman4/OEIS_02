def A277586(n)
  s = 1
  t = 1
  n_ary = [1]
  (1..n).each{|i|
    s *= (2 * i * i) / ((2 * i) * (2 * i + 1r))
    t += s
    n_ary << t.numerator
  }
  n_ary
end

n = 1200
ary = A277586(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
