def ncr(n, r)
  return 1 if r == 0
  return 0 if r > n
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end
def A(k, n)
  ary = [1]
  (1..n).each{|i|
    ary << k * k * (f(i - 1) * (1..i).inject(0){|s, j| s + ncr(k - 1, j - 1) * ary[i - j] / f(i - j).to_r}).to_i
  }
  ary
end
n = 21
ary = A(2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
