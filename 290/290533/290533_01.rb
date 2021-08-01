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

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

n = 500
b_ary = bernoulli(2 * n)
ary = (0..n).map{|i| (2 * i * (2 * i + 1) * b_ary[2 * i]).numerator}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
