def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A(k, n)
  ary = [1]
  (1..n).each{|i| ary << k * (0..i - 1).inject(0){|s, j| s + ncr(i - 1, j) * ary[j]}}
  ary
end
def A292866(n)
  (0..n).map{|i| A(-i, i)[-1]}
end
p A292866(20)
