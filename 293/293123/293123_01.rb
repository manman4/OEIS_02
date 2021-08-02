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
  (1..n).each{|i| ary << (-1) ** (k % 2 + 1) * (k..i - 1).inject(0){|s, j| s + (-1) ** (j % 2) * f(j + 1) * ncr(i - 1, j) * ary[i - 1 - j]}}
  ary
end

n = 21
ary = A(2, n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
