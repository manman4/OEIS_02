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

n = 1000
ary = (0..n).map{|i| B(i, i)[-1]}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
