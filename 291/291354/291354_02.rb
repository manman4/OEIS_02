def A(n)
  m = (n + 3) / 4
  ary = []
  (1..m).each{|i|
    (6..9).each{|j|
      ary << j * (10 ** (58 * i) - 1) / 59
    }
  }
  ary[0..n - 1]
end

n = 100
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
