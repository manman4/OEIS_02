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

n = 300
ary = bernoulli(2 * n)
(1..n).each{|i|
  j = ((1 - 1r / 2 ** (2 * i - 1)) * ary[2 * i].abs / (4 * i * (2 * i - 1))).numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
