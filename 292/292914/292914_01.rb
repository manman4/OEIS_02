def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A(k, n)
  ary = [1]
  (1..n).each{|i| ary << k * (0..i - 1).inject(0){|s, j| s + ncr(i - 1, j) * ary[j]}}
  ary
end

n = 17
ary = A(1, n)
(0..ary.size - 1).each{|i|
  j = i ** i *ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
