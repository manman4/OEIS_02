def A(k, n)
  # aは符号なし
  a = [1]
  ary = [1]
  (1..n).each{|i|
    a << 0
    b = [0]
    (0..i - 1).each{|j|
      b[j + 1] = a[j] + (i - 1) * a[j + 1]
    }
    a = b
    ary << (0..i).inject(0){|s, j| s + (k * j + 1) ** (j - 1) * a[j]}
  }
  ary
end

n = 20
p ary = A(2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
