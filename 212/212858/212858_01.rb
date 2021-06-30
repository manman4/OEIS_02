def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n, k)
  a = [1]
  (1..n).each{|i| a << (0..i - 1).inject(0){|s, j| s + (-1) ** (j + i + 1) * ncr(i, j) ** k * a[j]}}
  a
end

n = 200
ary = A(n, 5)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

