def A(n)
  s = n
  (n - 1).downto(1){|i|
    s += 1
    s = i.to_r / s
  }
  s += 1
  1r / s
end

n = 900
ary = (1..n).map{|i| A(i)}
(1..n).each{|i|
  j = ary[i - 1].denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
