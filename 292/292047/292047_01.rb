def A(n)
  ary = [[1]]
  (1..n).each{|i|
    a = [0]
    (1..((Math.sqrt(i * 8 + 1) - 1) / 2).floor).each{|j|
      b = ary[i - j]
      s = 0
      s += (-1) ** (j % 2) * b[j - 1].to_i + b[j].to_i
      a << s
    }
    ary << a
  }
  ary
end

n = 481
ary = A(n).flatten
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
