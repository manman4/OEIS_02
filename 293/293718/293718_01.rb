def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end
def A(k, n)
  ary = [1]
  (1..n).each{|i|
    ary << (f(i - 1) * (1..[i, k + 1].min).inject(0){|s, j| s + j * j * ary[i - j] / f(i - j).to_r}).to_i
  }
  ary
end
def A293718(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end
n = 10
ary = A293718(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
