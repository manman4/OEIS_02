def s(k, i)
  s = 0
  (1..i).each{|j| s += j ** (k * j + 1) if i % j == 0}
  s
end

def A(k, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(k, i)}
  n.times{|i|
    ary << (1..i + 1).inject(0){|s, j| s + ary[-j] * s_ary[j]} / (i + 1)
  }
  ary
end

n = 9
a = []
(0..n).each{|i| a << A(i, n - i)}
p a
ary = []
(0..n).each{|i|
  (0..i).each{|j|
    ary << a[i - j][j]
  }
}
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
