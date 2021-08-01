def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A290575(n)
  (0..n).map{|i| (0..i).inject(0){|s, j| s + ncr(i, j) ** 2 * ncr(j + j, i) ** 2}}
end
n = 20
ary = A290575(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
