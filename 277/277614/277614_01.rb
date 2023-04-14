def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  m = f(n)
  (0..n / 2).inject(0){|s, i| s + (n / 2.to_r) ** i * ncr(n - i, i) * m / f(n - i)}
end

n = 1000
(0..n).each{|i|
  j = A(i).to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
