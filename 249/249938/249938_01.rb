def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A000670(n)
  ary = [1]
  (1..n).each{|i|
    ary << (0..i - 1).inject(0){|s, j| s + ncr(i, j) * ary[j]}
  }
  ary
end

n = 15
ary = A000670(2 * n)
(0..n).each{|i|
  j = ary[2 * i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}