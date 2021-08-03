def A(k, n)
  ary = [[1]]
  (1..n).each{|i|
    a = [0]
    (1..((Math.sqrt(i * 8 + 1) - 1) / 2).floor).each{|j|
      b = ary[i - j]
      s = 0
      s += k * b[j - 1].to_i + b[j].to_i
      a << s
    }
    ary << a
  }
  ary
end

def B(k, n)
  A(-k, n).map{|i| i.inject(:+)}
end

n = 139
a = []
(0..n).each{|i| a << B(i, n - i)}
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
