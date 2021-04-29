def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  return 1 if n == 0
  s = 0
  (1..n).each{|i|
    s += (-1) ** ((i - 1) % 2) * ncr(i + n / i - 2, i - 1) if n % i == 0
  }
  s
end

n = 10000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
