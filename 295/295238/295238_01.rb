def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(k, m, n)
  a = f(n)
  (0..n).inject(0){|s, i| s + i ** (n - i) * a / f(n - i) * ncr(k * n + m * i + 1, i) / (k * n + m * i + 1r)}.to_i
end

n = 1000
(0..n).each{|i|
  j = A(0, 2, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}