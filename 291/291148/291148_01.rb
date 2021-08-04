def A(n)
  s = 1
  f = [[1]]
  ary = [1]
  1.upto(n){|i|
    s = 0
    a = []
    ((Math.sqrt(i * 8 + 1) - 1) / 2).floor.downto(1){|j|
      a.unshift(s += f[i - j][j - 1])
    }
    f.push(a.unshift(s))
    ary << s
  }
  ary
end

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

n = 43
ary = I(A(n), n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
