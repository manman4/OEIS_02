def f(n)
  ary = [[1]]
  (1..n).each{|i|
    a = [0]
    (1..((Math.sqrt(i * 8 + 1) - 1) / 2).floor).each{|j|
      b = ary[i - j]
      s = 0
      s += b[j - 1].to_i + j * b[j].to_i
      a << s
    }
    ary << a
  }
  ary
end

def A(k, n)
  m = k * (k + 1) / 2
  ary = f(n + m)
  (m..m + n).map{|i| ary[i][k]}
end

n = 7000
ary = A(4, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
