def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  (0..n).map{|i| (0..i).inject(0){|s, j| s + (ncr(i, j) * ncr(i + j, j + 1)) ** k} / 2}
end

n = 50
ary = A(2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
