def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, n)
  return 1 if n == 0 or k == 0
  (1..n).inject(0){|s, i| s + k ** (n - i) * ncr(n, i) * ncr(n, i - 1)} / n
end

def B(n)
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << A(i - j, j)
    }
  }
  ary
end

n = 10
ary = B(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
