def A(n)
  s = 1
  f = [[0], [1]]
  ary = [1, 1]
  2.upto(n){|i|
    s = 0
    a = []
    (i - 1).downto(2){|j|
      k = f[i - j][j - 1 - j % 2 - 1]
      a.unshift(s += k) if !(k.nil?)
    }
    f.push(a.unshift(f[-1][0] + a[0].to_i))
    ary << a[0]
  }
  ary[0..n]
end

n = 4000
ary = A(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
