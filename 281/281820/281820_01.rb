def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end
def A(n)
  s = 0
  ary = []
  (1..n).each{|i|
    s += (30 * i - 11r) / ((8 * i - 4) * i * i * i * ncr(2 * i, i) ** 2)
    ary << s
  }
  ary
end

n = 400
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1].numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
