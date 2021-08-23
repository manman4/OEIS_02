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
    a2 = a[2].nil? ? 0 : a[2]
    ary << a2
  }
  ary
end

n = 5000
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
