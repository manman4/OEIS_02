def A(m, n)
  ary = [m]
  (2..n).each{|i| ary << ary[-1] * (ary[-1] + 1)}
  [0] + ary
end

def B(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = []
  (1..n).each{|i|
    (1..i - 1).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

n = 13
ary = B(n + 1)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
