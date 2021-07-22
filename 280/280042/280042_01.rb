def A(m, n)
  ary = []
  (0..n - 1).each{|i|
    s = m ** i
    (i - 1).downto(0){|j|
      s = 1r / s
      s += m ** j
    }
    ary << s.numerator
  }
  ary
end

n = 50
ary = A(10, n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
