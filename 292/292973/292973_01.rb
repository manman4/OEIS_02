def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end
def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A(k, n)
  ary = [1]
  (1..n).each{|i| ary << (-1) ** (k % 2 + 1) * f(k) * (0..i - 1).inject(0){|s, j| s + (-1) ** (j % 2) * ncr(i - 1, j) * ncr(j + 1, k) * ary[i - 1 - j]}}
  ary
end
def A292973(n)
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
p A292973(10)
