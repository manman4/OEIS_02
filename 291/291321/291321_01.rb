def A(n)
  m = (n + 1) / 2
  ary = []
  (1..m).each{|i|
    (8..9).each{|j|
      ary << j * (10 ** (13 * i) - 1) / 79
    }
  }
  ary[0..n - 1]
end

n = 200
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
