def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  (0..n).inject(0){|s, i| s + ncr(n, n - i) * ncr(n - 1, n - i) * k ** i * f(n - i)}
end

def B(n)
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << A(j, i - j)
    }
  }
  ary
end

p ncr(-1, 0) # 1
p A(0, 0)    # 1
n = 3
ary = B(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}