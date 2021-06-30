def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  (0..k / 2).inject(0){|s, i| s + n ** (k - 2 * i) * ncr(k, 2 * i) * ncr(2 * i, i) / (i + 1)}
end

def B(k, n)
  (0..n).map{|i| A(k, i)}
end

def A247495(n)
  a = []
  (0..n).each{|i| a << B(i, n - i)}
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

n = 10
ary = A247495(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
