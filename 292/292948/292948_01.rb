def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A(k, n)
  ary = [1]
  (1..n).each{|i| ary << (-1) ** (k % 2 + 1) * (0..i - 1).inject(0){|s, j| s + (-1) ** (j % 2) * ncr(i - 1, j) * ncr(j + 1, k) * ary[i - 1 - j]}}
  ary
end
def A292948(n)
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
p A292948(20)
n = 10
ary = A292948(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
