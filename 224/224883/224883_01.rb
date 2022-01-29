def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(k, n)
  (1..n - 1).inject(1){|s, i| s * (1 + i * k ** (n - 1))} * k ** n / f(n)
end

n = 100
(0..n).each{|i|
  j = A(2, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
