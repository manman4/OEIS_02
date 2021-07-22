def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A277584(n)
  [0] + (1..n).map{|i| ncr(3 * i - 1, i - 1) ** 2}
end

n = 700
ary = A277584(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
