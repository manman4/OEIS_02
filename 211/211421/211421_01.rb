def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  f(3 *n) * f(8 * n) / (f(6 * n) * f(4 * n) * f(n))
end

n = 600
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}