include Math

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

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

n = 30
ary = bernoulli(2 * n)
f_ary = (0..n).map{|i| (-1) ** (i % 2 + 1) * f(2 * i) * (2 ** (2 * i) + 1) / (ary[2 * i] * 2 ** (4 * i - 1))}
(0..n).each{|i|
  j = f_ary[i].denominator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

p (0..5).map{|i| (PI ** (2 * i)).round}
