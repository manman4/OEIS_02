def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0] # Bn = a[0]
  }
  ary
end

def A(n)
  a = bernoulli(n)
  a[1] = -1r / 2
  (1..n).map{|i| (0..i).inject(0){|s, j| s + ncr(i + j, 2 * j) * a[j] / (i + j).to_r}}
end

n = 30
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1].numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
