def A(k, n)
  ary = [1]
  s, t, i = 1, 1r / 2, 1
  while ary.size < n + 1
    s += t ** k
    ary << s
    i += 2
    t *= i / (i + 1r)
  end
  ary
end

n = 900
ary = A(2, n)
(0..n).each{|i|
  j = ary[i].numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
